#include "stdio.h"
#include "stdio-kernel.h"
#include "global.h"
#include "console.h"


#define va_start(ap, v) ap = (va_list)&v  // 把ap指向第一个固定参数v
#define va_arg(ap, t) *((t*)(ap += 4))      // ap指向下一个参数并返回其值
#define va_end(ap) ap = NULL          // 清除ap

/* 格式化输出字符串format */
void printk(const char *format, ...) {
    va_list args;
    va_start(args, format);           // 使args指向format
    char buf[1024] = {0};           // 用于存储拼接后的字符串
    uint32_t strlen = vsprintf(buf, format, args);
    buf[strlen] = '\n';
    va_end(args);
    console_put_str(buf);
}
