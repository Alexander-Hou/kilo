#include "kilo.h"
struct termios orignal_termios; // 定义全局变量保存原始终端属性
int main() {
    enableRawMode(); // 进入原始模式
    while(1){
        char c = '\0';
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        if(iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if(c == 'q') break;
    }
    return 0;
}