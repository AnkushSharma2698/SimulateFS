#define cout(n) cout << Line: __LINE__ << " "<< (n) << endl;
#define init_map(map, size) { \
	for (int i=0; i<(size);i++ ) { \
		(map)[i] = 0; \
	} \
}
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <map>

#define FREE_SPACE_LIST 16
#define INODE_NUM 126
#define BYTE_SIZE 8
#define NUM_BLOCKS 128

using namespace std;

typedef struct {
	char name[5];        // Name of the file or directory (First 5 bytes)
	char used_size;   // Inode state and the size of the file or directory
	char start_block; // Index of the start file block
	char dir_parent;  // Inode mode and the index of the parent inode
} Inode;

typedef struct {
	char free_block_list[16]; // 16 bytes
	Inode inode[126]; //each Inode is 8 bytes
} Super_block;

void fs_mount(const char *new_disk_name);
void fs_create(char name[5], int size);
void fs_delete(char name[5]);
void fs_read(char name[5], int block_num);
void fs_write(char name[5], int block_num);
void fs_buff(char buff[1024]);
void fs_ls(void);
void fs_resize(char name[5], int new_size);
void fs_defrag(void);
void fs_cd(char name[5]);

// Helper methods
void check_map_vs_inodes(map<int, int> &block_map);
void error_repr(int error_code, const char * new_disk_name);
void recursive_delete(int idx, int cwd);
void get_name_from_inode(int index, char * name_array);
int number_items_in_dir(int directory_idx);
int get_size_of_file(int file_idx);