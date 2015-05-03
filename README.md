# SANIC_TEEM File System
### *"gotta go FAT"*

This is SANIC_TEEM's File System, implemented for project 6 of CSE 325 at NMT!

// TODO: make better README

### Design

The first block of the filesystem is unique, and contains the file allocation table (FAT). Every file in the system (up to 64) has an entry in the FAT, consisting of the filename as a string (up to 15 characters) and the offset of the first block in the file.

Every other block of the filesystem consists of one `short` containing either the block offset of the next block in the file, `BLOCK_TERMINATOR` (-1) indicating the block is the last in the file, or `BLOCK_FREE` (0) indicating the block is not allocated to a file.

File descriptors are stored in memory, in an array of size 32. A file descriptor consists of the block offset of the first block in the file, and a seek offset indicating the current position in the file.