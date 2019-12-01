// Needed inlcusions
#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include<map>
#include <cmath>
#include <iterator>
// Headers to be included
#include "FileSystem.h"

#define FREE_SPACE_LIST 16
#define INODE_NUM 126
#define BYTE_SIZE 8
#define NUM_BLOCKS 128

using namespace std;

// GLOBAL VARIABLES
Super_block super_block;
fstream disk;


// HELPERS

void printBits(uint8_t byte) {
   int i;
    for (i = 0; i < 8; i++) {
        printf("%d", !!((byte << i) & 0x80));
    }
    printf("\n");
}

void print_map(map<int,int> &my_map) {
    map<int, int>::iterator itr; 
    cout << "\nThe map gquiz1 is : \n"; 
    cout << "\tKEY\tELEMENT\n"; 
    for (itr = my_map.begin(); itr != my_map.end(); ++itr) { 
        cout << '\t' << itr->first 
             << '\t' << itr->second << '\n'; 
    } 
    cout << endl; 
}

void tokenize(string str, vector<string>&words) {
    stringstream stream(str);
    string tok;
    while(stream >> tok) words.push_back(tok);

}

void printVector(vector<string> &words) {
    for (auto x: words) {
        cout << x << " ";
    }
    cout << endl;
}

int convertByteToDecimal(uint8_t n, int iterations){
    int sum = 0;
    int decimal = 1;
    uint8_t mask = (1 << 1) - 1; // mask  00000001
    for (int i = 0; i < iterations; i++ ) {
        if ((n & mask)) {
            sum +=decimal;
        }
        mask <<= 1;
        decimal *=2;
    }
    return sum;
}


int main(int argc, char *argv[]) {
    string command;
    while (getline(cin, command)) {
        vector<string> words;
        // Turn command into an vector of string
        tokenize(command, words);

        switch(command[0]) {
            case 'M':
                if (words.size() > 1) {
                    fs_mount(words[1].c_str());
                } else {
                    cout << "Error: Must provide name to mount disk" << endl;
                }
                break;
            default:
                cout << "Error: Command Not Found" << endl;
        }

    }

    return 0;
}

// Implementations of the filesystem
void fs_mount(const char *new_disk_name) {

    // Open the disk
    disk.open(new_disk_name);
    
    if (!disk) {
        cerr << "Error: Cannot find disk " << new_disk_name << endl;
        return;
    }

    // Read the FB list into memory
	disk.read(super_block.free_block_list, FREE_SPACE_LIST);
    // Read the rest of the inodes into memory into the inode list
	for (uint8_t i = 0; i < INODE_NUM; i++) {
		disk.read(super_block.inode[i].name, 5); // Read the name into mem
		disk.read((char*)&super_block.inode[i].used_size, 1);
		disk.read((char*)&super_block.inode[i].start_block, 1);
		disk.read((char*)&super_block.inode[i].dir_parent, 1);
	}
	
	//===========Consistency Check 1============//
    map<int, int> used_block_map;
    init_map(used_block_map, NUM_BLOCKS);
    //Consistency check 1
	check_map_vs_inodes(used_block_map);
    int count = 0;
	for (unsigned int i=0; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++) {
		uint8_t mask = 1 << 7;
		while(mask) {
            if (i == 0 && (mask == 128))  {
                count++;
			    mask >>= 1;
                continue;
            }
			if (super_block.free_block_list[i] & mask) {
				// Block is apparently being used
				if (!used_block_map[count]) {
                    cout << "Error: File system in " << new_disk_name <<  " is inconsistent (error code: " <<   "1" << ")" << endl;
                    return;
                } else if (used_block_map[count] > 1) {
                    cout << "Error: File system in " << new_disk_name <<  " is inconsistent (error code: " <<   "1" << ")" << endl;
                    return;
                }
			} else {
				// Block is apparently free
				if (used_block_map[count]) { // Check if the block that we are looking at is actually frees
                    // cout << "BLOCK:" << count << endl;
                    cout << "Error: File system in " << new_disk_name <<  " is inconsistent (error code: " <<   "1" << ")" << endl;
                    return;
                }		
			}
			count++;
			mask >>= 1;
		}
	}
    cout << "Passed consistency check 1" << endl;

	// Consistency check 2
    // 2. name of every file/directory must be unique within a directory
    

	disk.close();
    // Load the superblock of the file system <-- Read super block into mem
    // Check file system Consistency: --> DO NOT MOUNT, if Consistency check fails
    // 3. Free inode must have all bits = 0, else the "name" at least must have one non-zero bit
    // 4. The startblock of every inode that is a file must be between 1-127 inclusive
    // 5. size and start block of inode that is directory must be 0
    // 6. Parent can never be 126. If parent inode between 1-125 inclusive, parent inode must be in use and marked as a directory

    // If pass, unmount the other ffd
    // If all is well set cwd to root of new file disk
    // NOTE: DO NOT FLUSH BUFFER WHEN MOUNTING A FS
}

// This method is used as part of consistency check 1
void check_map_vs_inodes(map<int, int> &block_map) {
    uint8_t base_mask = 1 << 7;
    for (int i = 0; i < INODE_NUM; i++) { // Iterate all of the inodes
        // Check if the inode is in use
        if (super_block.inode[i].used_size & base_mask) {
            // Determine if it is a file or a directory
            if (super_block.inode[i].dir_parent < 128) {
                // Determine the start block for this
                int start_block_idx = convertByteToDecimal(super_block.inode[i].start_block, BYTE_SIZE);
                int blocks_covered = convertByteToDecimal(super_block.inode[i].used_size, BYTE_SIZE - 1);
                // Do i even need to look at dir parent??
                for (i = start_block_idx; i < start_block_idx  + blocks_covered; i++) {
                    // If the block is listed as in use, but the number of inodes using it is greater than 1
                    int val = block_map[i] + 1;
                    block_map[i] = val;
                }
            }
        }
	}
}


void fs_create(char name[5], int size) {
    // Create a file on the ffd
    // Make sure to check that there is an available disk right now
    // If there is, make a file, create file under the cwd
    // The cwd is not always the root
    // Assume the size of the file is given, make sure file is able to be allocated in contiguous space
    // size 0 means user wants to make a directory
    // Check the name, make sure it is unique in the same cwd
    // . and .. are reserved. .. in root means stay in root
}
void fs_delete(char name[5]) {
    // delete file or dir
    // if directory is selected, remove everything inside it
    // Must change the file block and inodes
}
void fs_read(char name[5], int block_num) {
    // Read 1kb data into buffer
}
void fs_write(char name[5], int block_num) {
    // Put 1kn data in buffer to the specified block
}
void fs_buff(uint8_t buff[1024]) {
    // put user input to buffer
}
void fs_ls(void) {
    // ls should just list the files
    // Cant call the unix file ls. Wanna list ffd stuff only
}
void fs_resize(char name[5], int new_size) {
    // because we assume we give the size of file.
    // Need to call function so we can change the size of file
    //case 1 : new size is greater than prev
    //      Want to add blocks at end, but there may be cases where we cannot do that, in this case we have to move them into a new space that can fit this
    //      If we shift, we have to clean up everything in those positions
    // case 2 : new size is less than prev
}
void fs_defrag(void) {
    // Organize contentes of all files in ffd
    // Put all small peices of available blocks together so they are continuous
    // Maintain the order of files even after the defrag
}
void fs_cd(char name[5]) {
    // Same as the terminal to change the current working directory
    // If the name doesnt exist just print out the error message
}
