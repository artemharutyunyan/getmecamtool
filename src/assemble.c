/*
 
 Header goes here 

*/

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "assemble.h"

#define HARDCODED_DIR_NAME "testdir"

int main() {
  list_target_dir (HARDCODED_DIR_NAME);
  return 0;
}

char *concatenate (const char* path, const char* fname) {
  const size_t path_len = strlen(path);
  const size_t fname_len = strlen(fname);

  char *r = (char*) malloc ((path_len + fname_len) * sizeof(*r) + 1);
  if (r == NULL) {
    // TODO: verbose error message 
    // Proper error handling 
    perror ("malloc failed");
    exit (-1);
  }
  
  r[0] = '\0';

  strcat (r, path);  
  strcat (r, "/");  
  strcat (r, fname);
  return r;
} 

int list_target_dir (const char* dir_name) { 
  struct dirent* dir_entry;
  int err_code = 0;

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
    struct stat s;

    // Skip parrent and current directories 
    if (strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0)
      continue;

    // Read the entr 
    //if (fstatat (dir_entry, dir_entry->d_name, &s)  != 0) {
    char *full_path = concatenate (dir_name, dir_entry->d_name); 
    if (stat ( full_path, &s)) {
      // TODO: verbose error message 
      // Proper error handling 
      perror ("fstatat failed");
      free(full_path);
      return -1;
    }

    if (S_ISDIR(s.st_mode)) {
      printf ("Found a directory: %s\n", dir_entry->d_name);
    } 
    else if (S_ISREG(s.st_mode)) {
      printf ("Found a regular file: %s\n", dir_entry->d_name);
    }  

    free(full_path);
  }
} 

