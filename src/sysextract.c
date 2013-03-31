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
	fprintf(stdout, "Try \'sysextract -h\' for more information\n");
}

static void
usage()
{
	fprintf(stdout,
		"Unpack/integrity check tool for system firmware.\n"
		"Usage: sysextract [-cx] <input file> -o <output dir>\n"
		"-c <file>        perform integrity check of <file>\n"
		"-x <file>        extract <file>. Default destination is current working dir\n"
		"-o <output dir>  output to <output dir>\n");
}
int32_t
sys_read_header(FILE * f, const sys_header_offset_t type,
		sys_file_header * file_header)
{
	int32_t         retval = 1;
	fseek(f, sys_header_field[type], SEEK_SET);
	switch (type) {
	case SYS_OFFSET_MAGIC:
		fread(&file_header->magic, 1, 4, f);
		// magic number
		break;
	case SYS_OFFSET_RESERVE1:
		fread(&file_header->reserve1, 1, 4, f);
		// some reserved field
		break;
	case SYS_OFFSET_RESERVE2:
		fread(&file_header->reserve2, 1, 4, f);
		// some reserved field
		break;
	case SYS_OFFSET_SIZE_LINUX_BIN:
		fread(&file_header->size_linux, 1, 4, f);
		// size of linux.bin
		break;
	case SYS_OFFSET_SIZE_ROMFS:
		fread(&file_header->size_romfs, 1, 4, f);
		// size of romfs
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
int32_t
sys_validate_header(FILE * f, sys_file_header * file_header)
{
	fseek(f, 0, SEEK_END);
	int32_t         file_size = ftell(f);
	if (file_size <= 0) {
		fprintf(stderr, "Invalid file size\n");
		return 0;
	}
	fseek(f, 0, SEEK_SET);
	if (!sys_read_header(f, SYS_OFFSET_MAGIC, file_header))
		return 0;
	if (file_header->magic != SYS_MAGIC) {
		fprintf(stderr,
			"Declared file magic number doesn't match the known number: %#x/%#x\n",
			file_header->magic, SYS_MAGIC);
		return 0;
	}
	if (!sys_read_header(f, SYS_OFFSET_RESERVE1, file_header))
		return 0;
	if (!sys_read_header(f, SYS_OFFSET_RESERVE2, file_header))
		return 0;
	if (!sys_read_header(f, SYS_OFFSET_SIZE_LINUX_BIN, file_header))
		return 0;
	if (!sys_read_header(f, SYS_OFFSET_SIZE_ROMFS, file_header))
		return 0;
	if ((sizeof(sys_file_header) + file_header->size_linux +
	     file_header->size_romfs) != file_size) {
		fprintf(stderr,
			"Declared sizes of linux.bin: %d and romfs: %d doesn't match the real data size: %d\n",
			file_header->size_linux, file_header->size_romfs,
			file_size - (int32_t) sizeof(sys_file_header));
		return 0;
	}
	fprintf(stdout,
		"System firmware file has valid structure\nlinux.bin size: %d, romfs.img size: %d\n",
		file_header->size_linux, file_header->size_romfs);
	return 1;
}
int32_t
sys_extract_file(FILE * f, const int32_t offset, const int32_t len,
		 const char *file_name)
{
	char           *buf = malloc(len);
	if (buf) {
		fseek(f, offset, SEEK_SET);
		fread(buf, 1, len, f);
		if (feof(f)) {
			return 0;
		} else if (ferror(f)) {
			fprintf(stderr, "Error reading file: %s\n", strerror(errno));
			return 0;
		}
		FILE           *file = fopen(file_name, "wb");
		if (file == NULL) {
			fprintf(stderr, "Unable to write file %s: %s\n", file_name,
				strerror(errno));
			return 0;
		}
		fprintf(stdout, "Extracting %s(%d bytes)...\n", file_name, len);
		fwrite(buf, 1, len, file);
		fclose(file);
		free(buf);
	} else {
		fprintf(stderr, "Cannot allocate requested memory: %d bytes\n", len);
		return 0;
	}
	return 1;
}
int32_t
sys_extract_files(FILE * f, const char *dst_path)
{
	sys_file_header file_header = { 0 };
	if (!sys_validate_header(f, &file_header)) {
		return 0;
	}
	char            dst_file[MAX_FILE_NAME_LEN];
	memset(dst_file, '\0', sizeof(dst_file));
	sprintf(dst_file, "%s%s", dst_path, "linux.bin");
	if (!sys_extract_file
	    (f, sizeof(sys_file_header), file_header.size_linux, dst_file))
		return 0;
	memset(dst_file, '\0', sizeof(dst_file));
	sprintf(dst_file, "%s%s", dst_path, "romfs.img");
	if (!sys_extract_file
	    (f, sizeof(sys_file_header) + file_header.size_linux,
	     file_header.size_romfs, dst_file))
		return 0;

	return 1;
}

int
main(int argc, char **argv)
{

	if (argc < 2) {
		usage();
		return 1;
	}
	char            o;
	int             validate_only = 0;
	int             index;
	char            in_file_name[MAX_FILE_NAME_LEN] = { 0 };
	char            dst_path[MAX_FILE_NAME_LEN] = { 0 };
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
			strncpy(dst_path, optarg, MAX_PATH_LEN);
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

	if (!strlen(dst_path)) {
		strncpy(dst_path, DEFAULT_PATH, sizeof(DEFAULT_PATH));
	}
	FILE           *file = fopen(in_file_name, "rb");
	if (!file) {
		fprintf(stderr, "Error opening file %s: %s\n", in_file_name,
			strerror(errno));
		return 1;
	}
	if (validate_only) {
		sys_file_header file_header = { 0 };
		if (!sys_validate_header(file, &file_header)) {
			return 1;
		}
	} else {
		if (mkdir(dst_path, 0770) != 0) {
			if (EEXIST != errno) {
				fprintf(stderr,
					"Unable to create directory %s: %s\n",
					dst_path, strerror(errno));
				return 1;
			}
		} else {
			fprintf(stdout, "Created directory %s\n", dst_path);
		}
		if (!sys_extract_files(file, dst_path)) {
			fprintf(stderr, "Cannot extract system files, exiting...\n");
		}
	}

	return 0;
}
