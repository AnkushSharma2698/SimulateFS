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
#include <algorithm>
// Headers to be included
#include "FileSystem.h"

using namespace std;

// GLOBAL VARIABLES
Super_block super_block;
char buffer[1024];
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

bool inRange(unsigned int low, unsigned int high, unsigned int x) {
    return (low <= x && x <= high);
}

bool check_exists(string name, int current_dir) {
    bool exist = false;
    for (auto it = directory_map[current_dir].begin(); it != directory_map[current_dir].end(); ++it) {
        char name_array[6] = {0,0,0,0,0,0};
        get_name_from_inode(*it, name_array);
        string inode_name(name_array);
        if (inode_name.size() == 6) {
            inode_name.erase(5, 1);
        }
        if (name.size() == 6) {
            name.erase(5, 1);
        }
        if (inode_name.compare(name) == 0) {
            exist = true;
            break;
        }
    }
    return exist;
}

int get_index_from_map_by_name(string name, int current_dir) {
    int idx;
    for (auto it = directory_map[current_dir].begin(); it != directory_map[current_dir].end(); ++it) {
        char name_array[6] = {0,0,0,0,0,0};
        get_name_from_inode(*it, name_array);
        string inode_name(name_array);
        if (inode_name.size() == 6) {
            inode_name.erase(5, 1);
        }
        if (inode_name.compare(name)==0) {
            idx = *it;
            break;
        }
    }
    return idx;
}

void get_name_from_inode(int index, char * name_array) {
    for (int i = 0; i < 6; i++) {
        if (super_block.inode[index].name[i] != '\0') {
            name_array[i] = super_block.inode[index].name[i];
        } else {
            break;
        }
    }
}

void transfer_char_to_char_array(char * output_array, char * input_array) {
    for (int i = 0; i < 6; i++) {
        if (input_array[i] != '\0') {
            output_array[i] = input_array[i];
        } else {
            break;
        }
    }
}

