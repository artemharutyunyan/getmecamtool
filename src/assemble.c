/*
 
 Header goes here 

*/

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "assemble.h"
#include "camtool.h"

#include "common.h"

#define TODO_HARDCODED_DIR_NAME "."

#define WEBUI_DATABLOB_INITIAL_SIZE (1024 * 1024) // 1 MB

int main() {
  traverse_target_dir (TODO_HARDCODED_DIR_NAME);
  return 0;
}

/* Internal functions */

/// \brief Appends a file name to the path  
char* append_file_name (char* path, const char* fname) {
  const size_t path_len = strlen (path),
               fname_len = strlen (fname);

  assert ((path_len + fname_len) < MAX_PATH_LEN);

  // Make sure we do not prepend an empty path with /
  if (path[0] != '\0')
    strcat (path, "/");  

  strcat (path, fname);
  return path;
} 

/// \brief Given a directory name and the file name creates a path to a file   
char* create_path (char* path, const char*dirname, const char* fname) {
  const size_t dirname_len = strlen (dirname),
               fname_len = strlen (fname); 
  
  assert ((dirname_len + fname_len) < MAX_PATH_LEN);

  path[0] = '\0';
  append_file_name(path, dirname);
  append_file_name(path, fname);
  return path; 
}

int traverse_target_dir (const char* dir_name) { 
  struct dirent* dir_entry;
  int err_code = 0;
  char full_path[MAX_PATH_LEN] = {'\0'};

  // Open the directory 
  DIR* dir = opendir (dir_name);

  if ( dir == NULL ) {
    // TODO: verbose error message 
    // Proper error handling
    perror ("opendir failed ");
    return -1;
  }
  
  // Iterate over entries 
  
  // First read all the files (skip subdirectories)    
  struct stat s;
  while ( (dir_entry = readdir (dir)) != NULL  ) {
    full_path[0] = '\0';

    // Skip parrent and current directories 
    if (strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0)
      continue;

    // Read the entry 
    append_file_name (full_path, dir_name);
    append_file_name (full_path, dir_entry->d_name);
    
    if (stat  (full_path, &s)) {
      // TODO: verbose error message 
      // Proper error handling 
      printf ("Error doing fstatat on %s\n", full_path);
      perror ("fstatat failed");
      return -1;
    }

    if (S_ISDIR(s.st_mode)) {
      continue;
    } 
    else if (S_ISREG(s.st_mode)) {
      printf ("Found a regular file: %s/%s\n", dir_name, dir_entry->d_name);
    }  
  }

  // Then iterate over subdirectories 

  // Reset the directory handle
  rewinddir(dir);

  while ( (dir_entry = readdir(dir)) != NULL ) {
    full_path[0] = '\0';

    // Skip parrent and current directories 
    if (strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0)
      continue;

    // Read the entry 
    append_file_name (full_path, dir_name);
    append_file_name (full_path, dir_entry->d_name);
    
    if (stat (full_path, &s)) {
      // TODO: verbose error message 
      // Proper error handling 
      printf ("Error doing fstatat on %s\n", full_path);
      perror ("fstatat failed");
      return -1;
    }

    if (S_ISDIR(s.st_mode)) {
      printf ("Found a directory: %s/%s\n", dir_name, dir_entry->d_name);
      traverse_target_dir (create_path (full_path, dir_name, dir_entry->d_name));
    } 
  }
  return 0;
} 

/// \brief Initialize Web UI blob data structure 
int webui_data_blob_init (webui_data_blob* blob) {
  blob->data = malloc (WEBUI_DATABLOB_INITIAL_SIZE);

  if (blob->data == NULL) {
    // TODO Verbose error message 
    perror ("malloc failed");
    exit(-1);
  }

  blob->alloc_size = WEBUI_DATABLOB_INITIAL_SIZE;
  blob->size = 0;

  return 0;
}

/// \brief Cleans up Web UI data blob structure
int webui_data_blob_clean (webui_data_blob* blob) {
  free(blob->data);
  return 0;
}

/// \brief Calculates the size of the fentry 
int get_fentry_size (const webui_fentry* fentry) {
  return  (WEBUI_ENTRY_NAME_SIZE_FIELD_LEN + 
          fentry->name_size + 
          WEBUI_ENTRY_TYPE_FIELD_LEN + 
          WEBUI_FENTRY_SIZE_FIELD_LEN + 
          fentry->size);
}

int webui_append_fentry (const webui_fentry* fentry, webui_data_blob *blob) {
  
  // Make sure there is enough memory allocaed 
  if (blob->alloc_size < (blob->size + get_fentry_size(fentry)) ) {
    //TODO Realloc
    perror("Realloc");
    exit (-1);
  }  

  // Copy file entry fields into the blob 
  size_t offset = blob->size; 
  
  // Size of the name field
  memmove ( &blob->data[offset], &fentry->name_size, WEBUI_ENTRY_NAME_SIZE_FIELD_LEN);
  offset += WEBUI_ENTRY_NAME_SIZE_FIELD_LEN;

  // Name field 
  memmove (&blob->data[offset], fentry->name, fentry->name_size);
  offset += fentry->name_size;

  // Type field 
  memmove (&blob->data[offset], &fentry->type, WEBUI_ENTRY_TYPE_FIELD_LEN);
  offset += WEBUI_ENTRY_TYPE_FIELD_LEN;

  // Size of the file 
  memmove (&blob->data[offset], &fentry->size, WEBUI_FENTRY_SIZE_FIELD_LEN);
  offset += WEBUI_FENTRY_SIZE_FIELD_LEN;
  
  // File contents  
  memmove (&blob->data[offset], fentry->data, fentry->size);
  offset += fentry->size; 

  blob->size = offset;

  return 0;
} 

int webui_append_dentry (const webui_dentry* dentry, FILE* fd) {

  return 0;
}
