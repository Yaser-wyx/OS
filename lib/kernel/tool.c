#include "stdint.h"

uint32_t strlen(uint8_t *str)
{
    uint32_t len = 0;
    while (*(str + len) != '\0')
    {
        len++;
    }
    return len;
}