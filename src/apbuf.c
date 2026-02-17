#include "kilo.h"

void bufferAppend(struct apbuf *append_buffer, const char *str, int len) {
    char *new = realloc(append_buffer->str, append_buffer->len + len); // 重新分配内存，增加新的字符串长度
    if(new == NULL) return; // 如果内存分配失败，直接返回
    memcpy(&new[append_buffer->len], str, len); // 将新的字符串内容复制到缓冲区末尾
    append_buffer->str = new; // 更新缓冲区指针
    append_buffer->len += len; // 更新缓冲区长度
}

void bufferFree(struct apbuf *ab) {
    free(ab->str); // 释放缓冲区内容
    ab->str = NULL; // 将缓冲区指针置空
    ab->len = 0; // 将缓冲区长度置零
}