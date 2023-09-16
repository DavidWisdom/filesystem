#ifndef SIMDISK_HPP
#define SIMDISK_HPP
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <sys/stat.h>
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long long;
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[1;34m"
#define WHITE   "\033[1;37m"
struct User {
    std::string name;
    std::string pwd;
};
struct Superblock {
    i32 block_size;
    i32 block_num;
    i32 inode_num;
    constexpr Superblock() : block_size(1024), block_num(100 * 1024), inode_num(100 * 1024) {}   
};
struct Block {
    char data[1024];
}blocks[100 * 1024];
struct Inode {
    char type;                    // 文件类型
    u32 size;                     // 文件大小
    mode_t mode;                  // 文件权限
    uid_t uid;                    // 用户 ID
    gid_t gid;                    // 组 ID
    time_t atime;                 // 访问时间
    time_t mtime;                 // 修改时间
    time_t ctime;                 // 创建时间
    u32 num_blocks;               // 文件块数量
    u32 direct_ptrs[8];           // 直接指针
    std::string author;
};
struct DirectoryEntry {
    Inode* inode = nullptr;
    std::string name;
    DirectoryEntry* parent = nullptr;
    std::map<std::string, DirectoryEntry*> files;
};
constexpr Superblock superblock;
std::bitset<superblock.block_num> block_bitmap;
std::bitset<superblock.inode_num> inode_bitmap;
User current_user;
DirectoryEntry* last_directory;
DirectoryEntry* current_directory;
DirectoryEntry* root_directory;
void init_disk() {
    last_directory = nullptr;
    current_user.name = "root";
    current_user.pwd = "root";
    current_directory = new DirectoryEntry();
    // TODO: inode
    current_directory->name = "~";
    root_directory = current_directory;
}
void release_disk() {

}
void new_file(Inode* inode) {
    
}
void delete_file(Inode* inode) {

}
const u32 INVALID_BLOCK_INDEX = -1;
u32 new_block() {
    for (u32 i = 0; i < block_bitmap.size(); ++i) {
        if (!block_bitmap.test(i)) {
            block_bitmap.set(i);
            // 在这里可以对新的数据块进行进一步处理或者存储
            // ...
            return i; // 返回新数据块的索引
        }
    }
    // 如果执行到这里，表示没有可用的数据块了
    // 可以抛出错误或执行相应的处理
    return INVALID_BLOCK_INDEX; // 返回一个无效的索引（根据需要进行定义）
}
void delete_block(u32 index) {
    if (index >= block_bitmap.size()) {
        // 检查索引是否越界
        // 可以抛出错误或执行相应的处理
        return;
    }
    if (!block_bitmap.test(index)) {
        // 检查数据块是否已被删除
        // 可以抛出错误或执行相应的处理
        return;
    }
    block_bitmap.reset(index); // 将相应的位设置为0，表示数据块被删除
    // 在这里可以对被删除的数据块进行进一步处理，如清零、释放等
    // ...
}
size_t show_path() {
    DirectoryEntry* temp = current_directory;
    std::vector<std::string> path;
    while (temp != nullptr) {
        path.push_back(temp->name);
        temp = temp->parent;
    }
    std::cout << "/";
    size_t sz = 1;
    int i = (int)path.size() - 2;
    for (; i >= 1; --i) {
        std::cout << path[i] << "/";
        sz += path[i].size() + 1;
    }
    if (i >= 0) {
        std::cout << path[i];
        sz += path[i].size();
    }
    return sz;
}
void info() {
    std::cout << std::right;

    std::cout << std::setw(10) << "Filesystem";
    std::cout << std::setw(10) << "Size";
    std::cout << std::setw(10) << "Used";
    std::cout << std::setw(10) << "Avail";
    std::cout << std::setw(10) << "Use%";
    std::cout << std::setw(10) << "Mounted\n";

    std::cout << std::setw(10) << "simdisk";
    std::cout << std::setw(10) << "100M";
    std::cout << std::setw(10) << "0";
    std::cout << std::setw(10) << "100M";
    std::cout << std::setw(10) << "0%";
    std::cout << std::setw(10) << "/\n";
}
void help() {
    std::cout << "These shell commands are defined internally.  Type 'help' to see this list." << std::endl;
    std::cout << "Type 'help name' to find out more about the function 'name'." << std::endl;
}
std::vector<std::string> split_path(std::string path) {
    std::vector<std::string> res;
    std::string curr_path;
    for (int i = 0; i < (int)path.size(); ++i) {
        if (path[i] != '/') {
            curr_path.push_back(path[i]);
        } else {
            res.push_back(curr_path);
            curr_path.clear();
        }
    }
    if (!curr_path.empty()) res.push_back(curr_path);
    return res;
}
std::vector<std::string> split_command(std::string command) {
    std::vector<std::string> res;
    std::string command_token;
    for (int i = 0; i < (int)command.size(); ++i) {
        if (command[i] != ' ') {
            command_token.push_back(command[i]);
        } else {
            if (!command_token.empty()) {
                res.push_back(command_token);
                command_token.clear();
            }
        }
    }
    if (!command_token.empty()) res.push_back(command_token);
    return res;
}
DirectoryEntry* get_path(std::string path) {
    if (path[0] == '/') {
        DirectoryEntry* absolute = root_directory;
        std::vector<std::string> splitpath = split_path(path);
        for (int i = 1; i < (int)splitpath.size(); i++) {
            if (absolute->files.find(splitpath[i]) == absolute->files.end()) {
                return nullptr;
            }
            absolute = absolute->files[splitpath[i]];
        }
        return absolute;
    } else {
        DirectoryEntry* relative = current_directory;
        std::vector<std::string> splitpath = split_path(path);
        for (int i = 0; i < (int)splitpath.size(); ++i) {
            if (splitpath[i] == ".") continue;
            else if (splitpath[i] == "..") {
                if (relative == root_directory) {
                    return nullptr;
                } else {
                    relative = relative->parent;
                }
            }
            else if (relative->files.find(splitpath[i]) == relative->files.end()) {
                return nullptr;
            }
            else if (relative->files.find(splitpath[i]) != relative->files.end()) {
                relative = relative->files[splitpath[i]];
            }
        }
        return relative;
    }
    return nullptr;
}
void cd(std::string path = "") {
    if (path.empty()) path = "/";
    if (path == "-") {
        if (last_directory == nullptr) {
            std::cout << "cd: OLDPWD not set" << std::endl;
            return;
        }
        DirectoryEntry* temp_directory = current_directory;
        current_directory = last_directory;
        last_directory = temp_directory;
        show_path();
        std::cout << std::endl;
        return;
    }
    DirectoryEntry* cd_directory = get_path(path);
    if (cd_directory) {
        last_directory = current_directory;
        current_directory = cd_directory;
    } else {
        std::cout << "cd: " << path << ": No such file or directory" << std::endl;
    }
}
void dir(std::string arg) {
    if (arg.empty()) {
        for (auto it = current_directory->files.begin(); it != current_directory->files.end(); ++it) {
            if (it->second->inode->type == 'd') {
                std::cout << BLUE;
            } 
            else {
                std::cout << WHITE;
            }
            std::cout << it->first << "    ";
            std::cout << RESET;
        }
        if (!current_directory->files.empty()) std::cout << std::endl;
        return;
    }
    /* TODO: 指定路径的dir */
}
void md(std::string name = "") {
    if (name.empty()) {
        std::cout << "md: missing operand" << std::endl;
        return;
    } 
    if (current_directory->files.find(name) != current_directory->files.end()) {
        std::cout << "md: cannot create directory '" << name << "': File exists" << std::endl;
        return;
    }
    /* TODO: 给定路径的md */
    Inode* inode = new Inode();
    inode->type = 'd';
    inode->size = 0;
    inode->mode = S_IRUSR | S_IWUSR | S_IXUSR;
    inode->author = current_user.name;
    DirectoryEntry* dentry = new DirectoryEntry();
    dentry->inode = inode;
    dentry->name = name;
    dentry->parent = current_directory;
    current_directory->files[name] = dentry;
}
void delete_directory(DirectoryEntry* dentry) {
    for (auto it = dentry->files.begin(); it != dentry->files.end(); ++it) {
        if (it->second->inode->type == 'f') {
            delete it->second;
            it->second = nullptr;
        }    
    }
    for (auto it = dentry->files.begin(); it != dentry->files.end(); ++it) {
        if (it->second) {
            delete_directory(it->second);
        }
    }
    dentry->files.clear();
    delete dentry;
}
void rd(std::string name) {
    if (name.empty()) {
        std::cout << "rd: missing operand" << std::endl;
        return;
    }
    if (current_directory->files.find(name) == current_directory->files.end()) {
        std::cout << "rd: failed to remove '" << name << "': No such file or directory" << std::endl;
        return;
    }
    delete_directory(current_directory->files[name]);
    current_directory->files.erase(name);
}
void newfile(std::string name = "") {
    if (name.empty()) {
        std::cout << "newfile: missing operand" << std::endl;
        return;
    } 
    if (current_directory->files.find(name) != current_directory->files.end()) {
        std::cout << "newfile: cannot create file '" << name << "': File exists" << std::endl;
        return;
    }
    /* TODO: 给定路径的newfile */
    Inode* inode = new Inode();
    inode->type = 'f';
    inode->size = 0;
    inode->mode = S_IRUSR | S_IWUSR | S_IXUSR;
    inode->author = current_user.name;
    DirectoryEntry* dentry = new DirectoryEntry();
    dentry->inode = inode;
    dentry->name = name;
    dentry->parent = current_directory;
    current_directory->files[name] = dentry;
}
void cat(std::string name) {
    if (name.empty()) {
        std::cout << "cat: missing operand" << std::endl;
        return;
    }
}
void copy(std::string arg) {

}
void del(std::string name = "") {
    if (name.empty()) {
        std::cout << "del: missing operand" << std::endl;
        return;
    }
    if (current_directory->files.find(name) == current_directory->files.end()) {
        std::cout << "del: cannot delete '" << name << "': No such file or directory" << std::endl;
        return;
    }
    delete current_directory->files[name];
    current_directory->files.erase(name);
}
void check() {

}
void exit() {}
void su(std::string username) {

}
void echo(std::string name) {
    std::cout << name << std::endl;
}
#endif 