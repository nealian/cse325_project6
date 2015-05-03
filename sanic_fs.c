
#include <stdlib.h>
#include <unistd.h>
#include "sanic_fs.h"
#include "disk.h"

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
  // TODO
  return -1;
}

int fs_close(int fildes){
  // TODO
  return -1;
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
