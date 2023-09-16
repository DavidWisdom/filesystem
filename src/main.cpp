#include "simdisk.hpp"
#include <termios.h>
#include <unistd.h>
using namespace std;
#define UP 65
#define DOWN 66
#define LEFT 68
#define RIGHT 67
int getch() {
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
int main() {
    init_disk();
    std::string command;
    std::vector<std::string> history;
    int curr = 0;
    while (true) {
        command.clear();
        cout << GREEN << "root@LAPTOP" << RESET << ":";
        size_t sz = 12;
        cout << BLUE;
        sz += show_path();
        std::cout << RESET;
        std::cout << "$ ";
        sz += 2;
        // getline(cin, command);
        char ch;
        while (true) {
            ch = getch();
            // 检测方向键输入
            if (ch == 27) {
                ch = getch();
                if (ch == 91) {
                    ch = getch();
                    switch(ch) {
                        case UP: {
                            if (!history.empty()) {
                                if ((int)history.size() > curr) ++curr;
                                std::string lastCommand = ((int)history.size() >= curr && curr > 0) ? history[history.size() - curr] : "";
                                std::cout << "\r" << std::string(64, ' ') << "\r";
                                command = lastCommand;
                                std::cout << GREEN << "root@LAPTOP" << RESET << ":";
                                std::cout << BLUE;
                                show_path();
                                std::cout << RESET;
                                std::cout << "$ ";
                                std::cout << command << std::flush;
                            }
                        } break;
                        case DOWN: {
                            if (!history.empty()) {
                                if (curr > 1) --curr;
                                std::string lastCommand = ((int)history.size() >= curr && curr > 0) ? history[history.size() - curr] : "";
                                std::cout << "\r" << std::string(64, ' ') << "\r";
                                command = lastCommand;
                                std::cout << GREEN << "root@LAPTOP" << RESET << ":";
                                std::cout << BLUE;
                                show_path();
                                std::cout << RESET;
                                std::cout << "$ ";
                                std::cout << command << std::flush;
                            }
                        } break;
                    }
                }
            } else if (ch == 127) { // 检测删除键的 ASCII 值
                if (!command.empty()) {
                    std::cout << "\b \b"; // 删除光标前一个字符
                    command.pop_back();
                }
            } else if (ch == 10) {
                curr = 0;
                std::cout << std::endl;
                break;
            } else {
                command += ch;
                std::cout << ch << std::flush;
            }
        }
        if (!command.empty()) history.push_back(command);
        std::vector<std::string> args = split_command(command);
        if (args.size() < 3) args.push_back("");
        if (args[0] == "simdisk") {
            if (args[1] == "info") info();
            else if (args[1] == "su") su(args[2]);
            else if (args[1] == "echo") echo(args[2]);
            else if (args[1] == "help") help();
            else if (args[1] == "cd") {
                if (args.size() == 3) {
                    cd(args[2]);
                } else {
                    std::cout << "cd: too many arguments" << std::endl;
                }
            } else if (args[1] == "dir") {
                if (args.size() == 3) {
                    dir(args[2]);
                } else {
                    std::cout << "dir: too many arguments" << std::endl;
                }
            } else if (args[1] == "md") {
                for (int i = 2; i < (int)args.size(); i++) md(args[i]);
            } else if (args[1] == "rd") {
                for (int i = 2; i < (int)args.size(); i++) rd(args[i]);
            } else if (args[1] == "newfile") {
                for (int i = 2; i < (int)args.size(); i++) newfile(args[i]);
            } else if (args[1] == "cat") {
                if (args.size() == 3) {
                    cat(args[2]);
                } else {
                    std::cout << "cat: too many arguments" << std::endl;
                }
            } else if (args[1] == "copy") copy(args[2]);
            else if (args[1] == "del") {
                for (int i = 2; i < (int)args.size(); i++) del(args[i]);
            }
            else if (args[1] == "check") check();
            // else if (args[1] == "clear") clear();
            else if (args[1] == "exit") return 0; 
            else if (args[1].empty()) continue;
            else cout << args[1] << ": command not found" << endl;
        } else {
            if (args[0].empty()) continue;
            else {
                cout << args[0] << ": command not found" << endl;
            }
        }
    }
}