/*
 
 Header goes here 

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <inttypes.h>
#include <getopt.h>

#include "camtool.h"
#include "common.h"

static int conf_copy_file(const char* src, const char* dst); 
static int conf_process_settings(const char* src, const char* dst, const char* settings);
static int conf_get_setting_line(char* buf, uint16_t* delim_start, uint16_t* delim_end, FILE* fd);
static int conf_update_checksum(FILE* out); 
static int conf_update_port(FILE* out, const char* port_str); 

static void 
usage()
{
  fprintf(stdout,
  "Tool for overwriting configuration settings (at this time only ovewriting port number is supported).\n"
  "Usage: confpack -f <original> -o <new> -s <settings>\n"
  "\t-f <original> original settings binary to use\n"
  "\t-o <new> new binary to produce\n"
  "\t-s <settings> settings file to use\n"
  "\t-h prints this message\n");  
}

int 
main( int argc, char** argv) 
{

  if (argc < 4) {
    usage();
    return 1;
  }

  char original_file[MAX_FILE_NAME_LEN];
  original_file[0] = '\0';

  char new_file[MAX_FILE_NAME_LEN];
  new_file[0] = '\0';

  char settings[MAX_FILE_NAME_LEN];
  settings[0] = '\0';

  char o;
  while ((o = getopt(argc, argv, ":f:o:s:h")) != -1) {
    switch(o) {
    case 'f':
      if (strlen(optarg) > MAX_FILE_NAME_LEN) {
        fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
        return 1;
      }
      strncpy(original_file, optarg, MAX_FILE_NAME_LEN);
      break;
    case 'o':
      if (strlen(optarg) > MAX_FILE_NAME_LEN) {
        fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
        return 1;
      }
      strncpy(new_file, optarg, MAX_FILE_NAME_LEN);
      break;
    case 's':
      if (strlen(optarg) > MAX_FILE_NAME_LEN) {
        fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
        return 1;
      }
      strncpy(settings, optarg, MAX_FILE_NAME_LEN);
      break;
 
    case 'h':
      usage();
      return 0;
    case '?':
      fprintf(stderr, "Illegal option -%c\n", optopt);
      usage();
      return 1;
    defalt:
      fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      usage();
      return 1;
    }
  }

  if (strlen(original_file) == 0 || strlen(new_file) == 0 || strlen(settings) == 0)  {
    usage();
    return 1;
  }

  if(conf_process_settings(original_file, new_file, settings) != 0) {
    fprintf(stderr, "Error while preparing new settings file\n");
    return 1;
  } 

  return 0;
}

/* Internal Functions */

/// \brief reads a line from config file and sets delimiter positions 
static int
conf_get_setting_line(char* buf, uint16_t* delim_start, uint16_t* delim_end, FILE* fd) 
{
  uint32_t len = 0;
  char c;
  uint16_t delim_count = 0;
  
  c = getc(fd);
  if (c == EOF)
    return EOF;

  while ((c != '\n') && (c != EOF)) {
    if (len == MAX_SETTING_LINE_LEN) {
      fprintf(stderr, "Config file line %s... too long\n", buf);
      return 1;
    }
    
    buf[len] = c;

    if (c == SETTING_FILE_DELIM)
      ++delim_count;

    if (delim_count == 1) 
      *delim_start = len;
    else if (delim_count == SETTING_FILE_DELIM_COUNT) {
      *delim_end = len;  
      delim_count = SETTING_FILE_DELIM_COUNT + 1;
    }
  
    c = getc(fd);
    ++len;
  }

  buf[len] = '\0';
  if (*delim_end - *delim_start != SETTING_FILE_DELIM_COUNT - 1) {
    fprintf(stderr, "No delimiter found on config file line %s\n", buf);
    return 1;
  }

  return 0; 
}

/// \brief recalculates the checksum and writes it to the file
static int 
conf_update_checksum(FILE* out) 
{
  int ret = 0;
  if ((ret = fflush(out)) != 0) {
    fprintf(stderr, "Could not flush output file handle for calculating checksum");
    return ret;
  }

  // Calculate the new checkum
  uint32_t checksum = calc_checksum_file(out, conf_header_field[CONF_OFFSET_CAMID]);
  
  // Seek to checksum position  
  if ((ret = fseek(out, conf_header_field[CONF_OFFSET_CHECKSUM], SEEK_SET)) != 0) {
    fprintf(stderr, "Failed to seek to checksum position (%d) in output file. Error is %s\n", 
            CONF_OFFSET_CHECKSUM, 
            strerror(errno));
    return ret;    
  }

  // Update the checksum
  if (fwrite(&checksum, 4, 1, out) != 1 || ferror(out) != 0) {
    fprintf(stderr, "Failed to write updated checksum to output file at position %d.\n", 
            CONF_OFFSET_CHECKSUM); 
    return 1; 
  }

  return ret; 
}

