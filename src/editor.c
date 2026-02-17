#include "kilo.h"

struct editorConfig editor; // 定义全局变量保存编辑器配置

char editorReadKey(void) {
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if(nread == -1 && errno != EAGAIN) {
            die("read"); // 读取键盘输入
        }
    }
    return c;
}

void editorProcessKeypress(void) {
    char c = editorReadKey(); // 读取一个键盘输入
    switch(c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4); // 清屏
            write(STDOUT_FILENO, "\x1b[H", 3); // 将光标移动到左上角
            exit(0); // 按下Ctrl-Q退出程序
            break;
    }
}

void editorRefreshScreen(void) {
    struct apbuf ab = {NULL, 0}; // 定义一个动态字符串缓冲区
    bufferAppend(&ab, "\x1b[?25l", 6); // 隐藏光标
    bufferAppend(&ab, "\x1b[H", 3); // 将光标移动到左上角
    editorDrawRows(&ab); // 绘制屏幕内容
    bufferAppend(&ab, "\x1b[H", 3); // 将光标移动到左上角
    bufferAppend(&ab, "\x1b[?25h", 6); // 显示光标
    write(STDOUT_FILENO, ab.str, ab.len); // 将缓冲区内容写入标准输出
    bufferFree(&ab); // 释放缓冲区内容
}

void editorDrawRows(struct apbuf *ab) {
    int y;
    for(y = 0; y < editor.screenrows; y++) {
        if(y==editor.screenrows/3) {
            char welcome[81];   // 定义欢迎信息字符串
            int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor");  // 格式化欢迎信息字符串
            if(welcomelen > editor.screencols) welcomelen = editor.screencols;  // 如果欢迎信息长度超过屏幕宽度，则截断
            int padding = (editor.screencols - welcomelen) / 2;  // 计算欢迎信息的左右填充空格数
            if(padding > 0) {
                bufferAppend(ab, "~", 1);
                padding--;
            }  // 在欢迎信息前面显示一个波浪符，同时减少一个填充空格
            while(padding--) {
                bufferAppend(ab, " ", 1);
            }
            bufferAppend(ab, welcome, welcomelen);  // 显示欢迎信息
        } else {
            bufferAppend(ab, "~", 1); // 在每行显示一个波浪符
        }
        bufferAppend(ab, "\x1b[K", 3); // 清除行尾内容
        if(y<editor.screenrows-1) {
            bufferAppend(ab, "\r\n", 2); // 换行
        }
    }
}

int getWindowSize(int *rows, int *cols) {
    struct winsize window;  // 定义结构体保存窗口大小
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == -1 || window.ws_col == 0) {
        return -1; // 获取窗口大小失败
    } else {
        *cols = window.ws_col; // 设置列数
        *rows = window.ws_row; // 设置行数
        return 0; // 获取窗口大小成功
    }
}

void editorInitConfig(void) {
    if(getWindowSize(&editor.screenrows, &editor.screencols) == -1) {
        die("getWindowSize"); // 获取窗口大小失败
    }
}