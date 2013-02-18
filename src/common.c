/*
 
 Header goes here 

*/

#include <stdio.h>

#include "camtool.h"
#include "common.h"

/*! Takes the open file descriptor pointing to a Web UI file 
    as an input and calculates the checksum  

    \param f open file descriptor 
    \return The checksum (sum of all the bytes that follow file header)
*/ 
int32_t calc_checksum_file (FILE* f)
{
  fseek(f, OFFSET_VERSION, SEEK_SET);
  int32_t sum = 0;
  unsigned char buf;
  while(1){
    fread(&buf, 1, 1, f);
    if(feof(f))
      break;
    sum += buf;
  }
  return sum;
}

/*! Takes a webui_data_blob populated with the Web UI data as an input
    and calculates the checksum 

    \param blob the blob containing Web UI data 
    \return The checksum (sum of all the bytes that follow file header)
*/ 
int32_t calc_checksum_blob (const webui_data_blob* blob, const size_t offset)
{
  int32_t sum = 0;
  size_t i = offset;

  while (i < blob->size) {
    sum += blob->data[i];
    ++i;
  }

  return sum;
}


