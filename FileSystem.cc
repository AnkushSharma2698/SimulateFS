// Needed inlcusions
#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <string>
#include <cstring>
#include <string.h>
#include <vector>
#include <fstream>
#include<map>
#include <cmath>
#include <iterator>
#include <set>
// Headers to be included
#include "FileSystem.h"

using namespace std;

// GLOBAL VARIABLES
Super_block super_block;
fstream disk;
string m_disk_name;
map<int, set<int>> directory_map; // Holds the directories and anything that may currently exist in it
int cwd; // Refers to the name of the currently mounted disk


// HELPERS
void printBits(uint8_t byte) {
   int i;
    for (i = 0; i < 8; i++) {
        printf("%d", !!((byte << i) & 0x80));
    }
    printf("\n");
}

bool check_exists(string name, int current_dir) {
    bool exist = false;
    for (auto it = directory_map[current_dir].begin(); it != directory_map[current_dir].end(); ++it) {
        string inode_name(super_block.inode[*it].name);
        if (inode_name == name) {
            exist = true;
        }
    }
    return exist;
}

int get_index_from_map_by_name(string name, int current_dir) {
    int idx;
    for (auto it = directory_map[current_dir].begin(); it != directory_map[current_dir].end(); ++it) {
        string inode_name(super_block.inode[*it].name);
        if (inode_name == name) {
            idx = *it;
        }
    }
    return idx;
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
void print_str_map(map<int, set<string>> my_map) {
        // REMOVE THIS TEST PRINT CODE
    for (auto it = my_map.begin(); it != my_map.end();++it) {
        cout <<"Key: " << it->first << endl;
    }
}

void tokenize(string str, vector<string>&words) {
    stringstream stream(str);
    string tok;
    while(stream >> tok) words.push_back(tok);

}

void printVector(vector<string> &words) {
    for (auto x: words) {
        cout << x << endl;
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
            case 'M': // Mount a disk
                if (words.size() > 1) {
                    char new_disk_name[5];
                    strcpy(new_disk_name, words[1].c_str());
                    fs_mount(new_disk_name);
                } else {
                    cerr << "Error: Must provide name to mount disk" << endl;
                }
                break;
            case 'C': // Create file or directory
                if (!m_disk_name.empty()) {
                    // Maybe do some error handling here in case we get bad input
                    if (words.size() < 3) {
                        cerr << "Error: Must proved both a Name and Size, respectively" << endl;
                    }else if (words[1].size() > 5) {
                        cerr << "Error: File name can be 5 characters max" << endl;
                    } else {
                        char name[5];
                        strcpy(name, words[1].c_str());
                        fs_create(name, stoi(words[2]));
                    }
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'D': // Deletion Case
                if (!m_disk_name.empty()) {
                    if (words.size() > 1) {
                        char delete_name[5];
                        strcpy(delete_name, words[1].c_str());
                        fs_delete(delete_name);
                     }
                    else{cerr << "Error: Must provide name to delete file/directory" << endl;}
                } else {cerr << "Error: no file system is mounted" << endl;}
                break;
            default:
                cerr << "Error: Command Not Found" << endl;
                break;
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
            if (i == 0 && (mask == 128))  { // Checking if we are looking at the super block
                count++;
			    mask >>= 1;
                continue;
            }
			if (super_block.free_block_list[i] & mask) {
				// Block is apparently being used
				if (!used_block_map[count]) {
                    error_repr(1, new_disk_name);
                    return;
                } else if (used_block_map[count] > 1) {
                     error_repr(1, new_disk_name);
                    return;
                }
			} else {
				// Block is apparently free
				if (used_block_map[count]) { // Check if the block that we are looking at is actually frees
                    // cout << "BLOCK:" << count << endl;
                    error_repr(1, new_disk_name);
                    return;
                }		
			}
			count++;
			mask >>= 1;
		}
	}

    // ===================Consistency check 2 ====================//
    // 2. name of every file/directory must be unique within a directory
    
    directory_map[127]; // Root directory index

    // Iterate each inode to find out which one is directory
    for (int i = 0; i < INODE_NUM; i++) {
        if (!(super_block.inode[i].dir_parent < 128)) {
            // The current index is a directory
            directory_map[i];
        }
    }

    // Go through the inodes now and add the item to the correct parent directory
    uint8_t idx_mask = 127;
    for (int i = 0; i< INODE_NUM; i++) {
        // Check if the inode we are looking at is in use
        if (super_block.inode[i].used_size & (1 << 7)) {
            int idx = super_block.inode[i].dir_parent & idx_mask;
            if (!directory_map[idx].insert(i).second) {
                // There is a duplicate inserted into a set
                error_repr(2, new_disk_name);
                return;
            }
        }
    }

    //============Consistency Check 3 ===============//
    // 3. Free inode must have all bits = 0, else the "name" at least must have one non-zero bit
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & 1<< 7) {
            // Check if the name has at least one bit in it that is not 0
            if (strcmp(super_block.inode[i].name, "\0\0\0\0\0") ==0) {
                error_repr(3, new_disk_name);
                return;
            }
        } else {
            // All every bit in this inode must be 0
            if (!strcmp(super_block.inode[i].name, "\0\0\0\0\0") == 0) {
                error_repr(3, new_disk_name);
                return;
            }
            if (!super_block.inode[i].used_size == 0 || !super_block.inode[i].start_block == 0 || !super_block.inode[i].dir_parent == 0) {
                error_repr(3, new_disk_name);
                return;
            }

        }
    }

    // =================Consistency check 4 ===============//
    // 4. The startblock of every inode that is a file must be between 1-127 inclusive
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & 1<<7) { // Node is in use
            // Check if the inode is a file
            if (super_block.inode[i].dir_parent < 128) {
                if (!(super_block.inode[i].start_block > 0 && super_block.inode[i].start_block < 128)) {
                    error_repr(4, new_disk_name);
                    return;
                }
            }

        }
    }
    
    // ==============Consistency Check 5==================//
    // 5. size and start block of inode that is directory must be 0
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & 1<<7) { // Node is in use
            if (!(super_block.inode[i].dir_parent < 128)) { // We are looking at a directory
                if (!(super_block.inode[i].start_block == 0 && (super_block.inode[i].used_size & 127) == 0 )) {
                    error_repr(5, new_disk_name);
                    return;
                }
            }

        }
    }

    // ==============Consistency check 6=================//
    // 6. Parent can never be 126. If parent inode between 1-125 inclusive, parent inode must be in use and marked as a directory
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & 1<<7) {
            int idx_parent = super_block.inode[i].dir_parent & idx_mask;
            if (idx_parent == 126) {
                error_repr(6, new_disk_name);
                return;
            }

            if (idx_parent >=0 && idx_parent <=125) {
                if (!((super_block.inode[idx_parent].used_size & (1 << 7)) && (super_block.inode[idx_parent].dir_parent < 128))) {
                    error_repr(6, new_disk_name);
                    return;
                }
            }
        }
    }

    // mount the disk and set the cwd
    m_disk_name = new_disk_name;
    cwd = 127;
	disk.close();
}

