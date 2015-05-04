#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sanic_fs.h"
#include "disk.h"

directory_entry directory[MAX_FILES];
file_descriptor descriptor_table[MAX_DESCRIPTORS];
int descriptors;

/* Helper function prototypes */
int search_directory(char* fname);
int get_block_ptr(int block);
int set_block_ptr(int block, short ptr);
int fs_read_block(int block, char *buf);
int fs_write_block(int block, char *buf);
int fs_allocate_block(void);
int block_at_current_seek(int fildes);
void free_list(int head);

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

  /* Initialize descriptor table */
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    descriptor_table[i].directory_i = -1;
  }
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
    if (descriptor_table[i].directory_i == -1) {
      descriptor_table[i].directory_i = di;
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
      || descriptor_table[fildes].directory_i == -1) {
    fprintf(stderr, "fs_close: Invalid file descriptor.\n");
    return -1;
  } else {
    descriptor_table[fildes].directory_i = -1;
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

  /* Get the next available block */
  int start_block = fs_allocate_block();
  if (start_block == -1) {
    fprintf(stderr, "fs_create: Block allocation failed.\n");
    return -1;
  }

  /* Set directory name to new name, and pad with 0s */
  for (i = 0; i < MAX_FNAME; i++) {
    directory[di].filename[i] = (i < len ? name[i] : 0);
  }

  directory[di].start = start_block;
  directory[di].size = 0;

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
    if (descriptor_table[i].directory_i == di) {
      fprintf(stderr, "fs_delete: There are open descriptors to file %s.\n",
              name);
      return -1;
    }
  }

  /* Mark all blocks in list as free */
  free_list(directory[di].start);
  
  /* Mark directory entry as free */
  directory[di].start = 0;
  
  return 0;
}

int fs_read(int fildes, void* buf, size_t nbyte){
  int next_block;

  
  return 0;
}

int fs_write(int fildes, void* buf, size_t nbyte){

  if (fildes < 0 || fildes >= MAX_DESCRIPTORS
      || descriptor_table[fildes].directory_i == -1) {
    fprintf(stderr, "fs_write: Invalid file descriptor.\n");
    return -1;
  }

  char* buf_as_char = (char*) buf;
  
  int block_i = block_at_current_seek(fildes);
  char block_buffer[BLOCK_SIZE - 2];
  int sub_i = descriptor_table[fildes].offset % (BLOCK_SIZE - 2);

  int i = 0;
  while (i < nbyte) {
    /* Read current block into buffer*/
    if (fs_read_block(block_i, block_buffer) == -1) {
      fprintf(stderr, "fs_write: Couldn't read block from disk.\n");
      return -1;
    }

    /* Write input buffer to block buffer */
    for (; sub_i < BLOCK_SIZE - 2 && i < nbyte; sub_i++, i++, descriptor_table[fildes].offset++) {
      block_buffer[sub_i] = buf_as_char[i];
    }

    /* Write block buffer to disk */
    if ((block_i = fs_write_block(block_i, block_buffer)) == -1) {
      fprintf(stderr, "fs_write: Couldn't write block to disk.\n");
      return -1;
    }

    /* Extend file if needed */
    if (i < nbyte
        && block_i == BLOCK_TERMINATOR) {
      if ((block_i = fs_allocate_block()) == -1) {
        fprintf(stderr, "fs_write: Out of disk space.\n");
        break;
      }
      
      sub_i = 0;
    }
    
  }
  
  if (descriptor_table[fildes].offset >
      directory[descriptor_table[fildes].directory_i].size) {
    directory[descriptor_table[fildes].directory_i].size = descriptor_table[fildes].offset;
  }

  return i;
}

int fs_get_filesize(int fildes){
  // TODO
  return -1;
}

int fs_lseek(int fildes, off_t offset){
  
  int fsize = fs_get_filesize(fildes);

  if (fsize == -1) {
    fprintf(stderr, "fs_lseek: Cannot determine size of file.\n");
    return -1;
  }
  
  if (offset < 0 || offset > fsize) {
    fprintf(stderr, "fs_lseek: Seek offset out of bounds.\n");
    return -1;
  }

  descriptor_table[fildes].offset = offset;
  return 0;
}

int fs_truncate(int fildes, off_t length){

  int fsize = fs_get_filesize(fildes);
  if (fsize == -1) {
    fprintf(stderr, "fs_truncate: Cannot determine size of file.\n");
    return -1;
  }

  if (length > fsize) {
    fprintf(stderr,
            "fs_truncate: Cannot truncate to length greater than file size.\n");
    return -1;
  } else if (length < fsize) {
    int new_blocksize = length / (BLOCK_SIZE - 2);

    int block_i = directory[descriptor_table[fildes].directory_i].start;
    int i;
    for (i = 0; i < new_blocksize; i++) {
      block_i = get_block_ptr(block_i);
    }

    int tail = get_block_ptr(block_i);

    if (set_block_ptr(tail, BLOCK_TERMINATOR)) {
      fprintf(stderr, "fs_truncate: Couldn't set new file end block.\n");
      return -1;
    }

    free_list(tail);
  }

  return 0;
}

