#ifndef KILO_H
#define KILO_H
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<errno.h>

extern struct termios orignal_termios;

void enableRawMode(void);  // 使终端进入原始模式
void disableRawMode(void); // 恢复终端的规范模式
void die(const char *str); // 错误处理函数，输出错误信息并退出程序

#endif