void error_repr(int error_code, const char * new_disk_name) {
     cerr << "Error: File system in " << new_disk_name <<  " is inconsistent (error code: " <<   error_code << ")" << endl;
}

// CONSISTENCY CHECK 1 HELPER
void check_map_vs_inodes(map<int, int> &block_map) {
    uint8_t base_mask = 1 << 7;
    for (int i = 0; i < INODE_NUM; i++) { // Iterate all of the inodes
        // Check if the inode is in use
        if (super_block.inode[i].used_size & base_mask) {
            // Determine if it is a file or a directory
            if (super_block.inode[i].dir_parent < 128) {
                // Current inode is a file
                // Determine the start block for this
                int start_block_idx = convertByteToDecimal(super_block.inode[i].start_block, BYTE_SIZE);
                int blocks_covered = convertByteToDecimal(super_block.inode[i].used_size, BYTE_SIZE - 1);
                for (int j = start_block_idx; j < start_block_idx  + blocks_covered; j++) {
                    // If the block is listed as in use, but the number of inodes using it is greater than 1
                    int val = block_map[j] + 1;
                    block_map[j] = val;
                }
            }
        }
	}
}

void fs_create(char name[5], int size) {
    string n(name);
    bool is_file = (size > 0)? true: false;
    set<string>::iterator it;
    int start_block = 0;
    // Check for a free inode
    int free_inode = 10000; // Set it to an arbitrarily high value
    for (int i = 0; i < INODE_NUM; i++) {
        if (!(super_block.inode[i].used_size & (1<<7))) { // If the inode is not in use
            free_inode = i;
            break;
        }
    }
    if (free_inode == 10000) { // Fail is no free inode is found
        cerr << "Error: Superblock in disk " << m_disk_name << " is full, cannot create "<< name << endl;
        return;
    }
    // Make sure the name is not a reserved value
    if (n.compare(".") == 0 || n.compare("..") == 0) {
        cerr << "Error: '..' and '.' are reserved" << endl;
        return;
    }
    // Make sure there isnt duplicates in the cwd
    if (check_exists(n, cwd)) { 
        // This name is already in the cwd
        cerr << "Error: File or directory " << name << " already exists" << endl;
        return;
    }

    if (is_file) {
        // Go through the free block list and see if there is space for 'size' contiguous blocks
        int count = 0;
        start_block = 10000;
        int consecutive_blocks = 0;
        for (unsigned int i=0; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++) {
            uint8_t mask = 1<<7; 
            while (mask) {
                if (i == 0 && (mask == 128))  { // Checking if we are looking at the super block
                    count++;
                    mask >>= 1;
                    continue;
                }
                if (super_block.free_block_list[i] & mask) { // This block is in is use
                    consecutive_blocks = 0;
                } else {
                    consecutive_blocks++;
                    if (consecutive_blocks == size) {
                        start_block = count - consecutive_blocks + 1;
                        break;
                    }
                }
                count++;
                mask >>=1;
            }
            if (start_block != 10000) {
                break;
            }
        }

        if (start_block == 10000) {
            // Remove the name from the map too
            // We did not find the consective blocks we wanted to
            cerr << "Cannot allocate " << size << " on " << m_disk_name << endl;
            return;
        }

        // Now set up the file to have the space it needs
        int block_count = 0;
        int start_index = start_block / 8; // Which index to start on in the FBL
        int mask_offset = start_block - (start_index * 8);
        for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
            uint8_t mask = 1<<7; 
            if ((int)i == start_index) {
                mask >>=mask_offset;
            }
            while (mask) {
                if (block_count == size) {
                    break;
                }
                super_block.free_block_list[i] |= mask;
                block_count++;
                mask>>=1;
            }
            if (block_count == size) {
                break;
            }
        }

    }
    
    // Manipulate the inode
    strncpy(super_block.inode[free_inode].name, name, 5); // Set Name
    super_block.inode[free_inode].start_block = start_block; // Set start block
    if (is_file) { // This is a file
        super_block.inode[free_inode].used_size = 128 | size; // Set SIZE
    } else { // A directory
        super_block.inode[free_inode].used_size = 128;
    }
    if (is_file) {
        super_block.inode[free_inode].dir_parent = 0 | cwd;
    } else {
        super_block.inode[free_inode].dir_parent = 128 | cwd;
    }

    directory_map[cwd].insert(free_inode);

    // Write the superblock to memory again
    disk.open(m_disk_name);
    // write the FB list into memory
	disk.write(super_block.free_block_list, FREE_SPACE_LIST);
    // Write the rest of the inodes into memory into the inode list
	for (uint8_t i = 0; i < INODE_NUM; i++) {
		disk.write(super_block.inode[i].name, 5); // Read the name into mem
		disk.write((char*)&super_block.inode[i].used_size, 1);
		disk.write((char*)&super_block.inode[i].start_block, 1);
		disk.write((char*)&super_block.inode[i].dir_parent, 1);
	}
    disk.close();
}

