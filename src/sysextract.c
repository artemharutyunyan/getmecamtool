/*
 * Header goes here
 * 
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

static void 
usage()
{
	fprintf(stdout,
		"Unpack/integrity check tool for system firmware.\n"
		"Usage: sysextract [-cx] <input file> -o <output dir>\n"
		"-c <file>        perform integrity check of <file>\n"
		"-x <file>        extract <file>. Default destination is current working dir\n"
		"-o <output dir>  output to <output dir>\n"
	);
}

int32_t
sys_valid_header(FILE * f, sys_file_header * file_header)
{ 
  
	int32_t		retval = 1;

  return 0;
}

int32_t
sys_extract_files(FILE * f, const char *dst_path)
{
  return 0;
}

int32_t
sys_read_header(FILE * f, const sys_header_offset_t type, sys_file_header * file_header)
{
 	int32_t		retval = 1;
	fseek(f, sys_header_field[type], SEEK_SET);
	switch (type) {
	case SYS_OFFSET_MAGIC:
		fread(&file_header->magic, 1, 4, f);
		//magic number
			break;
	case SYS_OFFSET_RESERVE1:
		fread(&file_header->reserve1, 1, 4, f);
		//some reserved field
			break;
	case SYS_OFFSET_RESERVE2:
		fread(&file_header->reserve2, 1, 4, f);
		//some reserved field
			break;
  case SYS_OFFSET_SIZE_LINUX_BIN:
      fread(&file_header->size_linux, 1, 4, f);
      //size of linux.bin
      break;
  case SYS_OFFSET_SIZE_ROMFS:
      fread(&file_header->size_romfs, 1, 4, f);
      //size of romfs
      break;
	default:
		retval = 0;
		break;
	}
	if (feof(f)) {
		retval = 0;
	} else if (ferror(f)) {
		fprintf(stderr, "Error reading file: %s\n", strerror(errno));
		retval = 0;
	}
	return retval;
}
int 
main(int argc, char **argv)
{

	if (argc < 2) {
		usage();
		return 1;
	}
	char		o;
	int		  validate_only = 0;
  int     index;
	char		in_file_name[MAX_FILE_NAME_LEN] = {0};
	char		dst_path  [MAX_FILE_NAME_LEN] = {0};
	while ((o = getopt(argc, argv, ":c:x:ho:")) != -1) {
		switch (o) {
		case 'c':
			validate_only = 1;
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
			fprintf(stderr, "Illegal option -%c\n", optopt);
			usage();
			return 1;
		default:
			fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			usage();
			return 1;
		}
	}
  for (index = optind; index < argc; index++) {
    fprintf (stderr, "Non-option argument %s\n", argv[index]);
    usage();
    return 1;
  }

	if (!strlen(dst_path)) {
		strncpy(dst_path, DEFAULT_PATH, sizeof(DEFAULT_PATH));
	}
	FILE           *file = fopen(in_file_name, "rb");
	if (!file) {
		fprintf(stderr, "Error opening file %s: %s\n", in_file_name, strerror(errno));
		return 1;
	}

	sys_file_header file_header = {0};
	if (!sys_valid_header(file, &file_header)) {
		return 1;
	}
	if (!validate_only) {
		if (mkdir(dst_path, 0770) != 0) {
			if (EEXIST != errno) {
				fprintf(stderr, "Unable to create directory %s: %s\n", dst_path, strerror(errno));
				exit(-1);
			}
		} else {
			fprintf(stdout, "Created directory %s\n", dst_path);
		}
		sys_extract_files(file, dst_path);
	}

  return 0;
}
