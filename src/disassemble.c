/*
 
 Header goes here 

*/
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "disassemble.h"

static void info() {
  printf("Try \'disassemble -h\' for more information\n"); //, PACKAGE);
}

int check_header(FILE *f)
{
  return 1;
}

int extract_files(FILE *f)
{
  return 1;
}
int main(int argc, char **argv) {

  if (argc < 2) {
    info();
    return -1; 
  }
  char o;
  int check = 0;
  int disassemble = 0;
  char in_file_name[1024] = { 0 };
  char out_file_name[1024] = { 0 };
  while((o = getopt(argc, argv, ":c:d:ho:")) != -1) {
    switch (o) {
      case 'c':
        check = 1;
        strncpy(in_file_name, optarg, 1023);
        break;
      case 'd':
        disassemble = 1;
        strncpy(in_file_name, optarg, 1023);
        break;
      case 'h':
        info();
        break;
      case 'o':
        strncpy(out_file_name, optarg, 1023);
        break;
      case '?':
        printf("Illegal option -%c\n", optopt);
        info();
        return -1; 
      default:
        printf("Option -%c requires an argument.\n", optopt);
        info();
        return -1;
    }
  }
  printf("in %s out %s\n", in_file_name, out_file_name);
  FILE *file = fopen(in_file_name, "rb");
  if(!file) {
    printf("Error opnening file %s: %s\n", in_file_name, strerror(errno));
    return -1;
  }
  check_header(file);
  extract_files(file);
  fclose(file); 

}