void fs_delete(char name[5]) {
    // Check if the file or directory exists in the cwd
    string n(name);
    if (!check_exists(n, cwd)) {cerr << "File or directory " << n << " does not exist" << endl; return;}
    int inode_idx = get_index_from_map_by_name(n, cwd);
    disk.open(m_disk_name);
    recursive_delete(inode_idx, cwd);
    disk.close();
}

void recursive_delete(int idx, int cwd) {
    // If directory
    if (!(super_block.inode[idx].dir_parent < 128)) {
        for (auto it = directory_map[idx].begin(); it != directory_map[idx].end(); ++it) {
            recursive_delete(*it, idx);
        }
        // Remove the directory from the map by its own idx
        map<int, set<int>>::iterator it;
        it = directory_map.find(idx);
        directory_map.erase(it);
    }
    // Remove the directory index from the set of the parent dir
    set<int>::iterator it;
    it = directory_map[cwd].find(idx);
    
    int start_block = convertByteToDecimal(super_block.inode[idx].start_block, BYTE_SIZE);
    int blocks_covered = convertByteToDecimal(super_block.inode[idx].used_size, BYTE_SIZE - 1);
    // Set the bits in the free space list to 0
    // Now set up the file to have the space it needs
    int block_count = 0;
    int start_index = start_block / 8; // Which index to start on in the FBL
    int mask_offset = start_block - (start_index * 8);
    for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
        uint8_t mask = 1<<7; 
        if ((int)i == start_index) {
            mask >>=mask_offset;
        }
        while (mask) {
            if (block_count == blocks_covered) {
                break;
            }
            super_block.free_block_list[i] ^= mask;
            block_count++;
            mask>>=1;
        }
        if (block_count == blocks_covered) {
            break;
        }
    }
    //Writing super block into mem
    disk.write(super_block.free_block_list, FREE_SPACE_LIST);
    // Zero out the corresponding inode in memory
    disk.seekp(FREE_SPACE_LIST + (idx * 8), ios_base::beg);
    disk.write("\0\0\0\0\0\0\0\0", 8);

    
    // Erase from the actual blocks
    // MUST DO THIS PART
    // Zero out the inode
    super_block.inode[idx].start_block = 0;
    super_block.inode[idx].used_size = 0;
    memset(super_block.inode[idx].name, 0, 5);
    super_block.inode[idx].dir_parent = 0;
}

void fs_read(char name[5], int block_num) {
    // Read 1kb data into buffer
    string n(name);
    check_exists(n, cwd);
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
