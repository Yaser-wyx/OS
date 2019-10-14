#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "global.h"
#include "interrupt.h"

static struct lock console_lock;

//初始化终端
void console_init() { lock_init(&console_lock, 1); }

//获取终端
void get_console() { lock_acquire(&console_lock); }

//释放终端
void release_console() { lock_release(&console_lock); }

//使用终端输出字符串
void console_printf(char *str) {
    get_console();
    printf(str);
    release_console();
}

//使用终端输出字符串
void console_printInt(unsigned long num) {
    get_console();
    printInt(num);
    release_console();
}

void console_putchar(uint8_t c){
    get_console();
    put_char(c);
    release_console();
}