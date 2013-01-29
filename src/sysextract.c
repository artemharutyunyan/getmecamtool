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
		"Unpack/integrity check tool for WebUI firmware.\n"
		"Usage: uiextract [-cx] <input file> -o <output dir>\n"
		"-c <file>        perform integrity check of <file>\n"
		"-x <file>        extract <file>. Default destination is current working dir\n"
		"-o <output dir>  output to <output dir>\n"
	);
}
int 
main(int argc, char **argv)
{

	if (argc < 2) {
		usage();
		return -1;
	}
	char		o;
	int		check = 0;
	char		in_file_name[MAX_FILE_NAME_LEN] = {0};
	char		dst_path  [MAX_FILE_NAME_LEN] = {0};
	while ((o = getopt(argc, argv, ":c:x:ho:")) != -1) {
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
			fprintf(stderr, "Illegal option -%c\n", optopt);
			usage();
			return -1;
		default:
			fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			usage();
			return -1;
		}
	}
  return 1;
}
