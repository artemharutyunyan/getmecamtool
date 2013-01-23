/*
 
 Header goes here 

*/
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "camtool.h"
#include "common.h"

#define DEFAULT_PATH "./"
#define WEBUI_MAGIC 0x440C9ABD

static void usage() {
  printf(
      "Unpack/integrity check tool for WebUI firmware.\n"
      "Usage: disassemble [-cx] <input file> -o <output dir>\n"
      "-c <file>        perform integrity check of <file>\n"
      "-x <file>        extract <file>. Default destination is current working dir\n"
      "-o <output dir>  output to <output dir>\n" 
      );
}

int calc_checksum(FILE* f)
{
  fseek(f, OFFSET_VERSION, SEEK_SET);
  int sum = 0;
  unsigned char buf;
  while(1){
    fread(&buf, 1, 1, f);
    if(feof(f))
      break;
    sum += buf;
  }
  fseek(f, OFFSET_VERSION, SEEK_SET); // get back to where we were
  return sum;
}

int check_header(FILE *f, webui_file_header *file_header)
{
  int32_t checksum = 0;
  int retval = 1;

  fseek(f, 0, SEEK_END); 
  int32_t file_size = ftell(f); // real file size

  fseek(f, OFFSET_MAGIC, SEEK_SET); // rewind

  fread(&file_header->magic, 1, 4, f); // magic number
  if(file_header->magic != WEBUI_MAGIC) {
    printf("Declared file magic number doesn't match the known number: %d/%d\n",
        file_header->magic, WEBUI_MAGIC);
    retval = 0;
  }

  fseek(f, OFFSET_CHECKSUM, SEEK_SET);
  fread(&file_header->checksum, 1, 4, f); // declared checksum

  fseek(f, OFFSET_SIZE, SEEK_SET);
  fread(&file_header->size, 1, 4, f); // declared file size
  if(file_header->size != file_size) {
    printf("Declared file size doesn't match the real file size: %d/%d\n",
        file_header->size, file_size);
    retval = 0;
  }
  
  fseek(f, OFFSET_VERSION, SEEK_SET);
  fread(&file_header->version, 1, 4, f); // WebUI firmware version

  checksum = calc_checksum(f); // do it here since we are at offset 12 already
  if(file_header->checksum != checksum) {
    printf("Declared checksum doesn't match the real checksum: %#x/%#x\n",
        file_header->checksum, checksum);
    retval = 0;
  }

  fseek(f, OFFSET_FIRST_FILE, SEEK_SET); // seek to the first file
  return retval;
}

int extract_files(FILE *f, const char *dst_path)
{
  int   len = 0,
        max_buf = 0,
        type = 0;
  char  file_name[MAX_FILE_NAME_LEN],
        dst_file[MAX_FILE_NAME_LEN],
        *buf = NULL;

  while(1) {
    memset(file_name, 0, sizeof(file_name));
    memset(dst_file, 0, sizeof(dst_file));

    fread(&len, 1, 4, f); // read filename length
    if(feof(f))
      break;
    fread(file_name,1,len,f); // read filename
    if(feof(f))
      break;
    sprintf(dst_file, "%s%s", dst_path, file_name);

    type = 0;
    fread(&type, 1, 1, f); // read entry type
    if(feof(f))
      break;
    if (type == 0) { // type: dir
      if (mkdir(dst_file, 0770) != 0 && EEXIST != errno) {
        fprintf(stderr, "Unable to create directory %s: %s\n", dst_file, strerror(errno));
        exit(-1);
      }
    } else if (type == 1) { // type: file
      FILE *file = fopen(dst_file, "wb");
      if (file == NULL) {
        fprintf(stderr, "Unable to write file %s: %s\n", dst_file, strerror(errno));
        exit(-1);
      }

      fread(&len, 1, 4, f); // read file length
      if(feof(f))
        break;
      if (len > max_buf) {
        buf = realloc(buf, len);
        max_buf = len;
        if (buf == NULL) {
          fprintf(stderr, "Unable to allocate  data necessary to extract file.  Requested: %d bytes.\n", len);          exit(-1);
        }
      }
      memset(buf, 0, sizeof(len));
      fread(buf,1,len,f);
      if(feof(f))
        break;

      fprintf(stdout, "Extracting %s (%d bytes)...\n", file_name, len);
      fwrite(buf, 1, len, file);
      fclose(file);
    }
  }
  free(buf);
  return 1;
}
int main(int argc, char **argv) {

  if (argc < 2) {
    usage();
    return -1; 
  }
  char o;
  int check = 0;
  char in_file_name[MAX_FILE_NAME_LEN] = { 0 };
  char dst_path[MAX_FILE_NAME_LEN] = { 0 };
  while((o = getopt(argc, argv, ":c:x:ho:")) != -1) {
    switch (o) {
      case 'c':
        check = 1;
        strncpy(in_file_name, optarg, 255);
        break;
      case 'x':
        strncpy(in_file_name, optarg, 255);
        break;
      case 'h':
        usage();
        break;
      case 'o':
        strncpy(dst_path, optarg, 1023);
        break;
      case '?':
        printf("Illegal option -%c\n", optopt);
        usage();
        return -1; 
      default:
        printf("Option -%c requires an argument.\n", optopt);
        usage();
        return -1;
    }
  }
  
  if(!strlen(dst_path)) {
    strncpy(dst_path, DEFAULT_PATH, sizeof(DEFAULT_PATH));
  }

  FILE *file = fopen(in_file_name, "rb");
  if(!file) {
    printf("Error opening file %s: %s\n", in_file_name, strerror(errno));
    return -1;
  }
  webui_file_header file_header = {0, 0, 0, 0};
  if(!check_header(file, &file_header)) {
    return -1;
  }
  if(!check)
    extract_files(file, dst_path);
  fclose(file);

  printf(
      "Untegrity check passed:\n"
      "magic number\t%#x\n"
      "file length\t%d bytes\n"
      "version    \t%d.%d.%d.%d\n"
      "checksum   \t%#x\n"
      , file_header.magic
      , file_header.size
      , (file_header.version >> (0 * 8)) & 0xFF
      , (file_header.version >> (1 * 8)) & 0xFF
      , (file_header.version >> (2 * 8)) & 0xFF
      , (file_header.version >> (3 * 8)) & 0xFF
      , file_header.checksum
      );
}

