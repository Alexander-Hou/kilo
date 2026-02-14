#include "kilo.h"
void enableRawMode(void) {
    if(tcgetattr(STDIN_FILENO, &orignal_termios) == -1) {
        die("tcgetattr"); // 获取当前终端属性
    }
    atexit(disableRawMode); // 注册退出函数，确保程序退出时恢复终端属性

    struct termios rawmode=orignal_termios; // 复制原始终端属性
    rawmode.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // 关闭输入处理功能
    rawmode.c_oflag &= ~(OPOST); // 关闭输出处理功能
    rawmode.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // 关闭回显、规范模式、中断信号和扩展功能
    rawmode.c_cflag |= (CS8); // 设置字符大小为8位
    rawmode.c_cc[VMIN] = 0; // 最小输入字符数为0
    rawmode.c_cc[VTIME] = 1; // 读取超时时间为0.1秒

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawmode) == -1) {
        die("tcsetattr"); // 设置新的终端属性
    }
}

void disableRawMode(void){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orignal_termios) == -1) {
        die("tcsetattr"); // 恢复原始终端属性
    }
}

void die(const char *str){
    perror(str); // 输出错误信息
    exit(1); // 退出程序
}