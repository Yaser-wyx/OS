#include <stdio.h>
void main()
{
    int a = 10, b = 20, out;
    asm("movb %b0,%1;"
        :
        : "a"(a), "m"(b));
    printf("%d", b);
}