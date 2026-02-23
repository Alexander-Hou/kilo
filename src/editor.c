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

void editorMoveCursor(int key) {
    row_text *row = (editor.cursor_y >= editor.numrows) ? NULL : &editor.row[editor.cursor_y]; // 获取当前行文本内容
    switch(key) {
        case ARROW_UP:
            if(editor.cursor_y != 0) {
                editor.cursor_y--; 
            }
            break;
        case ARROW_DOWN:
            if(editor.cursor_y < editor.numrows - 1) {
                editor.cursor_y++; 
            }
            break;
        case ARROW_LEFT:
            if(editor.cursor_x != 0) {
                editor.cursor_x--; 
            }
            else if(editor.cursor_x == 0 && editor.cursor_y > 0) {
                editor.cursor_y--; 
                editor.cursor_x = editor.row[editor.cursor_y].size; 
            }
            break;
        case ARROW_RIGHT:
            if(row && editor.cursor_x < row->size) {
                editor.cursor_x++; 
            }
            else if(row && editor.cursor_x == row->size) {
                editor.cursor_y++; 
                editor.cursor_x = 0; 
            }
            break;
    }
    row = (editor.cursor_y >= editor.numrows) ? NULL : &editor.row[editor.cursor_y]; // 获取当前行文本内容
    int rowlen = row ? row->size : 0; // 获取当前行文本长度
    if(editor.cursor_x > rowlen) {
        editor.cursor_x = rowlen; // 如果光标x坐标超过当前行文本长度，则将光标x坐标调整到当前行文本末尾
    }
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

void editorInitConfig(void) {
    editor.row = NULL; // 初始化行文本内容指针
    editor.numrows = 0;  // 初始化文本行数
    editor.cursor_x = 0; // 初始化光标x坐标
    editor.cursor_y = 0; // 初始化光标y坐标
    editor.row_offset = 0; // 初始化行偏移量
    editor.cols_offset = 0; // 初始化列偏移量
    if(getWindowSize(&editor.screenrows, &editor.screencols) == -1) {
        die("getWindowSize"); // 获取窗口大小失败
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

void editorDrawRows(struct apbuf *ab) {
    int y;
    for(y = 0; y < editor.screenrows; y++) {
        int filerow = y + editor.row_offset; // 计算文件行索引
        if(filerow >= editor.numrows) {
            if(editor.numrows == 0 && y == editor.screenrows/3) {
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
        } else {
            int len = editor.row[filerow].size - editor.cols_offset; // 获取行文本长度
            if(len < 0) len = 0; // 如果行文本长度小于0，则设置为0
            if(len > editor.screencols) len = editor.screencols; // 如果行文本长度超过屏幕宽度，则截断
            bufferAppend(ab, editor.row[filerow].str + editor.cols_offset, len); // 显示行文本内容
        }
        bufferAppend(ab, "\x1b[K", 3); // 清除行尾内容
        if(y<editor.screenrows-1) {
            bufferAppend(ab, "\r\n", 2); // 换行
        }
    }
}

void editorRefreshScreen(void) {
    editorScorllScreen(); // 根据光标位置调整行偏移量
    struct apbuf ab = {NULL, 0}; // 定义一个动态字符串缓冲区
    bufferAppend(&ab, "\x1b[?25l", 6); // 隐藏光标
    bufferAppend(&ab, "\x1b[H", 3); // 将光标移动到左上角
    editorDrawRows(&ab); // 绘制屏幕内容
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (editor.cursor_y - editor.row_offset) + 1, (editor.cursor_x - editor.cols_offset) + 1); // 将光标移动到指定位置
    bufferAppend(&ab, buf, strlen(buf)); // 将光标位置字符串追加到动态字符串缓冲区
    bufferAppend(&ab, "\x1b[?25h", 6); // 显示光标
    write(STDOUT_FILENO, ab.str, ab.len); // 将缓冲区内容写入标准输出
    bufferFree(&ab); // 释放缓冲区内容
}

void editorAppendRow(char *s, size_t len) {
    editor.row = realloc(editor.row, sizeof(row_text) * (editor.numrows + 1)); // 为行文本内容重新分配内存
    if(editor.row == NULL) {
        die("realloc"); // 内存分配失败
    }

    int row_index = editor.numrows; // 获取当前行索引
    editor.row[row_index].size = len; // 设置行文本长度
    editor.row[row_index].str = malloc(len + 1); // 为行文本内容分配内存
    if(editor.row[row_index].str == NULL) {
        die("malloc"); // 内存分配失败
    }

    memcpy(editor.row[row_index].str, s, len); // 将行文本内容复制到编辑器配置的行文本内容中
    editor.row[row_index].str[len] = '\0'; // 在行文本内容末尾添加字符串结束符

    editor.row[row_index].rsize = 0; // 初始化行渲染文本长度
    editor.row[row_index].render = NULL; // 初始化行渲染文本内容指针
    editorUpdateRow(&editor.row[row_index]); // 更新行渲染文本内容

    editor.numrows++; // 增加文本行数
}

void editorOpenFile(char *filename) {
    FILE *fp= fopen(filename, "r"); // 打开文件
    if(fp == NULL) {
        die("fopen"); // 打开文件失败
    }
    char line[1024]; // 定义一行文本内容
    while(fgets(line, sizeof(line), fp) != NULL) {
        int linelen = strcspn(line, "\r\n"); // 获取行文本长度，去除行末的换行符
        editorAppendRow(line, linelen); // 向编辑器配置中追加一行文本内容
    }
    fclose(fp); // 关闭文件
}

void editorScorllScreen(void) {
    if(editor.cursor_y < editor.row_offset) {
        editor.row_offset = editor.cursor_y; // 如果光标在当前行偏移量上方，则将行偏移量调整到光标所在行
    }
    if(editor.cursor_y >= editor.row_offset + editor.screenrows) {
        editor.row_offset = editor.cursor_y - editor.screenrows + 1; // 如果光标在当前行偏移量下方，则将行偏移量调整到光标所在行的上方
    }
    if(editor.cursor_x < editor.cols_offset) {
        editor.cols_offset = editor.cursor_x; // 如果光标在当前列偏移量左侧，则将列偏移量调整到光标所在列
    }
    if(editor.cursor_x >= editor.cols_offset + editor.screencols) {
        editor.cols_offset = editor.cursor_x - editor.screencols + 1; // 如果光标在当前列偏移量右侧，则将列偏移量调整到光标所在列的左侧
    }
}

void editorUpdateRow(row_text *row) {
    int tabs = 0; // 定义变量统计行文本中的制表符数量
    int i;
    for(i = 0; i < row->size; i++) {
        if(row->str[i] == '\t') {
            tabs++; // 如果行文本中有制表符，则增加制表符数量
        }
    }
    free(row->render); // 释放行渲染文本内容内存
    row->rsize = row->size + tabs * (TAB_STOP - 1); // 计算行渲染文本长度，制表符占用TAB_STOP个字符位置
    row->render = malloc(row->rsize + 1); // 为行渲染文本内容分配内存
    if(row->render == NULL) {
        die("malloc"); // 内存分配失败
    }
    int idx = 0; // 定义变量表示行渲染文本的索引位置
    for(i = 0; i < row->size; i++) {
        if(row->str[i] == '\t') {
            row->render[idx++] = ' '; // 将制表符替换为一个空格
            while(idx % TAB_STOP != 0) {
                row->render[idx++] = ' '; // 在制表符位置添加空格，直到达到下一个制表符停止位置
            }
        } else {
            row->render[idx++] = row->str[i]; // 将行文本中的非制表符字符复制到行渲染文本中
        }
    }
    row->render[idx] = '\0'; // 在行渲染文本末尾添加字符串结束符
}