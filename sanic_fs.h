#ifndef _SANIC_FS_H_
#define _SANIC_FS_H_

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
 * Closes the file descriptor fildes.
 *
 * @return  0 on success, and -1 if the fildes does not exist or is not open.
 */
int fs_close(int fildes);

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
 * descriptor fildes into the buffer pointed to by buf.
 *
 * @return  Number of bytes read on success, -1 on failure.
 */
int fs_read(int fildes, void* buf, size_t nbyte);

/**
 * Attempts to write nbyte bytes of data to the file referenced by the
 * descriptor fildes from the buffer pointed to by buf.
 *
 * @return  Number of bytes actually written on success, -1 on failure.
 */
int fs_write(int fildes, void* buf, size_t nbyte);

/**
 * @return  The current size of the file pointed to by the file descriptor
 *          fildes. In case fildes is invalid, returns -1.
 */
int fs_get_filesize(int fildes);

/**
 * Sets the file pointer (the offset used for read and write operations)
 * associated with the file descriptorfildes to the argument offset.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_lseek(int fildes, off_t offset);

/**
 * Causes the file referenced by fildes to be truncated to length bytes in size.
 * If the file was previously larger than this new size, the extra data is lost
 * and the corresponding data blocks on disk (if any) must be freed.
 *
 * @return  0 on success, -1 on failure.
 */
int fs_truncate(int fildes, off_t length);

#endif
