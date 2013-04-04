#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

#define MAX_CLIENTS 3

typedef enum {
  UNDEFINED = -1,
  INIT = 0,
  CSENT, 
  READY_R,
  READY_W_IN,
  READY_W_OUT,
  DISCONNECTED
} state_t;

typedef struct pair{
  int ifd;
  int ofd;
  state_t state;
  time_t to;
  int tunnel;
} pair_t;

typedef struct xfer_buf{
  char buf[1460];
  int len;
} xfer_buf_t;
pair_t pairs[MAX_CLIENTS + 1] = {-1};
xfer_buf_t mainbuf = {0};
char verb[32] = {0};
char dest_host[255] = {0};
char  dest_port_str[6] = {0};
int dest_port = 0;
const char localhost[] = "127.0.0.1";
const char reply200[] = "HTTP/1.1 200 Connection established\r\n\r\n";
const char reply503[] = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
#define IN 1
#define OUT 0
#define PAIR_ST(i) pairs[i].state
#define PAIR_TU(i) pairs[i].tunnel
#define PAIR_FD(d,i) ((d)?pairs[i].ifd:pairs[i].ofd)
#define SET_PAIR_FD(d,i,v)  if(d) pairs[i].ifd=(v); else pairs[i].ofd=(v);
#define MAIN_FD pairs[0].ifd

static int set_nonblocking(int fd) {
  int flags;

  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    flags = 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
} 

static int init_fds(int dir, fd_set * fds) {
  int maxfd = -1;
  int i;
  int fd=-1;
  for(i = 0; i < MAX_CLIENTS + 1; ++i) {
    fd = -1;
    if((PAIR_FD(dir, i) == -1))
      continue;
    if(OUT == dir){
        if( PAIR_ST(i)==READY_W_IN)
          fd = PAIR_FD(IN,i);
        else if(PAIR_ST(i)==READY_W_OUT)
          fd = PAIR_FD(OUT,i);
    } else {
      if( (PAIR_ST(i)==READY_R) || (PAIR_ST(i)==INIT))
        fd = PAIR_FD(IN,i);
    }
    if(fd>0){
      if(maxfd < fd)
        maxfd = fd;
      FD_SET(fd, fds);
      if((dir==IN) && (PAIR_ST(i)!=INIT) ){
        if(maxfd < PAIR_FD(OUT, i))
          maxfd = PAIR_FD(OUT, i);
        FD_SET(PAIR_FD(OUT, i), fds);
      }
    }
  }
  return maxfd;
}

static void closepair(int i) {
  PAIR_ST(i) = DISCONNECTED;
  close(PAIR_FD(IN, i));
  close(PAIR_FD(OUT, i));
  SET_PAIR_FD(IN, i,-1);
  SET_PAIR_FD(OUT, i,-1);
}

static int get_free_pair() {
  int i;
  for(i = 1; i < MAX_CLIENTS + 1; ++i) {
    if((PAIR_ST(i) == DISCONNECTED)|| (PAIR_ST(i)==INIT))
      break;
  }
  return i;
}

static void get_verb(const char * data, char * verb) {
  int len = 0;
  while(data[len] != ' ' && data[len] != '\r' && data[len] != '\n') len++;
  int i;
  for(i = 0; i < len; i++){
    verb[i] = data[i];
  }
  verb[len] = '\0';
}

static void get_dest_addr(const char * data, char * server, char * port) {
  int start = 0;
  int len = 0;
  int pos;
  int i;
  while(data[start] != ' ' && data[start] != '\r' && data[start] != '\n') start++;
  start++;
  len = start;
  for(i = start; data[i] != ':'; i++){
    server[i - start] = data[i];
  }
  server[i - start] = '\0';
  ++i;
  start = i;
  for(;data[i] != ' '; i++){
    port[i - start] = data[i];
  }
  port[i - start] = '\0';
}

