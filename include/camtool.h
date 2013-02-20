/*
 * TODO: Add a meaningful header here
 *
 */

#ifndef __CAMTOOL_H_
#define __CAMTOOL_H

#include <stdint.h>


// Constant definitions
#define MAX_PATH_LEN 1024
#define MAX_FILE_NAME_LEN 1024

#define MAX_FILE_SIZE 1048576

// !Web UI data blob
typedef struct webui_data_blob_t {
	size_t          size;
	size_t          alloc_size;
	char           *data;
} webui_data_blob;

///!Web UI file header v .1 & 2
#pragma pack(push, 4)
typedef struct webui_file_header_t {
	int32_t         magic;	        /* !< Magic number */
	int32_t         checksum;	      /* !< Checksum (sum of all bytes starting from offset 12) */
	int32_t         size;	          /* !< Size of file */
	int32_t         version;	      /* !< Version */
	int32_t         format_version;	/* !< WebUI format version */
	char            desc[21];	      /* !< Description */
} webui_file_header;
#pragma pack(pop)

///!sys file header for FI8910W and similar
#pragma pack(push, 4)
typedef struct sys_file_header_t {
	int32_t         magic;	/* !< Magic number */
	int32_t         reserve1;	/* !< something 1 */
	int32_t         reserve2;	/* !< something 2 */
	int32_t         size_linux;	/* !< Size of linux.bin */
	int32_t         size_romfs;	/* !< Size of romfs */
} sys_file_header;
#pragma pack(pop)

//! Header for files that are packed into Web UI data blob
typedef struct webui_fentry_t {
  int32_t name_size;  /*!< Size of file name string */ 
  char*   name;       /*!< Name of the file */ 
  char    type;       /*!< Type of the entry (set 01 in case of files) */
  int32_t size;       /*!< Size of the file */
  char*   data;       /*!< File data */
} webui_fentry; 

//! Header for directories that are packed into Web UI data blob
typedef struct webui_dentry_t {
  int32_t name_size;  /*!< Size of file name string */ 
  char*   name;       /*!< Name of the file */ 
  char    type;      /*!< Type of the entry (set 00 in case of directories) */
} webui_dentry; 

//! Header for files and directories hat are packed into Web UI data blob
typedef struct webui_entry_t {
  int32_t name_size;                /*!< Size of file name string */ 
  char    name[MAX_FILE_NAME_LEN];  /*!< Name of the file/directory */ 
  char    type;                     /*!< Type of the entry (set 01 in case of files) */
  int32_t size;                     /*!< Size of the file */
  char    data[MAX_FILE_SIZE];      /*!< File data */
} webui_entry; 

// Function declarations
int32_t calc_checksum_file(FILE*);
int32_t calc_checksum_blob(const webui_data_blob*, const size_t); 

#endif				/* // __CAMTOOL_H_ */
