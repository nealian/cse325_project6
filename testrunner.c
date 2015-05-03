#include <stdio.h>
#include "sanic_fs.h"

/**
 * Make new filesystem, mount it, unmount it, re-mount it, and finally
 * unmount again.
 */
int test_fs_creation() {
  char* disk_name = "test.fs";

  if (make_fs(disk_name)) {
    fprintf(stderr, "test_fs_creation: FS creation failed.\n");
    return -1;
  }

  if (mount_fs(disk_name)) {
    fprintf(stderr, "test_fs_creation: Initial mount failed.\n");
    return -1;
  }

  if (umount_fs(disk_name)) {
    fprintf(stderr, "test_fs_creation: Initial unmount failed.\n");
    return -1;
  }

  if (mount_fs(disk_name)) {
    fprintf(stderr, "test_fs_creation: Second mount failed.\n");
    return -1;
  }

  if (umount_fs(disk_name)) {
    fprintf(stderr, "test_fs_creation: Second unmount failed.\n");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {

  if (test_fs_creation()) {
    printf("test_fs_creation failed.\n");
  } else {
    printf("test_fs_creation successful.\n");
  }
  
  return 0;
}
