#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sanic_fs.h"
#include "disk.h"

directory_entry directory[MAX_FILES];
file_descriptor descriptor_table[MAX_DESCRIPTORS];
int files, descriptors;

/* Helper function prototypes */
int search_directory(char* fname);
short get_block_ptr(int block);
int set_block_ptr(int block, short ptr);

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

  /* Get file from directory */
  int di = search_directory(name);
  if (di == -1) {
    fprintf(stderr, "fs_open: Couldn't find file %s.\n", name);
    return -1;
  }

  /* Insert new file descriptor into first open slot in table */
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    if (descriptor_table[i].start == 0) {
      descriptor_table[i].start = directory[di].start;
      descriptor_table[i].offset = 0;
      return i;
    }
  }

  /* Table is full */
  fprintf(stderr, "fs_open: Too many open file descriptors.\n");
  return -1;
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
  /* Check name length < 15 characters */
  int len = strlen(name);
  if (len >= MAX_FNAME) {
    fprintf(stderr, "fs_create: File name too long (> 15 characters).\n");
    return -1;
  }
  
  /* Get first free entry in directory, and check that name is unique */
  int di = -1;
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    if (directory[i].start == 0) {
      /* Entry is free */
      if (di == -1) {
        di = i;
      }
    } else {
      /* Entry is used */
      if (!strcmp(name, directory[i].filename)) {
        /* Entry name matches new file name! */
        fprintf(stderr, "fs_create: File %s already exists on disk.\n", name);
        return -1;
      }
    }
  }

  /* If no entries are free, disk is at capacity */
  if (di == -1) {
    fprintf(stderr, "fs_create: Disk is at file capacity (64 files).\n");
    return -1;
  }

  /* Allocate first free block */
  int block_i = -1;
  for (i = 1; i < DISK_BLOCKS; i++) {
    if (get_block_ptr(i) == BLOCK_FREE) {
      block_i = i;
      break;
    }
  }

  /* If no blocks are free, disk is at capacity */
  if (block_i == -1) {
    fprintf(stderr, "fs_create: Disk is at block capacity.\n");
    return -1;
  }
  
  /* Mark block as allocated */
  if(set_block_ptr(block_i, BLOCK_TERMINATOR)) {
    fprintf(stderr, "fs_create: Block allocation failed.\n");
    return -1;
  }
    
  /* Set directory name to new name, and pad with 0s */
  for (i = 0; i < MAX_FNAME; i++) {
    directory[di].filename[i] = (i < len ? name[i] : 0);
  }

  directory[di].start = block_i;

  files++;
  
  return 0;
}

int fs_delete(char* name){
  /* Find file in directory */
  int di = search_directory(name);
  if (di == -1) {
    fprintf(stderr, "fs_delete: File %s does not exist on disk.\n", name);
    return -1;
  }

  /* Make sure no descriptors to this file exist */
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    if (descriptor_table[i].start == directory[di].start) {
      fprintf(stderr, "fs_delete: There are open descriptors to file %s.\n",
              name);
      return -1;
    }
  }

  /* Mark all blocks in list as free */
  int block = directory[di].start;
  do {
    int next = get_block_ptr(block);

    if(set_block_ptr(block, BLOCK_FREE)) {
      fprintf(stderr, "fs_delete: Could not de-allocate blocks on disk.\n");
    }

    block = next;
  } while (block >= 0);
  
  /* Mark directory entry as free */
  directory[di].start = 0;
  
  return 0;
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

void print_directory() {
  printf("Directory table:\n\t[filename]\tstart\n");
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    printf("\t[%s]\t%d\n", directory[i].filename, directory[i].start);
  }
}

void print_descriptors() {
  printf("File Descriptor table:\n\tstart\toffset\n");
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    printf("\t%d\t%d\n", descriptor_table[i].start, descriptor_table[i].offset);
  }
}

/**
 * Search through the directory table for the first file with a given name.
 *
 * @param fname  Name of file to search for.
 * @return       Either the index of the file in the directory table,
 *               or -1 on failure.
 */
int search_directory(char* fname) {
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    if (directory[i].start != 0
        && !strcmp(fname, directory[i].filename)) {
      return i;
    }
  }

  return -1;
}

/**
 * Gets the index of the next block in the chain, stored as the first two bytes
 * of the block on the disk.
 *
 * @param block  Disk index of the block in question.
 * @return       Disk index of the next block.
 */
short get_block_ptr(int block) {
  char buffer[BLOCK_SIZE];

  if (block_read(block, buffer)) {
    fprintf(stderr, "get_block_ptr: Error reading block %d.\n", block);
    return -1;
  }

  short* as_short = (short*) buffer;
  return as_short[0];
}

/**
 * Sets the index of the next block in the chain, stored as the first two bytes
 * of the block on the disk. This operation is performed without altering the
 * content of the block in question.
 *
 * @param block  Disk index of the block in question.
 * @param ptr    Disk index of the next block.
 * @return       0 on success, -1 on failure.
 */
int set_block_ptr(int block, short ptr) {
  char buffer[BLOCK_SIZE];

  if (block_read(block, buffer)) {
    fprintf(stderr, "set_block_ptr: Error reading block %d.\n", block);
    return -1;
  }

  memcpy(buffer, &ptr, 2);

  if (block_write(block, buffer)) {
    fprintf(stderr, "set_block_ptr: Error writing block %d.\n", block);
    return -1;
  }

  return 0;
}
