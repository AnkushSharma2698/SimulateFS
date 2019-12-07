# File System 

## By: Ankush Sharma

### Design & Implementation
    - Global Variables
        - Super_block super_block: Held globally in order to manipulate the currently mounted disk and keep track of its state.
        - char buffer: This is the buffer used by the FileSystem to read and write exactly 1kb to and from files.
        - fstream disk: global handler to open/close/read/write from a disk.
        - string m_disk_name: global name for the currently mounted disk.
        - map <int set<int>>: Keeps track of every directory by inode index in the filesystem and the set contains every file or directory inode index contained in that directory.
        - int cwd: Holds the inode idx of the current working directory
    - Main Method:
        - Opted to use a switch statement to make error handling easy as well as flexibility to handle the multiple cases. 
    - fs_mount():
        - First read the entire Superblock into memory to begin consistency checks
        - Check 1: Iterated the inodes to determine blocks which are stored in a std::map<int, int> (block_number, times_used) in use and then iterated the Free Block List to compare against the map for consistency.
        - Check 2: Iterated inodes and added directories to global directory map. Following this, iterated the inodes again and checked for duplicates in the inodes set.
        - Check 3: Iterated inodes and performed required checks.
        - Check 4: SAME as 3
        - Check 5: SAME as 3
        - Check 6: SAME as 3
        - If all checks passed, set disk name and current working directory.
    - fs_create():
        -First checked if given name is correct size, then check if the size is correct, then check if the given name already exists
        - If checks satisfy, if directory can directly create, otherwise must find if there are N available block and N+1 blocks are asked for, create should fail
        

