#include "print.h"
void main()
{
    uint8_t *c = "kernel start ok!\n\0";
    printf(c);
    printInt(12345);
    while (1)
    {
    }
}