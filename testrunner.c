#include <stdio.h>
#include <stdlib.h>
#include "sanic_fs.h"
#include "disk.h"

#define DISK_NAME "test.fs"

/**
 * Attempt to clean test disk. Failed tests might leave the disk open, so try to
 * close it and wipe it. A return status of -1 indicates a fatal error.
 */
int env_cleanup() {
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "env_cleanup: Couldn't unmount disk.\n");
    return -1;
  }

  if (make_fs(DISK_NAME)) {
    fprintf(stderr, "env_cleanup: Couldn't wipe disk.\n");
    return -1;
  }

  return 0;
}

/**
 * Write (nbytes) of arbitrary ASCII data to (file).
 */
int write_test_pattern(int file, size_t nbytes) {
  char* buffer = malloc(nbytes);

  int i;
  for(i = 0; i < nbytes; i++) {
    buffer[i] = 'a' + (i % ('z' - 'a'));
  }

  if (fs_write(file, buffer, nbytes) == -1)  {
    fprintf(stderr, "write_test_pattern: Write failed.\n");
    free(buffer);
    return -1;
  }

  free(buffer);
  return 0;
}

/**
 * Read (nbytes) of data from the file and check that it matches the
 * test pattern.
 */
int check_test_pattern(int file, size_t nbytes) {
  char* buffer = malloc(nbytes);

  if (fs_write(file, buffer, nbytes) == -1)  {
    fprintf(stderr, "check_test_pattern: Read failed.\n");
    free(buffer);
    return -1;
  }
  
  int i;
  for(i = 0; i < nbytes; i++) {
    if(buffer[i] != 'a' + (i % ('z' - 'a'))) {
      fprintf(stderr, "check_test_pattern: Pattern mismatch at byte %d.\n", i);
      free(buffer);
      return -1;
    }
  }

  free(buffer);
  return 0;
}

/**
 * Make new filesystem, mount it, and unmount it.
 */
int test_fs_creation() {

  /* Make new filesystem */
  if (make_fs(DISK_NAME)) {
    fprintf(stderr, "test_fs_creation: FS creation failed.\n");
    return -1;
  }

  /* Mount it */
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_fs_creation: Mount failed.\n");
    return -1;
  }

  /* Unmount it */
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_fs_creation: Unmount failed.\n");
    return -1;
  }

  return 0;
}

/**
 * Mount the filesystem, create a file, unmount. Re-mount, delete file, unmount.
 */
int test_file_creation() {

  char* fname = "test_file";

  /* Mount filesystem */
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Mount failed.\n");
    return -1;
  }

  /* Create a file */
  if (fs_create(fname)) {
    fprintf(stderr, "test_file_creation: File creation failed.\n");
    return -1;
  }

  /* Unmount filesystem */
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Unmount failed.\n");
    return -1;
  }

  /* Mount filesystem */
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Mount failed.\n");
    return -1;
  }

  /* Delete file from earlier */
  if (fs_delete(fname)) {
    fprintf(stderr, "test_file_creation: File deletion failed.\n");
    return -1;
  }

  /* Unmount filesystem */
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Unmount failed.\n");
    return -1;
  }

  return 0;
}

/**
 * Mount the filesystem, create several files, open one for writing, write a few
 * blocks of the test pattern to one of them, unmount the filesystem, re-mount
 * the filesystem, read back test pattern, check that it matches, delete the
 * files, and unmount.
 */
int test_file_write() {
  char* fname1 = "test_file_1";
  char* fname2 = "test_file_2";
  char* fname3 = "test_file_3";

  int fd = 0;
  int fd_unused = 0;

  size_t nbytes = BLOCK_SIZE * 3 + 42;

  /* Mount filesystem */
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_write: Mount failed.\n");
    return -1;
  }

  /* Create multiple arbitrary files */
  if (fs_create(fname1)
      || fs_create(fname2)
      || fs_create(fname3)) {
    fprintf(stderr, "test_file_write: File creation failed.\n");
    return -1;
  }

  /* Get multiple file descriptors */
  if ((fd_unused = fs_open(fname1)) == -1
      || (fd = fs_open(fname2)) == -1) {
    fprintf(stderr, "test_file_write: Failed to open files.\n");
    return -1;
  }

  /* Write test pattern to one of the files */
  if (write_test_pattern(fd, nbytes)) {
    fprintf(stderr, "test_file_write: Failed to write test pattern.\n");
    return -1;
  }

  /* Check that the file size is as expected */
  int apparent_size = fs_get_filesize(fd);
  if(apparent_size != nbytes) {
    fprintf(stderr, "test_file_write: Expected size %lu, got %d.\n",
            nbytes, apparent_size);
    return -1;
  }

  apparent_size = fs_get_filesize(fd_unused);
  if(apparent_size != 0) {
    fprintf(stderr, "test_file_write: Expected size 0, got %d.\n",
            apparent_size);
    return -1;
  }
  
  /* Delete one of the unused files */
  if (fs_delete(fname3)) {
    fprintf(stderr, "test_file_write: Failed to delete unused file.\n");
    return -1;
  }

  /* Close open files */
  if (fs_close(fd)
      || fs_close(fd_unused)) {
    fprintf(stderr, "test_file_write: Failed to close open files.\n");
    return -1;    
  }

  /* Unmount the filesystem */
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_write: Unmount failed.\n");
    return -1;
  }

  /* Re-mount the filesystem */
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_write: Mount failed.\n");
    return -1;
  }

  /* Get descriptor of file written earlier */
  if ((fd = fs_open(fname2)) == -1) {
    fprintf(stderr, "test_file_write: Failed to re-open file.\n");
    return -1;
  }

  /* Check that the file size is as expected */
  apparent_size = fs_get_filesize(fd);
  if(apparent_size != nbytes) {
    fprintf(stderr, "test_file_write: Expected size %lu, got %d.\n",
            nbytes, apparent_size);
    return -1;
  }

  /* Check that file data is unchanged */
  if (check_test_pattern(fd, nbytes)) {
    fprintf(stderr, "test_file_write: Test pattern check failed.\n");
    return -1;
  }
  
  /* Close open file */
  if (fs_close(fd)) {
    fprintf(stderr, "test_file_write: Failed to close open file.\n");
    return -1;
  }

  /* Delete files from earlier */
  if (fs_delete(fname1)
      || fs_delete(fname2)) {
    fprintf(stderr, "test_file_write: Failed to delete files.\n");
    return -1;
  }

  /* Unmount filesystem */
  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_write: Unmount failed.\n");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {

  if (test_fs_creation()) {
    printf("test_fs_creation failed.\n");
    
    if (env_cleanup()) {
      return 1;
    }
  } else {
    printf("test_fs_creation successful.\n");
  }

  if (test_file_creation()) {
    printf("test_file_creation failed.\n");
    
    if (env_cleanup()) {
      return 1;
    }
  } else {
    printf("test_file_creation successful.\n");
  }

  if (test_file_write()) {
    printf("test_file_write failed.\n");
    
    if (env_cleanup()) {
      return 1;
    }
  } else {
    printf("test_file_write successful.\n");
  }


  return 0;
}
