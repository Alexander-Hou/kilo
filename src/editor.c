#include "kilo.h"

struct editorConfig editor; // 定义全局变量保存编辑器配置

int editorReadKey(void) {
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if(nread == -1 && errno != EAGAIN) {
            die("read"); // 读取键盘输入
        }
    }
    if(c== '\x1b') {
        char seq[3];
        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';  
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';  
        if(seq[0] == '[') {
            if(seq[1] >= '0' && seq[1] <= '9') {
                if(read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';  
                if(seq[2] == '~') {
                    switch(seq[1]) {
                        case '1': return HOME_KEY; 
                        case '3': return DEL_KEY; 
                        case '4': return END_KEY; 
                        case '5': return PAGE_UP; 
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch(seq[1]) {
                    case 'A': 
                        return ARROW_UP; // 上箭头键
                    case 'B': 
                        return ARROW_DOWN; // 下箭头键
                    case 'C': 
                        return ARROW_RIGHT; // 右箭头键
                    case 'D': 
                        return ARROW_LEFT; // 左箭头键
                    case 'H':
                        return HOME_KEY; // Home键
                    case 'F':
                        return END_KEY; // End键
                }
            }
        } else if(seq[0] == 'O') {
            switch(seq[1]) {
                case 'H':
                    return HOME_KEY; // Home键
                case 'F':
                    return END_KEY; // End键
            }
        }
    }
    return c; // 返回读取到的键盘输入
}

void editorProcessKeypress(void) {
    int c = editorReadKey(); // 读取一个键盘输入
    switch(c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4); // 清屏
            write(STDOUT_FILENO, "\x1b[H", 3); // 将光标移动到左上角
            exit(0); // 按下Ctrl-Q退出程序
            break;
        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editorMoveCursor(c); // 按下箭头键移动光标
            break;
        case PAGE_DOWN:
        case PAGE_UP:
            {
                int direction = (c == PAGE_UP) ? ARROW_UP : ARROW_DOWN; // 确定翻页方向
                int times = editor.screenrows;
                while(times--) {
                    editorMoveCursor(direction); // 按下Page Up或Page Down键翻页
                }
            }
            break;
        case HOME_KEY:
            editor.cursor_x = 0; // 按下Home键将光标移动到行首
            break;
        case END_KEY:
            editor.cursor_x = editor.screencols - 1; // 按下End键将光标移动到行尾
            break;
        }
}

void editorRefreshScreen(void) {
    struct apbuf ab = {NULL, 0}; // 定义一个动态字符串缓冲区
    bufferAppend(&ab, "\x1b[?25l", 6); // 隐藏光标
    bufferAppend(&ab, "\x1b[H", 3); // 将光标移动到左上角
    editorDrawRows(&ab); // 绘制屏幕内容
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.cursor_y + 1, editor.cursor_x + 1); // 将光标移动到指定位置
    bufferAppend(&ab, buf, strlen(buf)); // 将光标位置字符串追加到动态字符串缓冲区
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

void editorMoveCursor(int key) {
    switch(key) {
        case ARROW_UP:
            if(editor.cursor_y != 0) {
                editor.cursor_y--; 
            }
            break;
        case ARROW_DOWN:
            if(editor.cursor_y != editor.screenrows - 1) {
                editor.cursor_y++; 
            }
            break;
        case ARROW_LEFT:
            if(editor.cursor_x != 0) {
                editor.cursor_x--; 
            }
            break;
        case ARROW_RIGHT:
            if(editor.cursor_x != editor.screencols - 1) {
                editor.cursor_x++; 
            }
            break;
    }
}