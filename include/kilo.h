#ifndef KILO_H
#define KILO_H
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>
#include<sys/ioctl.h>
#include<sys/types.h>

#define CTRL_KEY(k) ((k) & 0x1f) // 将字符转换为控制键

typedef struct row_text{
    int size; // 行文本长度
    char *str; // 行文本内容
} row_text; // 定义结构体表示一行文本

struct editorConfig {
    struct termios orignal_termios;  // 保存原始终端属性
    int screenrows; // 屏幕行数
    int screencols; // 屏幕列数
    int cursor_x; // 光标x坐标
    int cursor_y; // 光标y坐标
    int numrows; // 文本行数
    row_text *row; // 文本行内容
};

extern struct editorConfig editor; // 声明全局变量保存编辑器配置

/*** terminal.c ***/
void enableRawMode(void);  // 使终端进入原始模式
void disableRawMode(void); // 恢复终端的规范模式
void die(const char *str); // 错误处理函数，输出错误信息并退出程序

/*** apbuf.c ***/
struct apbuf {
    char *str; // 缓冲区指针
    int len; // 缓冲区长度
}; // 定义动态字符串缓冲区结构体

void bufferAppend(struct apbuf *ab, const char *s, int len); // 向缓冲区追加字符串
void bufferFree(struct apbuf *ab); // 释放缓冲区内容

/*** editor.c ***/
typedef enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY,
    PAGE_UP,
    PAGE_DOWN,
} editorKey; // 定义枚举类型表示特殊键

int editorReadKey(void); // 读取一个键盘输入
void editorMoveCursor(int key); // 移动光标位置
void editorProcessKeypress(void); // 处理键盘输入
void editorInitConfig(void); // 初始化编辑器配置
int getWindowSize(int *rows, int *cols); // 获取窗口大小
void editorDrawRows(struct apbuf *ab); // 绘制屏幕内容
void editorRefreshScreen(void); // 刷新屏幕显示
void editorOpenFile(char *filename); // 打开文件并读取内容
void editorAppendRow(char *s, size_t len); // 向编辑器配置中追加一行文本内容


#endif