void print_directory() {
  printf("Directory table:\n\t[filename]\tstart\tsize\n");
  int i;
  for (i = 0; i < MAX_FILES; i++) {
    printf("\t[%s]\t%d\t%d\n", directory[i].filename, directory[i].start,
           directory[i].size);
  }
}

void print_descriptors() {
  printf("File Descriptor table:\n\tdir_link\toffset\n");
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    printf("\t%d\t%d\n", descriptor_table[i].directory_i,
           descriptor_table[i].offset);
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
int get_block_ptr(int block) {
  char buffer[BLOCK_SIZE];

  if (block_read(block, buffer)) {
    fprintf(stderr, "get_block_ptr: Error reading block %d.\n", block);
    return -1;
  }

  short* as_short = (short*) buffer;
  return (int) as_short[0];
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

/**
 * Read the whole (BLOCK_SIZE - 2) bytes into buf from block.
 *
 * @param block  Block to read from
 * @param buf    Buffer to write into
 * @return       Either the next block index, -1 on failure, or -2 on
 *               end-of-file.
 */
int fs_read_block(int block, char *buf) {

  char tmp_buf[BLOCK_SIZE];
  int next_block;

  if (block_read(block, tmp_buf)) {
    fprintf(stderr, "fs_read_block: Error reading the block %d.\n", block);
    return -1;
  }

  next_block = ((short *)tmp_buf)[0]; // The first two bytes are the next block
  memcpy(buf, tmp_buf + 2, BLOCK_SIZE - 2); // Copy (BLOCK_SIZE - 2) bytes into buf, starting at tmp_buf+2

  return next_block;
}

/**
 * Write the whole (BLOCK_SIZE - 2) bytes from buf into block.
 *
 * @param block  Block to write into
 * @param buf    Buffer to read from
 * @return       Either the next block index, -1 on failure, or -2 on (current)
 *               end-of-file.
 */
int fs_write_block(int block, char *buf) {
  char tmp_buf[BLOCK_SIZE];
  short next_block;

  if(block_read(block, tmp_buf)) {
    fprintf(stderr, "fs_write_block: Error reading the block %d.\n", block);
    return -1;
  }
  next_block = ((short *)tmp_buf)[0]; // The first two bytes are the next block
  memcpy(tmp_buf + 2, buf, BLOCK_SIZE - 2); // Copy (BLOCK_SIZE - 2) bytes into tmp_buf, starting at tmp_buf+2

  if(block_write(block, tmp_buf)) {
    fprintf(stderr, "fs_write_block: Error writing the block %d.\n", block);
    return -1;
  }

  return (int) next_block;
}

/**
 * Allocate the first free block, and empty its stale contents.
 *
 * @return  The first free block, or -1 on failure
 */
int fs_allocate_block(void) {
  int block_i = -1, i;
  char zeros[BLOCK_SIZE-2] = {0};

  /* Allocate first free block */
  for (i = 1; i < DISK_BLOCKS; i++) {
    if (get_block_ptr(i) == BLOCK_FREE) {
      block_i = i;
      break;
    }
  }

  /* If no blocks are free, disk is at capacity */
  if (block_i == -1) {
    fprintf(stderr, "fs_allocate_block: Disk is at block capacity.\n");
    return -1;
  }

  /* Mark block as allocated */
  if(set_block_ptr(block_i, BLOCK_TERMINATOR)) {
    fprintf(stderr, "fs_allocate_block: Block allocation on block %d failed.\n", block_i);
    return -1;
  }

  /* Clear out the block */
  if(fs_write_block(block_i, zeros) == -1) {
    fprintf(stderr, "fs_allocate_block: Block zeroing on block %d failed.\n", block_i);
    return -1;
  }

  return block_i;
}

/**
 * Return the block that the file descriptor is currently seeked to
 *
 * @param filedes  The file descriptor
 * @return         The block that the file is seeked to, or -1 on failure, or -2
 *                 if the file is overseeked
 */
int block_at_current_seek(int fildes) {
  int next_block = directory[descriptor_table[fildes].directory_i].start;
  int i = descriptor_table[fildes].offset;

  for(; i > BLOCK_SIZE - 2; i -= BLOCK_SIZE - 2) {
    if(next_block == BLOCK_TERMINATOR) {
      fprintf(stderr, "block_at_current_seek: Overseeked the file (fd %d)\n",
              fildes);
      return BLOCK_TERMINATOR;
    } else if((next_block = get_block_ptr(next_block)) == -1) {
      fprintf(stderr, "block_at_current_seek: Failed to get next "
              "block pointer (fd %d)\n", fildes);
      return -1;
    }
  }

  return next_block;
}

/**
 * Frees every block in the list recursively, starting with (head).
 *
 * @param head  Index of block to start freeing from.
 */
void free_list(int head) {
  if(head == BLOCK_TERMINATOR || head == BLOCK_FREE) {
    return;
  }
  
  int tail = get_block_ptr(head);  
  set_block_ptr(head, BLOCK_FREE);
  free_list(tail);
}
