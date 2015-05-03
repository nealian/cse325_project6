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
  if(make_disk(disk_name)) {
    fprintf(stderr, "make_fs: Could not create disk.\n");
    return -1;
  }

  return 0;
}

int mount_fs(char* disk_name){
  if(open_disk(disk_name)) {
    fprintf(stderr, "mount_fs: Could not open disk.\n");
    return -1;
  }

  /* Read entire super block into buffer */
  char* buffer = malloc(BLOCK_SIZE);
  if(block_read(SUPER_BLOCK, buffer)) {
    fprintf(stderr, "mount_fs: Failed to read super block from disk.\n");
    return -1;
  }
  /* Extract directory table into memory */
  memcpy(&directory, buffer, sizeof(directory));
  free(buffer);

  /* Count files */
  files = 0;
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    if (directory[i].start != 0) {
      files++;
    }
  }

  /* Initialize descriptor table */
  memset(descriptor_table, 0, sizeof(descriptor_table));
  descriptors = 0;

  return 0;
}

int umount_fs(char* disk_name){
  if (descriptors > 0) {
    fprintf(stderr, "umount_fs: There are still open file descriptors.\n");
    return -1;
  }

  /* Copy directory table into super block buffer */
  char* buffer = malloc(BLOCK_SIZE);
  memset(buffer, 0, BLOCK_SIZE);
  memcpy(buffer, &directory, sizeof(directory));

  /* Write super block to disk */
  block_write(SUPER_BLOCK, buffer);
  free(buffer);

  if(close_disk()) {
    fprintf(stderr, "umount_fs: Could not close disk.\n");
    return -1;
  }

  return 0;
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
