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
info()
{
	fprintf(stdout, "Try \'confextract -h\' for more information\n");
}

static void
usage()
{
	fprintf(stdout,
		"Extraction/integrity check tool for camera configuration.\n"
		"Usage: confextract [-cx] <input file> -o <output dir>\n"
		"\t-c <file>        perform integrity check of <file>\n"
		"\t-x <file>        extract <file>. Default destination is current working dir\n"
		"\t-o <output file>  output to <output file>\n");
}

int32_t
conf_extract_conf(FILE * f, FILE * out_f)
{
  return 1;
}

int32_t
conf_valid_file(FILE * f, conf_file * conf)
{
  return 1;
}
int32_t
main(int argc, char **argv)
{

	if (argc < 2) {
		usage();
		return 1;
	}
	char            o;
	int32_t         validate_only = 0;
	int32_t         index;
	char            in_file_name[MAX_FILE_NAME_LEN] = { 0 };
	char            dst_file_name[MAX_FILE_NAME_LEN] = { 0 };
	while ((o = getopt(argc, argv, ":c:x:ho:")) != -1) {
		switch (o) {
		case 'c':
			validate_only = 1;
			strncpy(in_file_name, optarg, MAX_FILE_NAME_LEN);
			break;
		case 'x':
			strncpy(in_file_name, optarg, MAX_FILE_NAME_LEN);
			break;
		case 'h':
			usage();
      return 0;
		case 'o':
      strncpy(dst_file_name, optarg, MAX_FILE_NAME_LEN);
			break;
		case '?':
			fprintf(stderr, "Illegal option -%c\n", optopt);
			info();
			return 1;
		default:
      fprintf(stderr, "Option -%c requires an argument.\n",
          optopt);
      info();
      return 1;
		}
	}

	for (index = optind; index < argc; index++) {
		fprintf(stderr, "Non-option argument %s\n", argv[index]);
		info();
		return 1;
	}

	if (!strlen(dst_file_name)) {
		strncpy(dst_file_name, DEFAULT_FILE, sizeof(DEFAULT_FILE));
	}
	FILE           *file = fopen(in_file_name, "rb");
	if (!file) {
		fprintf(stderr, "Error opening file %s: %s\n", in_file_name,
			strerror(errno));
		return 1;
	}
	conf_file conf = { 0 };
	if (!conf_valid_file(file, &conf)) {
		return 1;
	}
	if (!validate_only) {
    FILE           *dst_file = fopen(dst_file_name, "wb");
    if (file == NULL) {
      fprintf(stderr, "Unable to write file %s: %s\n",
          dst_file_name, strerror(errno));
      return 1;
    } else {
      fprintf(stdout, "Created output file %s\n", dst_file_name);
    }
		conf_extract_conf(file, dst_file);
    fclose(dst_file);
	}
	fclose(file);
	return 0;
}

