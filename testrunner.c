#include <stdio.h>
#include "sanic_fs.h"

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
 * Make new filesystem, mount it, and unmount it.
 */
int test_fs_creation() {

  if (make_fs(DISK_NAME)) {
    fprintf(stderr, "test_fs_creation: FS creation failed.\n");
    return -1;
  }

  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_fs_creation: Mount failed.\n");
    return -1;
  }

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
  
  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Mount failed.\n");
    return -1;
  }

  if (fs_create(fname)) {
    fprintf(stderr, "test_file_creation: File creation failed.\n");
    return -1;
  }

  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Unmount failed.\n");
    return -1;
  }

  if (mount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Mount failed.\n");
    return -1;
  }

  if (fs_create(fname)) {
    fprintf(stderr, "test_file_creation: File deletion failed.\n");
    return -1;
  }

  if (umount_fs(DISK_NAME)) {
    fprintf(stderr, "test_file_creation: Unmount failed.\n");
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

  return 0;
}
