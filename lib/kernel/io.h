#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"
static inline void outb(uint16_t port, uint8_t data)
{
    __asm__ volatile("outb %b0,%w1" ::"a"(data), "Nd"(port));
}
static inline void outsw(uint16_t port, const void *addr, uint32_t cnt)
{
    __asm__ volatile("cld; rep outsw"
                     : "+S"(addr), "+c"(cnt)
                     : "d"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile("inb %w1,%b0"
                     : "=a"(data)
                     : "Nd"(port));
    return data;
}
static inline void insw(uint16_t port, const void *addr, uint32_t cnt)
{
    __asm__ volatile("cld; rep insw"
                     : "+D"(addr), "+c"(cnt)
                     : "d"(port)
                     : "memory");
}
// //将一字节数据写入指定端口port中
// static inline void outb(uint16_t port, uint8_t data)
// {
//     __asm__ volatile("outb %b0,%w1" ::"a"(data), "Nd"(port));
// }
// //从addr位置指定n个字节数，将这些数据写入指定端口port中
// static inline void outsw(uint16_t port, const void *addr, uint32_t cnt)
// {
//     __asm__ volatile("cld; rep outsw"
//                  : "+S"(addr), "+c"(cnt)
//                  : "d"(port));
// }
// static inline uint8_t inb(uint16_t port)
// {
//     uint8_t out_data;
//     __asm__ volatile("inb %w1,%b0"
//                  : "=a"(out_data)
//                  : "Nd"(port));
//     return out_data;
// }

// static inline void insw(uint16_t port, const void *addr, uint32_t cnt)
// {
//     __asm__ volatile("cld;rep insw"
//                  : "+D"(addr), "+c"(cnt)
//                  : "d"(port)
//                  : "memory");
// }
#endif