int main(int argc, char **argv)
{
  struct timeval timeout;
  fd_set infds, outfds;
  int maxfd = -1;
  int tmp = -1;
  int ix_free_slot = -1;
  struct sockaddr_in  client_addr;
  socklen_t client_len = sizeof(client_addr);

  if(argc < 3)
    return 1;
  for(tmp=0;tmp<MAX_CLIENTS+1;tmp++){
    SET_PAIR_FD(IN,tmp,-1);
    SET_PAIR_FD(OUT,tmp,-1);
    PAIR_ST(tmp) = INIT;
  }

  pairs[0].ifd = socket(PF_INET, SOCK_STREAM, 0);

  if(pairs[0].ifd < 0) {
    return 1;
  }
  unsigned short port = atoi(argv[1]);
  unsigned short local_port = atoi(argv[2]);
  struct sockaddr_in server;
  server.sin_family = PF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  int len = sizeof(server);

  if(bind(pairs[0].ifd, (struct sockaddr *)&server, len) < 0) {
    return 1;
  }

  if(listen(pairs[0].ifd, 5) < 0)
    return 1;
 
  while(1) {
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;
    FD_ZERO(&infds);
    FD_ZERO(&outfds);
    maxfd = init_fds(IN, &infds);
    tmp = init_fds(OUT, &outfds);
    maxfd = maxfd < tmp?tmp:maxfd;
    int j;
    for(j = 0; j < MAX_CLIENTS + 1; ++j) {
      printf("PAIR[%d] IN fd: %d, OUT fd: %d, state %d tunnel %d\n", j, PAIR_FD(IN, j), PAIR_FD(OUT, j), PAIR_ST(j), PAIR_TU(j));
    }
    printf("BUF len is %d\n", mainbuf.len);

    tmp = select(maxfd + 1, &infds, &outfds , NULL, &timeout);
    if(tmp < 0) {
      printf("select() error: %m\n");
      break;
    } else if(tmp == 0) {
      // to porcessing for every pair
    } else {
      int i;
      for(i = 0; i < MAX_CLIENTS + 1; ++i) {
        if((PAIR_FD(IN,i)>0) && FD_ISSET(PAIR_FD(IN, i),&infds)) {
          if(i == 0 && pairs[0].state == INIT) {
            ix_free_slot = get_free_pair();
            if(ix_free_slot > MAX_CLIENTS)
              continue;
            SET_PAIR_FD(IN, ix_free_slot, accept(MAIN_FD, (struct sockaddr *) &client_addr, &client_len));
            SET_PAIR_FD(OUT, ix_free_slot, socket(PF_INET, SOCK_STREAM, 0));
            set_nonblocking(PAIR_FD(OUT, ix_free_slot));
            PAIR_ST(ix_free_slot) = INIT;
          } else {
            if(!mainbuf.len) {
              int ret;
              ret = recv(PAIR_FD(IN, i), mainbuf.buf, sizeof(mainbuf.buf), 0);
              if(ret > 0) {
                printf("receiving: %d sock: %d\n", ret, PAIR_FD(IN, i));
                mainbuf.len = ret;
                if(PAIR_ST(i) == INIT) {
                  get_verb(mainbuf.buf, verb);
                  if(!strcmp(verb, "CONNECT")){
                    PAIR_TU(i) = 1;
                    get_dest_addr(mainbuf.buf, dest_host, dest_port_str);
                    dest_port = atoi(dest_port_str);
                  } else {
                    strncpy(dest_host, localhost, sizeof(localhost));
                    dest_port = local_port;
                  }
                  struct sockaddr_in server;
                  struct hostent * hp;
                  server.sin_family = PF_INET;
                  hp = gethostbyname(dest_host);
                  if(!hp)
                    continue;
                  bcopy( (char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length );
                  server.sin_port = htons(dest_port);

                  connect(PAIR_FD(OUT,i), (struct sockaddr *)&server, sizeof(server));
                  PAIR_ST(i) = READY_W_OUT;   
                  // process connect , reply back if needed and set state to READY_W
                } else if(PAIR_ST(i) == READY_R) {
                  PAIR_ST(i) = READY_W_OUT;
                }
              } else {
                closepair(i);
              }
            }           
          }      
        }
        if((PAIR_FD(OUT,i)>0) && FD_ISSET(PAIR_FD(OUT, i),&infds)) {
          if(!mainbuf.len) {
            int ret;
            ret = recv(PAIR_FD(OUT, i), mainbuf.buf, sizeof(mainbuf.buf), 0);
            if(ret > 0) {
              mainbuf.len = ret;
              printf("receiving: %d sock: %d\n", ret, PAIR_FD(OUT, i));
              PAIR_ST(i)=READY_W_IN;
            } else {
              closepair(i);
            }
          }
        }
        if((PAIR_FD(OUT,i)>0) && FD_ISSET(PAIR_FD(OUT, i),&outfds)) {
          if(PAIR_TU(i) > 0) {
            send(PAIR_FD(IN, i), reply200, sizeof(reply200)-1, 0);
            mainbuf.len = 0;
            PAIR_TU(i) = 0;
            PAIR_ST(i) = READY_R;
          }
          if(mainbuf.len) {
            send(PAIR_FD(OUT, i), mainbuf.buf, mainbuf.len, 0);
            printf("sent: sock: %d\n", PAIR_FD(OUT, i));
            mainbuf.len = 0;
            PAIR_ST(i) = READY_R;
          }
        } 
        if((PAIR_FD(IN,i)>0) && FD_ISSET(PAIR_FD(IN, i),&outfds)) {
          if(mainbuf.len) {
            send(PAIR_FD(IN, i), mainbuf.buf, mainbuf.len, 0);
            printf("sent: sock: %d\n", PAIR_FD(IN, i));
            mainbuf.len = 0;
            PAIR_ST(i) = READY_R;
          }
        } 
      }
    }
  }
  return 0;
}
 
