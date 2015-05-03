#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sanic_fs.h"
#include "disk.h"

directory_entry directory[MAX_FILES];
file_descriptor descriptor_table[MAX_DESCRIPTORS];
int files, descriptors;

int make_fs(char* disk_name){
  // TODO
  return -1;
}

int mount_fs(char* disk_name){
  // TODO
  return -1;
}

int umount_fs(char* disk_name){
  // TODO
  return -1;
}

int fs_open(char* name){

  /* Make sure we can open another file descriptor */
  if (descriptors >= MAX_DESCRIPTORS) {
    fprintf(stderr, "fs_open: Too many file descriptors open.\n");
    return -1;
  } else {
    /* Search for filename in directory table */
    // TODO: we could do this better
    int i;
    for (i = 0; i < MAX_FILES; i++) {
      if (directory[i].start != 0
          && strcmp(directory[i].filename, name) == 0) {
        
        /* Make a new file descriptor and insert into table */
        int j;
        for (j = 0; j < MAX_DESCRIPTORS; j++) {
          if (descriptor_table[j].start == 0) {
            descriptor_table[j].start = directory[i].start;
            descriptor_table[j].offset = 0;
            return j;
          }
        }
      }
    }

    fprintf(stderr, "fs_open: Could not find file %s\n", name);
    return -1;
  }
}

int fs_close(int fildes){
  if (fildes < 0 || fildes >= MAX_DESCRIPTORS
      || descriptor_table[fildes].start == 0) {
    fprintf(stderr, "fs_close: Invalid file descriptor.\n");
    return -1;
  } else {
    descriptor_table[fildes].start = 0;
    descriptor_table[fildes].offset = 0;
    return 0;
  }
}

int fs_create(char* name){
  // TODO
  return -1;
}

int fs_delete(char* name){
  // TODO
  return -1;
}

int fs_read(int fildes, void* buf, size_t nbyte){
  // TODO
  return -1;
}

int fs_write(int fildes, void* buf, size_t nbyte){
  // TODO
  return -1;
}

int fs_get_filesize(int fildes){
  // TODO
  return -1;
}

int fs_lseek(int fildes, off_t offset){
  // TODO
  return -1;
}

int fs_truncate(int fildes, off_t length){
  // TODO
  return -1;
}
