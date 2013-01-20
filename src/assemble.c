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

#define TODO_HARDCODED_DIR_NAME "testdir"

int main() {
  list_target_dir (TODO_HARDCODED_DIR_NAME);
  return 0;
}

/* Internal functions */

/// \brief Appends a file name to the path  
char* append_file_name (char* path, const char* fname) {
  const size_t path_len = strlen (path);
  const size_t fname_len = strlen (fname);

  assert ((path_len + fname_len) < MAX_PATH_LEN);

  // Make sure we do not prepend an empty path with /
  if (path[0] != '\0')
    strcat (path, "/");  

  strcat (path, fname);
  return path;
} 

/// \brief Given a directory name and the file name creates a path to a file   
char* create_path (char* path, const char*dirname, const char* fname) {
  const size_t dirname_len = strlen (dirname);
  const size_t fname_len = strlen (fname); 
  
  assert ((dirname_len + fname_len) < MAX_PATH_LEN);

  path[0] = '\0';
  append_file_name(path, dirname);
  append_file_name(path, fname);
  return path; 
}

int list_target_dir (const char* dir_name) { 
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
  while ( ( dir_entry = readdir (dir)) != NULL  ) {
    full_path[0] = '\0';
    struct stat s;

    // Skip parrent and current directories 
    if (strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0)
      continue;

    // Read the entry 
    //if (fstatat (dir_entry, dir_entry->d_name, &s)  != 0) {
    append_file_name (full_path, dir_name);
    append_file_name (full_path, dir_entry->d_name);
    
    if (stat ( full_path, &s)) {
      // TODO: verbose error message 
      // Proper error handling 
      printf ("Error doing fstatat on %s\n", full_path);
      perror ("fstatat failed");
      return -1;
    }

    if (S_ISDIR(s.st_mode)) {
      printf ("Found a directory: %s/%s\n", dir_name, dir_entry->d_name);
      list_target_dir (create_path (full_path, dir_name, dir_entry->d_name));
    } 
    else if (S_ISREG(s.st_mode)) {
      printf ("Found a regular file: %s/%s\n", dir_name, dir_entry->d_name);
    }  
  }
} 