/// \brief bumps the port number by 1 and writes it to the file 
static int
conf_update_port(FILE* out, const char* port_str) {

  // Make sure port number is sane 
  int port = atoi(port_str);
  if (port <= 0 || (port > (1 << 16) - 1)) {
    fprintf(stderr, "Failed to convert port value (%s %d) to a valid port number.\n", 
            port_str, port);
    return 1; 
  }

  // Increment port number 
  ++port;

  // Seek to port position 
  uint32_t port_offset = conf_sections_field[CONF_OFFSET_NETWORK] + offsetof(conf_network, port);
  if (fseek(out, port_offset, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to seek to port position (%d) in output file. Error is %s\n", 
            port_offset,
            strerror(errno));
    return 1; 
  }
  
  // Write the port back
  if (fwrite(&port, 2, 1, out) != 1 || ferror(out) != 0) {
    fprintf(stderr, "Failed to write updated port number to output file at position %d.\n", 
            port_offset); 
    return 1; 
  }       

  return 0;
}

/// \brief processes settings file and rewrites corresponding values in the output file 
static int 
conf_process_settings(const char* src_file, const char* out_file, const char* settings_file) 
{
  int ret = 1;

  // Copy original settings binary  
  if ((ret = conf_copy_file(src_file, out_file)) != 0) {
    fprintf(stderr, "Could not copy original binary file\n");
    return ret;     
  }

  FILE* settings = NULL;
  FILE* out = NULL;

  // Open settings file (text) for reading 
  settings = fopen(settings_file, "r");
  if (settings == NULL) {
    fprintf(stderr, "Failed to open settings file %s: %s\n",
            settings_file,
            strerror(errno));
    goto cleanup;
  }

  // Open output file for updating 
  out = fopen(out_file, "rb+");
  if (out == NULL) {
    fprintf(stderr, "Failed to open output file %s: %s\n",
            out_file,
            strerror(errno));
    goto cleanup; 
  }

  char line[MAX_SETTING_LINE_LEN];
  uint16_t delim_start, delim_end;

  // Iterate over settings file 
  while ((ret = conf_get_setting_line(line, &delim_start, &delim_end, settings)) == 0) {
    
  if (strncmp(line, "port", delim_start) == 0) { 
      // Found port 
      if ((ret = conf_update_port(out, &line[delim_end + 1])) != 0) 
        goto cleanup;
    }
  }
 
  if (ret != EOF) {
    fprintf(stderr, "Error while reading settings from file %s\n", settings_file);
    goto cleanup;
  }
  
  ret = conf_update_checksum(out);
 
cleanup:
  fclose(settings); 
  fclose(out);
  return ret;  
}

/// \brief Copies original settings file 
static int
conf_copy_file(const char* src_file, const char* dst_file) 
{

  // Open original file for reading 
  FILE* src = fopen(src_file, "rb");
  if (src == NULL) {
    fprintf(stderr, "Failed to open input file %s: %s\n",
            src_file,
            strerror(errno));
    return 1;
  }
  
  // Get the size of the original file 
  uint32_t src_size = fseek(src, 0, SEEK_END);
  src_size = ftell(src);
  if (src_size > MAX_FILE_SIZE) {
    fprintf(stderr, "Size of file (%d) exceeds MAX_FILE_SIZE(%d)\n",
            src_size,
            MAX_FILE_SIZE);
    fclose(src);
    return -1;
  }
  rewind(src);

  // Open output file for writing 
  FILE* dst = fopen(dst_file, "wb+");
  if (dst == NULL) {
    fprintf(stderr, "Failed to open output file %s: %s\n",
            dst_file,
            strerror(errno));
    fclose(src);
    return 1;
  }

  // Copy original file 
  if (copy_file(src, dst, src_size) != 0) {
     fprintf(stderr, "Failed to copy %s to %s:%s\n",
            src_file,
            dst_file,
            strerror(errno));
    fclose(src);
    fclose(dst);
    return 1; 
  } 

  fclose(src);
  fclose(dst);

  return 0;
}

