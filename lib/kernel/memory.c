#include "memory.h"
#include "debug.h"

//复制指定字符串到内存dist
void memset(void *_dist_, uint32_t size, uint32_t value)
{
    ASSERT(_dist_ != NULL);
    uint8_t *dist = (uint8_t *)_dist_;
    while (size--)
    {
        *dist = value;
        dist++;
    }
}
//将指定个数的字符串复制到指定位置
void memcpy(void *_dist_, const void *_src_, uint32_t size)
{
    ASSERT(_dist_ != NULL && _src_ != NULL);

    uint8_t *dist = (uint8_t *)_dist_;
    uint8_t *src = (uint8_t *)_src_;
    while (size--)
    {
        *dist++ = *src++;
    }
}

//比较目标地址与原始地址的内存是否一致,相同返回0，dist>src则返回1，否则返回-1
int memcmp(const void *_dist_, const void *_src_, uint32_t size)
{
    ASSERT(_dist_ != NULL && _src_ != NULL);
    uint8_t *dist = (uint8_t *)_dist_;
    uint8_t *src = (uint8_t *)_src_;
    while (size--)
    {
        if (*dist != *src)
        {
            return *dist > *src ? 1 : -1;
        }
        dist++;
        src++;
    }
    return 0;
}
