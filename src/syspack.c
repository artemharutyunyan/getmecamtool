/*
 * Copyright 2013 Artem Harutyunyan, Sergey Shekyan
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <getopt.h>

#include "camtool.h"
#include "common.h"

static FILE* sys_fopen_read(const char* fname, uint32_t* s);
static int sys_create_file (const char* kernel, const char* romfs, const char* out); 

static void 
usage()
{
    fprintf(stdout,
            "Tool for packing system firmware.\n"
            "Usage: syspack -k <kernel> -i <romfs image>\n"
            "\t-k <kernel> a path to a kernel file to pack\n"
            "\t-i <romfs image> a path to romfs image\n"
            "\t-o <output file> output file name\n"
            "\t-h prints this message\n");
}

int 
main( int argc, char** argv) 
{

    if (argc < 4) {
        usage();
        return 1;
    }

    char kernel_file_path[MAX_FILE_NAME_LEN];
    kernel_file_path[0] = '\0';

    char romfs_file_path[MAX_FILE_NAME_LEN];
    romfs_file_path[0] = '\0';

    char out_file_name[MAX_FILE_NAME_LEN];
    out_file_name[0] = '\0';


    char o;
    while ((o = getopt(argc, argv, ":k:i:o:h")) != -1) {
        switch(o) {
        case 'k':
            if (strlen(optarg) > MAX_FILE_NAME_LEN) {
                fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
                return 1;
            }
            strncpy(kernel_file_path, optarg, MAX_FILE_NAME_LEN);
            break;
        case 'i':
            if (strlen(optarg) > MAX_FILE_NAME_LEN) {
                fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
                return 1;
            }
            strncpy(romfs_file_path, optarg, MAX_FILE_NAME_LEN);
            break;
        case 'o':
            if (strlen(optarg) > MAX_FILE_NAME_LEN) {
                fprintf(stderr, "%s can not be longer than %d\n", optarg, MAX_FILE_NAME_LEN);
                return 1;
            }
            strncpy(out_file_name, optarg, MAX_FILE_NAME_LEN);
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

    if (strlen(kernel_file_path) == 0 || strlen(romfs_file_path) == 0 || strlen(out_file_name) == 0)  {
        usage();
        return 1;
    }

    if(sys_create_file(kernel_file_path, romfs_file_path, out_file_name) != 0) {
        fprintf(stderr, "Error while packing system file");
        return 1;
    }

    return 0;
}

/* Internal Functions */

/// \brief Copies the kernel and romfs to the output file and creates a header
int
sys_create_file (const char* kernel, const char* romfs, const char* out) 
{

    // Open output file for writing
    const char* tmp = tmpnam(NULL);
    FILE* out_fd = fopen (tmp, "wb+");
    if (out_fd == NULL) {
        fprintf(stderr, "Failed to open output file %s: %s\n",
                tmp,
                strerror(errno));
        return 1;
    }
    fseek(out_fd, sizeof(sys_file_header), SEEK_SET);

    // Open kernel file
    FILE* kernel_fd;
    uint32_t kernel_size = 0;
    if ((kernel_fd = sys_fopen_read(kernel, &kernel_size)) == NULL) {
        fclose(out_fd);
        return 1;
    }

    // Copy kernel to the output file
    if (copy_file(kernel_fd, out_fd, kernel_size) != 0) {
        fclose(out_fd);
        fclose(kernel_fd);
        return 1;
    }
    fclose(kernel_fd);

    // Open romfs file
    FILE* romfs_fd;
    uint32_t romfs_size = 0;
    if ((romfs_fd = sys_fopen_read(romfs, &romfs_size)) == NULL) {
        fclose(out_fd);
        fclose(kernel_fd);
        return 1;
    }

    // Copy romfs to the output file
    if (copy_file(romfs_fd, out_fd, romfs_size) != 0) {
        fclose(out_fd);
        fclose(romfs_fd);
        return 1;
    }
    fclose(romfs_fd);

    // Populate the header
    sys_file_header h;
    h.magic = SYS_MAGIC;
    h.reserve1 = SYS_RESERVED;
    h.reserve2 = SYS_RESERVED;
    h.size_linux = kernel_size;
    h.size_romfs = romfs_size;

    // Write the header
    fseek(out_fd, 0, SEEK_SET);
    if (fwrite(&h, 1, sizeof(h), out_fd) != sizeof(h)) {
        fprintf (stderr, "Could not write the header to the output file");
        fclose(out_fd);
        return 1;
    }

    fclose(out_fd);

    // Everything went well. Atomically rename the resulting file
    if (rename(tmp, out) != 0) {
        fprintf(stderr, "System UI was generated (%s) but the output (%s) file could not be written: %s",
                tmp,
                out,
                strerror(errno));
    }

    return 0;
} 


/// \brief Given a file name returns a read handle and performs sanity checks 
static FILE* 
sys_fopen_read(const char* fname, uint32_t* s) 
{
    FILE* fd = fopen(fname, "rb");
    uint32_t size;
    if (fd == NULL) {
        fprintf(stderr, "Failed to open file %s: %s\n",
                fname,
                strerror(errno));
        return NULL;
    }

    // Get kernel file size
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    if (size >= MAX_FILE_SIZE) {
        fprintf(stderr, "Size of file (%d) exceeds MAX_FILE_SIZE(%d)\n",
                size,
                MAX_FILE_SIZE);
        fclose(fd);
        return NULL;
    }

    rewind(fd);
    *s = size;
    return fd;
}

