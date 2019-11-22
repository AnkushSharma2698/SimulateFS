// Needed inlcusions
#include <iostream>
#include <sys/stat.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

// Headers to be included
#include "FileSystem.h"

using namespace std;

Super_block sb; 

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
    fstream disk;
    // Check if a disk with the given name exists in the cwd
    disk.open(new_disk_name);
    if (!disk) {
        cerr << "Error: Cannot find disk " << new_disk_name << endl;
        return;
    } 
    cout << "Found disk: will begin mounting" << endl;   
    disk.close();
    // Load the superblock of the file system <-- Read super block into mem
    // Check file system Consistency: --> DO NOT MOUNT, if Consistency check fails
    // 1.Free blocks in free space lists must not be allocated to anythimg
    // 1. Blocks in use --> allocated to EXACTLY 1 file
    // 2. name of every file/directory must be unique within a directory
    // 3. Free inode must have all bits = 0, else the "name" at least must have one non-zero bit
    // 4. The startblock of every inode that is a file must be between 1-127 inclusive
    // 5. size and start block of inode that is directory must be 0
    // 6. Parent can never be 126. If parent inode between 1-125 inclusive, parent inode must be in use and marked as a directory

    // If pass, unmount the other ffd
    // If all is well set cwd to root of new file disk
    // NOTE: DO NOT FLUSH BUFFER WHEN MOUNTING A FS
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
