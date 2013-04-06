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

#include "camtool.h"
#include "common.h"

// when change, update ui_header_offset_t too
const int32_t   ui_header_field[] = {
    0x0,
    0x4,
    0x8,
    0x1D,
    0x0,
    0x4,
    0x8,
    0xC,
    0x10
};

// when change, update sys_header_offset_t too
const int32_t   sys_header_field[] = {
    0x0,
    0x4,
    0x8,
    0xC,
    0x10
};

// when change, update conf_header_offset_t too
const int32_t   conf_header_field[] = {
    0x0,
    0x4,
    0x8,
    0xC,
    0x19,
    0x1D,
    0x21
};

// when change, update conf_sections_offset_t too
const int32_t   conf_sections_field[] = {
    0x36,   // users section
    0x10E,  // network
    0x124,  // adsl
    0x1A7,  // wifi
    0x293,   // ntp
    0x518   // e-mail
};

/* Takes the open file descriptor pointing to a Web UI file as an input and
 * calculates the checksum
 *
 * \param[in] f open file descriptor
 * \param[in] offset offset to calculate checksum from
 * \return The checksum (sum of all the bytes that follow file header)
 */
int32_t
calc_checksum_file(FILE* f, const size_t offset)
{
    fseek(f, offset, SEEK_SET);
    int32_t         sum = 0;
    unsigned char   buf;

    while (1) {
        fread(&buf, 1, 1, f);
        if (feof(f))
            break;
        sum += buf;
    }

    return sum;
}

/* Takes a webui_data_blob populated with the Web UI data as an input
 * and calculates the checksum
 *
 * \param[in] blob the blob containing Web UI data
 * \param[in] offset offset to calculate checksum from
 * \return The checksum (sum of all the bytes that follow file header)
 */
int32_t 
calc_checksum_blob(const webui_data_blob* blob, const size_t offset)
{
    int32_t sum = 0;
    size_t i = offset;

    while (i < blob->size) {
        sum += blob->data[i];
        ++i;
    }

    return sum;
}

/* Given two file handles copies the contents of one file to another
 *
 * \param[in] src source file handle
 * \param[in] dst destination file handle
 * \param[in] size size of the file to copy
 * \return 0 in case of success 1 otherwise
 */
int32_t 
copy_file(FILE* src, FILE* dst, const uint32_t size) 
{
    // Read contents of a file
    char buf[MAX_FILE_SIZE];
    uint32_t n;
    n = fread(buf, 1, size, src);
    if (n != size) {
        fprintf(stderr, "Could not copy a file. Read only %d out of %d bytes",
                n,
                size);
        return 1;
    }

    // Write contents of a file
    n = fwrite(buf, 1, size, dst);
    if (n != size) {
        fprintf(stderr, "Could not write to a file. Wrote only %d bytes out of %d",
                n,
                size);
        return 1;
    }

    return 0;
}

