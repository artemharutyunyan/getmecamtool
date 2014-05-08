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
#define MAX_SETTING_LINE_LEN 1024
#define SETTING_FILE_DELIM ':'
#define SETTING_FILE_DELIM_COUNT 4

#define MAX_FILE_SIZE 2*1048576

#define MAX_HOSTNAME_LEN 256

// !Web UI data blob
typedef struct webui_data_blob_t {
	size_t        size;
	size_t        alloc_size;
	unsigned char *data;
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

//! Configuration file header
typedef struct conf_file_header_t {
	int32_t         magic;	  /* !< Magic number */
	int32_t         checksum;	/* !< checksum */
	int32_t         reserve;	/* !< something 2 */
  char            camid[16];/* !< camera id> */
	int32_t         sysver;	  /* !< System firmware version */
	int32_t         webuiver;	/* !< Web UI version */
  char            alias[21]; /* !<Alias> */
} conf_file_header;

#pragma pack(push, 1)
typedef struct conf_user_t {
  char username[13];
  char password[13];
  char role;
} conf_user;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct conf_network_t {
  int32_t ipaddr;
  int32_t mask;
  int32_t gateway;
  int32_t dns;
  int32_t unknown;
  unsigned short port;
} conf_network;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct conf_adsl_t {
  char attr;
  char username[65];
  char password[65];
} conf_adsl;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct conf_wifi_t {
  char enable;
  char ssid[41];
  char something[129];
  char wpa_psk[65];
  char country;
} conf_wifi;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct conf_email_t {
  char mail_inet_ip;
  char sender[65];
  char receiver1[65];
  char receiver2[65];
  char receiver3[65];
  char receiver4[65];
  char mail_server[65];
  uint16_t port;
  char username[65];
  char password[65];
} conf_email;
#pragma pack(pop)

typedef struct conf_file_t {
  conf_file_header header;
  conf_user users[8];
  conf_network network;
  conf_adsl adsl;
  conf_wifi wifi;
  conf_email email;
} conf_file;

#endif				/* // __CAMTOOL_H_ */

