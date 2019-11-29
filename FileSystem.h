#define cout(n) cout << Line: __LINE__ << " "<< (n) << endl;
#define init_map(map, size) { \
	for (int i=0; i<(size);i++ ) { \
		(map)[i] = false; \
	} \
}
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <map>

using namespace std;

typedef struct {
	char name[5];        // Name of the file or directory (First 5 bytes)
	uint8_t used_size;   // Inode state and the size of the file or directory
	uint8_t start_block; // Index of the start file block
	uint8_t dir_parent;  // Inode mode and the index of the parent inode
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
void fs_buff(uint8_t buff[1024]);
void fs_ls(void);
void fs_resize(char name[5], int new_size);
void fs_defrag(void);
void fs_cd(char name[5]);

// Helper methods
void check_map_vs_inodes(map<int, bool> &block_map);
