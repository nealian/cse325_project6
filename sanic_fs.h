#ifndef _SANIC_FS_H_
#define _SANIC_FS_H_

#define BLOCK_TERMINATOR -2
#define BLOCK_FREE 0

#define SUPER_BLOCK 0

#define MAX_FILES 64
#define MAX_DESCRIPTORS 32
#define MAX_FNAME 16

typedef struct t_directory_entry {
  char filename[MAX_FNAME]; // 16 bytes maximum
  short start; // block offset
  unsigned int size; // file size
} directory_entry;

typedef struct t_file_descriptor {
  int directory_i; // index in directory
  int offset; // seek offset
} file_descriptor;

/**
 * Creates a fresh (and empty) file system on the virtual disk with name
 * disk_name. Should invoke make_disk.
 *
 * @return  0 on success, -1 when the disk disk_name could not be created,
 *          opened, or properly initialized.
 */
int make_fs(char* disk_name);

/**
 * Mounts a file system that is stored on a virtual disk with name disk_name.
 * With this, the system becomes "ready for use."
 *
 * @return  0 on success, and -1 when the disk disk_name could not be opened or
 *          when the disk does not contain a valid file system.
 */
int mount_fs(char* disk_name);

/**
 * Unmounts the file system from a virtual disk with name disk_name. Writes back
 * all information so that the disk persistently reflects all changes that were
 * made to the file system.
 *
 * @return  0 on success, and -1 when the disk disk_name could not be closed or
 *          when data could not be written to the disk (this should not happen).
 */
int umount_fs(char* disk_name);

/**
 * Opens the file specified by name for reading and writing.
 *
 * @return  A non-negative integer file descriptor on success, or -1 on failure.
 */
int fs_open(char* name);

/**
 * Closes the file descriptor filedes.
 *
 * @return  0 on success, and -1 if the filedes does not exist or is not open.
 */
int fs_close(int filedes);

/**
 * Creates a new file with name name in the root directory of the file system.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_create(char* name);

/**
 * Deletes the file with name name from the root directory of the file system.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_delete(char* name);

/**
 * Attempts to read nbyte bytes of data from the file referenced by the
 * descriptor filedes into the buffer pointed to by buf.
 *
 * @return  Number of bytes read on success, -1 on failure.
 */
int fs_read(int filedes, void* buf, size_t nbyte);

/**
 * Attempts to write nbyte bytes of data to the file referenced by the
 * descriptor filedes from the buffer pointed to by buf.
 *
 * @return  Number of bytes actually written on success, -1 on failure.
 */
int fs_write(int filedes, void* buf, size_t nbyte);

/**
 * @return  The current size of the file pointed to by the file descriptor
 *          filedes. In case filedes is invalid, returns -1.
 */
int fs_get_filesize(int filedes);

/**
 * Sets the file pointer (the offset used for read and write operations)
 * associated with the file descriptorfiledes to the argument offset.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_lseek(int filedes, off_t offset);

/**
 * Causes the file referenced by filedes to be truncated to length bytes in size.
 * If the file was previously larger than this new size, the extra data is lost
 * and the corresponding data blocks on disk (if any) must be freed.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_truncate(int filedes, off_t length);

/**
 * Print out the contents of the directory table to standard out. Useful for
 * debugging.
 */
void print_directory();

/**
 * Print out the contents of the descriptor table to standard out. Useful for
 * debugging.
 */
void print_descriptors();


#endif
