/*

  TODO: Add a meaningful header here 
  
*/

#ifndef __CAMTOOL_H_
#define __CAMTOOL_H

#include <stdint.h>

//! Web UI data blob 
typedef struct webui_data_blob_t {
  size_t size;
  size_t alloc_size;
  char*  data;   
} webui_data_blob;

///! Web UI file header 
#pragma pack(push, 4) 
typedef struct webui_file_header_t {
  int32_t magic;    /*!< Magic number */ 
  int32_t checksum; /*!< Checksum (sum of all bytes starting from offset 12) */ 
  int32_t size;     /*!< Size of file */ 
  int32_t version;  /*!< Version */ 
} webui_file_header;  
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
  char    entry;      /*!< Type of the entry (set 00 in case of directories) */
} webui_dentry; 

#endif  // ___ASSEMBLE_H
