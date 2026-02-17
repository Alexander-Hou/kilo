#include "kilo.h"
struct termios orignal_termios; // 定义全局变量保存原始终端属性
int main() {
    enableRawMode(); // 进入原始模式
    editorInitConfig(); // 初始化编辑器配置
    while(1){
        editorRefreshScreen(); // 刷新屏幕显示
        editorProcessKeypress(); // 处理键盘输入
    }
    disableRawMode(); // 退出原始模式
    return 0;
}