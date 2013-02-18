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
	fprintf(stdout, "Try \'uiextract -h\' for more information\n");
}

static void
usage()
{
	fprintf(stdout,
		"Unpack/integrity check tool for WebUI firmware.\n"
		"Usage: uiextract [-cx] <input file> -o <output dir>\n"
		"\t-c <file>        perform integrity check of <file>\n"
		"\t-x <file>        extract <file>. Default destination is current working dir\n"
		"\t-o <output dir>  output to <output dir>\n");
}

int32_t
ui_read_header(FILE * f, const ui_header_offset_t type,
	       webui_file_header * file_header)
{
	int32_t         retval = 1;
	fseek(f, ui_header_field[type], SEEK_SET);
	switch (type) {
	case UI_OFFSET_MAGIC:
		fread(&file_header->magic, 1, 4, f);
		// magic number
		break;
	case UI_OFFSET_CHECKSUM:
		fread(&file_header->checksum, 1, 4, f);
		// declared checksum
		break;
	case UI_OFFSET_SIZE_v1:
	case UI_OFFSET_SIZE_v2:
		fread(&file_header->size, 1, 4, f);
		// declared file size
		break;
	case UI_OFFSET_VERSION_v1:
	case UI_OFFSET_VERSION_v2:
		fread(&file_header->version, 1, 4, f);
		// WebUI firmware version
		break;
	case UI_OFFSET_DESC:
		fread(&file_header->desc, 1, 21, f);
		break;
	case UI_OFFSET_FIRST_FILE_v1:
	case UI_OFFSET_FIRST_FILE_v2:
		// seek to the first file
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
ui_check_version(FILE * f, webui_file_header * file_header)
{
	fseek(f, 0, SEEK_END);
	int32_t         file_size = ftell(f);
	// real file size
	int32_t         declared_file_size = 0;
	fread(&declared_file_size, 1, 4, f);
	if (file_size == declared_file_size)
		file_header->format_version = 1;
	else
		file_header->format_version = 2;
	return file_size;
}
int32_t
ui_valid_header(FILE * f, webui_file_header * file_header)
{
	int32_t         retval = 1;
	int32_t         file_size = ui_check_version(f, file_header);
	if (1 == file_header->format_version) {
		if (!ui_read_header(f, UI_OFFSET_SIZE_v1, file_header))
			return 0;
		if (!ui_read_header(f, UI_OFFSET_VERSION_v2, file_header))
			return 0;
		if (!ui_read_header(f, UI_OFFSET_FIRST_FILE_v2, file_header))
			return 0;
	} else if (2 == file_header->format_version) {
		if (!ui_read_header(f, UI_OFFSET_MAGIC, file_header))
			return 0;
		if (file_header->magic != WEBUI_MAGIC) {
			fprintf(stderr,
				"Declared file magic number doesn't match the known number: %#x/%#x\n",
				file_header->magic, WEBUI_MAGIC);
			retval = 0;
			return retval;	// no reason to continue
		}
		if (!ui_read_header(f, UI_OFFSET_CHECKSUM, file_header))
			return 0;
		int32_t         checksum = calc_checksum_file(f);
		if (file_header->checksum != checksum) {
			fprintf(stderr,
				"Declared checksum doesn't match the calculated checksum: %#x/%#x\n",
				file_header->checksum, checksum);
			retval = 0;
		}
		if (!ui_read_header(f, UI_OFFSET_SIZE_v2, file_header))
			return 0;

		if (!ui_read_header(f, UI_OFFSET_VERSION_v2, file_header))
			return 0;

		if (!ui_read_header(f, UI_OFFSET_FIRST_FILE_v2, file_header))
			return 0;
	} else {
		fprintf(stderr, "Unknown format\n");
		retval = 0;
	}
	if (file_header->size != file_size) {
		fprintf(stderr,
			"Declared file size doesn't match the real file size: %d/%d\n",
			file_header->size, file_size);
		retval = 0;
	}
	return retval;
}
int32_t
ui_read_entry(FILE * f, const ui_entry_data_type_t type, webui_entry * entry)
{
	int32_t         retval = 1;
	switch (type) {
	case UI_TYPE_FILENAME_SIZE:
	case UI_TYPE_FILE_SIZE:
		fread(&entry->size, 1, 0x4, f);
		// read filename length
		break;
	case UI_TYPE_FILENAME:
		fread(&entry->name, 1, entry->size, f);
		// read filename
		break;
	case UI_TYPE_ENTRY_TYPE:
		fread(&entry->type, 1, 0x1, f);
		// read entry type
		break;
	case UI_TYPE_FILE:
		fread(&entry->data, 1, entry->size, f);
		break;
	default:
		retval = 0;
		break;
	}
	if (feof(f)) {
		retval = 0;
	} else if (ferror(f)) {
		fprintf(stderr, "Error reading file entry: %s\n", strerror(errno));
		retval = 0;
	}
	return retval;
}
int32_t
ui_extract_files(FILE * f, const char *dst_path)
{
	webui_entry     wui_entry = { 0 };
	// max_buf = 0,
	char            dst_file[MAX_FILE_NAME_LEN];
	// *buf = NULL;

	while (1) {
		memset(wui_entry.name, 0, sizeof(wui_entry.name));
		memset(dst_file, 0, sizeof(dst_file));

		if (!ui_read_entry(f, UI_TYPE_FILENAME_SIZE, &wui_entry))
			return 0;

		if (!ui_read_entry(f, UI_TYPE_FILENAME, &wui_entry))
			return 0;
		sprintf(dst_file, "%s%s", dst_path, wui_entry.name);

		wui_entry.type = 0;
		if (!ui_read_entry(f, UI_TYPE_ENTRY_TYPE, &wui_entry))
			return 0;
		if (wui_entry.type == 0) {
			// type: dir
			if (mkdir(dst_file, 0770) != 0 && EEXIST != errno) {
				fprintf(stderr,
					"Unable to create directory %s: %s\n",
					dst_file, strerror(errno));
				return 0;
			}
		} else if (wui_entry.type == 1) {
			// type: file
			FILE           *file = fopen(dst_file, "wb");
			if (file == NULL) {
				fprintf(stderr, "Unable to write file %s: %s\n",
					dst_file, strerror(errno));
				return 0;
			}
			if (!ui_read_entry(f, UI_TYPE_FILE_SIZE, &wui_entry))
				return 0;
			/* 
			 * if(len > max_buf) { buf = realloc(buf, len);
			 * max_buf = len; if(buf == NULL) { fprintf(stderr,
			 * "Unable to allocate  data necessary to extract
			 * file.  Requested: %d bytes.\n", len); exit(-1); }
			 * } memset(buf, 0, sizeof(len));
			 */

			if (!ui_read_entry(f, UI_TYPE_FILE, &wui_entry))
				return 0;

			fprintf(stdout, "Extracting %s(%d bytes)...\n",
				wui_entry.name, wui_entry.size);
			fwrite(wui_entry.data, 1, wui_entry.size, file);
			fclose(file);
		}
	}
	// free(buf);
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
	char            dst_path[MAX_FILE_NAME_LEN] = { 0 };
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
	webui_file_header file_header = { 0 };
	if (!ui_valid_header(file, &file_header)) {
		return 1;
	}
	if (!validate_only) {
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
		ui_extract_files(file, dst_path);
	}
	fclose(file);
	fprintf(stdout,
		"\nIntegrity check passed:\n"
		"WebUI format\tv.%d\n"
		"Magic number\t%#x\n"
		"File length\t%d bytes\n"
		"Version    \t%d.%d.%d.%d\n"
		"Checksum   \t%#x\n", file_header.format_version, file_header.magic,
		file_header.size, (file_header.version >> (0 * 8)) & 0xFF,
		(file_header.version >> (1 * 8)) & 0xFF,
		(file_header.version >> (2 * 8)) & 0xFF,
		(file_header.version >> (3 * 8)) & 0xFF, file_header.checksum);
	return 0;
}
