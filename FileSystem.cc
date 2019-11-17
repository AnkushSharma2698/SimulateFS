// Needed inlcusions
#include <iostream>
#include <sys/stat.h>

// Headers to be included
#include "FileSystem.h"

void fs_mount(char *new_disk_name) {
    // Check if a disk with the given name exists in the cwd
    // Load the superblock of the file system
    // Check file system Consistency: --> DO NOT MOUNT, if Consistency check fails
    // 1.Free blocks in free space lists must not be allocated to anythimg
    // 1. Blocks in use --> allocated to EXACTLY 1 file
    // 2. name of every file/directory must be unique within a directory
    // 3. Free inode must have all bits = 0, else the "name" at least must have one non-zero bit
    // 4. The startblock of every inode that is a file must be between 1-127 inclusive
    // 5. size and start block of inode that is directory must be 0
    // 6. Parent can never be 126. If parent inode between 1-125 inclusive, parent inode must be in use and marked as a directory

    // If all is well set cwd to root
    // NOTE: DO NOT FLUSH BUFFER WHEN MOUNTING A FS
}
void fs_create(char name[5], int size) {

}
void fs_delete(char name[5]) {

}
void fs_read(char name[5], int block_num) {

}
void fs_write(char name[5], int block_num) {

}
void fs_buff(uint8_t buff[1024]) {

}
void fs_ls(void) {

}
void fs_resize(char name[5], int new_size) {

}
void fs_defrag(void) {

}
void fs_cd(char name[5]) {

}