void read_into_buf(string cmd, char * buf) {
    memset(buf, '\0', 1024);
    // Find B First
    string::iterator initial_it = cmd.begin();
    while (*initial_it != 'B') {
        ++initial_it;
    }
    int count = 0; // Make sure no more than 1024 chars
    for (auto it = initial_it + 2; it!=cmd.end() && count != 1024; ++it) {
        buf[count] = *it;
        count++;
    }
    // Set the whole buffer to 0;
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
    // Set buffer to 0
    memset(buffer, 0, 1024);
    // Holds the user
    string command;
    int line_num = 0;
    string input_file_name = argv[1];
    fstream input_file(input_file_name);
    while (getline(input_file, command)) {
        ++line_num;
        vector<string> words;
        // Turn command into an vector of string
        tokenize(command, words);
        if (command.size() == 0) {
            continue;
        }
        switch(words[0][0]) {
            case 'M': // Mount a disk
                if (words.size() == 2) {
                    fs_mount(words[1].c_str());
                } else {
                    cerr << "Command Error: " << input_file_name << ", "<< line_num << endl;
                }
                break;
            case 'C': // Create file or directory
                if (!m_disk_name.empty()) {
                    // Maybe do some error handling here in case we get bad input
                    if (words.size() < 3) {
                        cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
                    }else if (words[1].size() > 5) {
                        cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
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
                    else{cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;}
                } else {cerr << "Error: no file system is mounted" << endl;}
                break;
            case 'R': // Read From file case
                if (!m_disk_name.empty()) {
                    if (words.size() == 3) {
                        char read_name[5];
                        strcpy(read_name, words[1].c_str());
                        fs_read(read_name, stoi(words[2]));
                    } else {
                        cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
                    }
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'W': // Writing to file from buffer
                if (!m_disk_name.empty()) {
                    if (words.size() == 3) {
                        char write_name[5];
                        strcpy(write_name, words[1].c_str());
                        fs_write(write_name, stoi(words[2]));
                    } else {
                        cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
                    }
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'B':
                // Special Parsing
                char buf[1024]; // This will hold a maximum of 1024 characters
                if (!m_disk_name.empty()) {
                    read_into_buf(command, buf);
                    fs_buff(buf);
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'L':
                if (!m_disk_name.empty()) {
                    fs_ls();
                } else {
                     cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'E':
                if (!m_disk_name.empty()) {
                    if (words.size() == 3) {
                        char resize_name[5];
                        strcpy(resize_name, words[1].c_str());
                        fs_resize(resize_name, stoi(words[2]));
                    } else {
                        cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
                    }
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            case 'O':
                if (!m_disk_name.empty()) {
                    fs_defrag();
                } else {
                    cerr << "Error: no file system is mounted" << endl;
                }
                break;
            default:
                cerr << "Command Error: " << input_file_name << ", "<<line_num << endl;
                break;
        }

    }
    disk.close();
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
		disk.read(&super_block.inode[i].used_size, 1);
		disk.read(&super_block.inode[i].start_block, 1);
		disk.read(&super_block.inode[i].dir_parent, 1);
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
    // Clear the map 
    directory_map.clear();
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
            int idx;
            int dir_parent_val = convertByteToDecimal(super_block.inode[i].dir_parent, BYTE_SIZE);

            if (dir_parent_val > 127) {
                idx = dir_parent_val - 128;
            } else {
                idx = dir_parent_val;
            }
            char name_array[6] = {0,0,0,0,0,0};
            get_name_from_inode(i, name_array);
            string n(name_array);
            if (check_exists(n, idx)) {
                error_repr(2, new_disk_name);
                return;
            } else {
                directory_map[idx].insert(i);
            }
        }
    }

    //============Consistency Check 3 ===============//
    // 3. Free inode must have all bits = 0, else the "name" at least must have one non-zero bit
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & 1<< 7) {
            // Check if the name has at least one bit in it that is not 0
            bool has_non_zero = false;
            for(int j=0; j < 5; j++) {
                if (super_block.inode[i].name[j] != 0) {

                    has_non_zero = true;
                    break;
                }
            }
            if (!has_non_zero) {
                error_repr(3, new_disk_name);
                return;
            }
        } else {
            // All every bit in this inode must be 0
            bool has_non_zero = false;
            for(int j=0; j < 5; j++) {
                if (super_block.inode[i].name[j] != 0) {
                    has_non_zero = true;
                    break;
                }
            }
            if (has_non_zero) {
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
            int dir_parent_val =  convertByteToDecimal(super_block.inode[i].dir_parent, BYTE_SIZE);
            if (dir_parent_val < 128) {
                int start_block_val =  convertByteToDecimal(super_block.inode[i].start_block, BYTE_SIZE);
                if (!(start_block_val > 0 && start_block_val < 128)) {
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
            int dir_parent_val =  convertByteToDecimal(super_block.inode[i].dir_parent, BYTE_SIZE);
            if (dir_parent_val > 127) { // We are looking at a directory
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
                // Check if in use
                if (!(super_block.inode[idx_parent].used_size & 1<<7)) {
                    error_repr(6, new_disk_name);
                    return;   
                }
                // Check if the inode is marked as a dir
                int dir_parent_val =  convertByteToDecimal(super_block.inode[idx_parent].dir_parent, BYTE_SIZE);
                if (dir_parent_val < 128) {
                    // This is pointing to a file so fail
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
    disk.close();
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
    // Make sure there isnt duplicates in the cwd
    char new_name[6] = {0,0,0,0,0,0};
    transfer_char_to_char_array(new_name, name);
    string n(new_name);
    bool is_file = (size > 0)? true: false;
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
    strncpy(super_block.inode[free_inode].name, new_name, 5); // Set Name
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
    char new_name[6] = {0,0,0,0,0,0};
    transfer_char_to_char_array(new_name, name);
    string n(new_name);
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
    directory_map[cwd].erase(it);
    
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
    if (start_block != 0) { // This means we are looking at a file
        char one_block[1024];
        memset(one_block, 0, 1024);
        disk.seekp(1024 * start_block, ios_base::beg);
        for (int i = 0; i < block_count; i++) {
            disk.write(one_block, 1024);
        }
    }
    // MUST DO THIS PART
    // Zero out the inode
    super_block.inode[idx].start_block = 0;
    super_block.inode[idx].used_size = 0;
    memset(super_block.inode[idx].name, 0, 5);
    super_block.inode[idx].dir_parent = 0;
}

void fs_read(char name[5], int block_num) {
    // Read 1kb data into buffer
    // Make sure there isnt duplicates in the cwd
    char new_name[6] = {0,0,0,0,0,0};
    transfer_char_to_char_array(new_name, name);
    string n(new_name);
    if (!check_exists(n, cwd)) { // This checks to see whatever we are looking for exists
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }
    int idx = get_index_from_map_by_name(name, cwd);
    int dir_parent_val = convertByteToDecimal(super_block.inode[idx].dir_parent, BYTE_SIZE);
    if (dir_parent_val > 127) { // Check if what we are looking for is a directory
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }

    // Get the total blocks covered
    int blocks_covered = convertByteToDecimal(super_block.inode[idx].used_size, BYTE_SIZE - 1);
    if (!inRange(0,blocks_covered - 1, block_num)) {
        cerr << "Error: File " << n << " does not have block "<< block_num <<endl;
        return;
    }
    // Reading from the disk
    disk.open(m_disk_name);
    int start_block_idx = convertByteToDecimal(super_block.inode[idx].start_block, BYTE_SIZE);
    disk.seekg(1024*start_block_idx + (block_num * 1024),ios_base::beg);
    disk.read(buffer, 1024);
    disk.close();


}
void fs_write(char name[5], int block_num) {
    // Put 1kn data in buffer to the specified block
    char new_name[6] = {0,0,0,0,0,0};
    transfer_char_to_char_array(new_name, name);
    string n(new_name);

    if (!check_exists(n, cwd)) { // This checks to see whatever we are looking for exists
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }
    int idx = get_index_from_map_by_name(name, cwd);
    int dir_parent_val = convertByteToDecimal(super_block.inode[idx].dir_parent, BYTE_SIZE);
    if (dir_parent_val > 127) { // Check if what we are looking for is a directory
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }

    // Get the total blocks covered
    int blocks_covered = convertByteToDecimal(super_block.inode[idx].used_size, BYTE_SIZE - 1);
    if (!inRange(0,blocks_covered - 1, block_num)) {
        cerr << "Error: File " << n << " does not have block "<< block_num <<endl;
        return;
    }

    // Write from the buffer
    disk.open(m_disk_name);
    int start_block_idx = convertByteToDecimal(super_block.inode[idx].start_block, BYTE_SIZE);
    disk.seekp(1024*start_block_idx + (block_num * 1024),ios_base::beg);
    disk.write(buffer, 1024);
    disk.close();

}
void fs_buff(char buff[1024]) {
    // put user input to buffer
    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
        buffer[i] = buff[i];
    }
}
void fs_ls(void) {
    uint8_t idx_mask = 127;
    int num_cwd_items;
    int num_parent_dir_items;
    // ls should just list the files
    num_cwd_items = number_items_in_dir(cwd) + 2;
    if (cwd == 127) {
        num_parent_dir_items = number_items_in_dir(cwd) + 2;
    } else {
        // TODO: Get parent dir of this inode
        int idx_parent = super_block.inode[cwd].dir_parent & idx_mask;
        num_parent_dir_items = number_items_in_dir(idx_parent) + 2;
    }

    cout << "." << "         " << num_cwd_items << endl; // 8 spaces
    cout << ".." << "        "<< num_parent_dir_items << endl; // 7 spaces
    for (auto it = directory_map[cwd].begin(); it != directory_map[cwd].end(); ++it) {
        char name_array[6] = {0,0,0,0,0,0};
        get_name_from_inode(*it, name_array);
        string inode_name(name_array);
        int offset;
        if (inode_name.size() == 6) {
            offset = 5;
        } else {
            offset = 10 - inode_name.size();
        }   
        int dir_parent_val = convertByteToDecimal(super_block.inode[*it].dir_parent, BYTE_SIZE);
        if (dir_parent_val < 128) { // We are looking at a file
            int file_size = get_size_of_file(*it);
            inode_name.append("          ", offset);
            cout << inode_name << file_size << " KB" << endl;
        } else { // directory
            int dir_count = number_items_in_dir(*it) + 2;
            inode_name.append("          ", offset);
            cout << inode_name << dir_count << endl;
        }
    }
}

// LS Helpers
int number_items_in_dir(int directory_idx) {
    int count = 0;
     for (auto it = directory_map[directory_idx].begin(); it != directory_map[directory_idx].end(); ++it) {
        ++count;
     }
     return count;
}

// LS HELPER
int get_size_of_file(int file_idx) {
    int blocks_covered = convertByteToDecimal(super_block.inode[file_idx].used_size, BYTE_SIZE - 1);
    return blocks_covered;
}

void fs_resize(char name[5], int new_size) {
    char new_name[6] = {0,0,0,0,0,0};
    transfer_char_to_char_array(new_name, name);
    string n(new_name);
    if (!check_exists(n, cwd)) { // This checks to see whatever we are looking for exists
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }
    int idx = get_index_from_map_by_name(name, cwd);
    int dir_parent_val = convertByteToDecimal(super_block.inode[idx].dir_parent, BYTE_SIZE);
    if (dir_parent_val > 127) { // Check if what we are looking for is a directory
        cerr << "Error: File " << n << " does not exist"<< endl;
        return;
    }

    // ON TO THE RESIZE CODE
    int blocks_covered = convertByteToDecimal(super_block.inode[idx].used_size, BYTE_SIZE - 1);
    int start_block = convertByteToDecimal(super_block.inode[idx].start_block, BYTE_SIZE);
    int iteration_block_start = start_block + blocks_covered; // This the block that is currently not in use after the last used block 
    if (new_size > blocks_covered) {
        // Greater than case
        int consecutive_blocks = 0;
        int needed_blocks = new_size - blocks_covered;
        bool has_free_space = true;
        // int count = 0;
        int start_index = iteration_block_start / 8; // Which index to start on in the FBL
        cout << "The start block of " << n <<  " is " << iteration_block_start << endl;
        cout << "The start index of " << n <<  " is " << start_index << endl;
        int mask_offset = iteration_block_start - (start_index * 8);
        for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
            uint8_t mask = 1<<7; 
            if ((int)i == start_index) {
                mask >>=mask_offset;
            }
            while (mask) {
                if (super_block.free_block_list[i] & mask) { // This block is in is use
                    consecutive_blocks = 0;
                    has_free_space = false;
                    break;
                } else {
                    consecutive_blocks++;
                    if (consecutive_blocks == needed_blocks) {
                        break;
                    }
                }
                mask >>=1;
            }
            if (consecutive_blocks == needed_blocks) {
                cout << "has enough space to expand" << endl;
                break;
            }
            if (!has_free_space) {
                cout << "does not have enough space" << endl;
                break;
            }
        }
        // Check if it has free space available contiguosly
        if (has_free_space) {
            // Allocate the memory to this by adding a bit more space to the used_size of the inode
            disk.open(m_disk_name);
            //Update the fbl and the inode, these blocks are already free so we dont need to worry about overwriting anything
            // Update the free space list
            int block_count = 0;
            int blocks_to_add = new_size - blocks_covered;
            int block_to_start_at = start_block + new_size - blocks_to_add; 
            int start_index = (block_to_start_at) / 8; // Which index to start on in the FBL
            int mask_offset = (block_to_start_at) - (start_index * 8);
            for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
                uint8_t mask = 1<<7; 
                if ((int)i == start_index) {
                    mask >>=mask_offset;
                }
                while (mask) {
                    if (block_count == blocks_to_add) {
                        break;
                    }
                    super_block.free_block_list[i] |= mask;
                    block_count++;
                    mask>>=1;
                }
                if (block_count == blocks_to_add) {
                    break;
                }
            }
            disk.seekp(0, ios_base::beg);
            disk.write(super_block.free_block_list, FREE_SPACE_LIST);
            // Update the inode
            super_block.inode[idx].used_size = 128 | new_size; // Set SIZE
            for (uint8_t i = 0; i < INODE_NUM; i++) {
                disk.write(super_block.inode[i].name, 5); // Read the name into mem
                disk.write((char*)&super_block.inode[i].used_size, 1);
                disk.write((char*)&super_block.inode[i].start_block, 1);
                disk.write((char*)&super_block.inode[i].dir_parent, 1);
            }
            disk.close();
            
        } else { // Does not have contiguous space to add to end, so we search for it through the whole space
            // Loop through the fbl to find consecutive blocks of new_size that are free
            int count = 0;
            int new_start_block = 10000;
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
                        if (consecutive_blocks == new_size) {
                            new_start_block = count - consecutive_blocks + 1;
                            break;
                        }
                    }
                    count++;
                    mask >>=1;
                }
                if (new_start_block != 10000) {
                    break;
                }
            }

            if (new_start_block == 10000) {
                // Remove the name from the map too
                // We did not find the consective blocks we wanted to
                cerr << "Error: File " << n << " cannot expand to size " << new_size << endl;
                return;
            }

            // Allocate size to the block from its new start block
            // manipulate the free space list
            
            cout << "Can allocate space to this file in a new spot" << endl;
            cout << "New start block is" << new_start_block << endl;
            disk.open(m_disk_name);
            
            // Copy the blocks over to the new area
            for(int i = 0; i < blocks_covered; i++) {
                char data[1024];
                char empty[1024];
                memset(empty, 0, 1024);
                // Old block
                disk.seekg(1024 * start_block + (1024 * i), ios_base::beg); // Move read ptr to the current block
                disk.read(data, 1024); // Read data from curr block into data array
                disk.seekp(1024 * start_block + (1024 * i), ios_base::beg); // Move write ptr to current block
                disk.write(empty, 1024);
                // seek out the new block
                disk.seekp(1024 * new_start_block + (1024 * i), ios_base::beg);
                disk.write(data, 1024);
            }

            // Update the fbl bits
            // 1. clear the old bits
            int block_count = 0;
            int index_to_start_deletion = start_block;
            int start_index = (index_to_start_deletion) / 8; // Which index to start on in the FBL
            int mask_offset = (index_to_start_deletion) - (start_index * 8);
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
            // 2. Update the new bits
            block_count = 0;
            int index_to_start_addition = new_start_block;
            start_index = (index_to_start_addition) / 8; // Which index to start on in the FBL
            mask_offset = (index_to_start_addition) - (start_index * 8);
            for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
                uint8_t mask = 1<<7; 
                if ((int)i == start_index) {
                    mask >>=mask_offset;
                }
                while (mask) {
                    if (block_count == new_size) {
                        break;
                    }
                    super_block.free_block_list[i] |= mask;
                    block_count++;
                    mask>>=1;
                }
                if (block_count == new_size) {
                    break;
                }
            }
            // Write the fbl and the inode
            disk.seekp(0, ios_base::beg);
            disk.write(super_block.free_block_list, FREE_SPACE_LIST);
            
            // Update the inode with the new size
            super_block.inode[idx].start_block = new_start_block;
            super_block.inode[idx].used_size = 128 | new_size;

            for (uint8_t i = 0; i < INODE_NUM; i++) {
                disk.write(super_block.inode[i].name, 5); // Read the name into mem
                disk.write((char*)&super_block.inode[i].used_size, 1);
                disk.write((char*)&super_block.inode[i].start_block, 1);
                disk.write((char*)&super_block.inode[i].dir_parent, 1);
            }
            disk.close();
        }   
    } else if (new_size < blocks_covered){
        //  ===========Less than case ============= // 
        cout << "in the less than case" << endl;
        // Zero out the specified blocks
        disk.open(m_disk_name);
        // How many blocks we need to zero out
        int blocks_to_zero = blocks_covered - new_size;
        // Seek out the blocks to be zeroed out
        disk.seekp(1024*start_block + (new_size * 1024), ios_base::beg);
        char one_block[1024];
        memset(one_block, 0, 1024);
        // Overwrite the blocks
        for (int i = 0; i < blocks_to_zero; i++) {
            disk.write(one_block, 1024);
        }
        // Update the free space list
        int block_count = 0;
        int index_to_start_deletion = (start_block + blocks_covered) - blocks_to_zero;
        int start_index = (index_to_start_deletion) / 8; // Which index to start on in the FBL
        int mask_offset = (index_to_start_deletion) - (start_index * 8);
        for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
            uint8_t mask = 1<<7; 
            if ((int)i == start_index) {
                mask >>=mask_offset;
            }
            while (mask) {
                if (block_count == blocks_to_zero) {
                    break;
                }
                super_block.free_block_list[i] ^= mask;
                block_count++;
                mask>>=1;
            }
            if (block_count == blocks_to_zero) {
                break;
            }
        }
        disk.seekp(0, ios_base::beg);
        disk.write(super_block.free_block_list, FREE_SPACE_LIST);
        // Update the used size of inode
         // Manipulate the inode
        super_block.inode[idx].used_size = 128 | new_size; // Set SIZE
        for (uint8_t i = 0; i < INODE_NUM; i++) {
            disk.write(super_block.inode[i].name, 5); // Read the name into mem
            disk.write((char*)&super_block.inode[i].used_size, 1);
            disk.write((char*)&super_block.inode[i].start_block, 1);
            disk.write((char*)&super_block.inode[i].dir_parent, 1);
	    }
        disk.close();
    }
}

void fs_defrag(void) {
    vector<Inode_block> sorted_inodes;
    for (int i = 0; i < INODE_NUM; i++) {
        if (super_block.inode[i].used_size & (1<<7)) { // Inode is in use
            int start = convertByteToDecimal(super_block.inode[i].start_block, BYTE_SIZE);
            sorted_inodes.push_back(Inode_block(i, start));
        }
    }
    // Sorted the inodes based on start block
    sort(sorted_inodes.begin(),sorted_inodes.end(), ascending_sb_order());
    // Start at position  == 1
    unsigned int position = 1;
    disk.open(m_disk_name);
    for (int unsigned i = 0; i < sorted_inodes.size(); i++) {
        int inode_index = sorted_inodes[i].index;
        int blocks_covered = convertByteToDecimal(super_block.inode[inode_index].used_size, BYTE_SIZE - 1);
        unsigned int start_blk = sorted_inodes[i].start_block;
        if (start_blk != position) {
            unsigned int new_start_block = position;
            char data[1024];
            for (int j = 0; j < blocks_covered; j++) {
                // Loop through each block
                // =====Copying blocks ======//
                disk.seekg(1024 * (start_blk + j), ios_base::beg);
                disk.read(data, 1024);
                disk.seekp(1024 * position, ios_base::beg);
                disk.write(data, 1024);



                // ==== Setting FBL value to 1 ========//
                uint8_t mask = 1<<7;
                int start_index = position / 8; // Which index to start on in the FBL
                int mask_offset = position - (start_index * 8);
                mask >>=mask_offset;
                super_block.free_block_list[start_index] |= mask; // Set the bit we are looking at to 1
                ++position;
            }
            // ===== Updating inode start position ====== //
            super_block.inode[inode_index].start_block = new_start_block;
        } else {
            position = position + blocks_covered;
        }
        
        // set bits to 0 after the last position
        int start_index = position / 8; // Which index to start on in the FBL
        int mask_offset = position - (start_index * 8);
        int count = 0;
        // Zero out all the blocks that we were at before
        for (unsigned int i=start_index; i < sizeof(super_block.free_block_list)/sizeof(super_block.free_block_list[0]); i++){
            uint8_t mask = 0x7F; 
            if ((int)i == start_index) {
                for (int i = 0; i <mask_offset; i++) {
                    mask = (mask >> 1)|128;
                }
            }
            while (mask) {
                count++;
                super_block.free_block_list[i] &= mask;
                mask = (mask >>1) | 128;
                
                if (count == blocks_covered - 1) {
                    break;
                }
            }
            if (count == blocks_covered - 1) {
                break;
            }
        }
    }
    // ====== ZERO OUT ALL DATA BLOCKS AFTER POSITION =========== //
    char deletion_buf[1024];
    memset(deletion_buf, 0 , 1024);
    disk.seekp(1024 * position, ios_base::beg); // Get to the current position
    for (int i = position; i < 128; i++) {
        disk.write(deletion_buf, 1024);
    }

    // ===== OverWrite FBL and INODES ======= //
    disk.seekg(0, ios_base::beg);
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
void fs_cd(char name[5]) {
    // Same as the terminal to change the current working directory
    // If the name doesnt exist just print out the error message
}
