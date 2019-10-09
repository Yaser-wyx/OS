#include "print.h"

void printf(uint8_t *str) {
  //先实现打印字符串的版本
  while (*str != '\0') {
    put_char(*str);
    str++;
  }
}

void printInt(unsigned long number) {
  uint8_t *str;
  int len = 0;
  while (number > 0) {
    *(str + len) = (number % 10) + 48;
    number = number / 10;
    len++;
  }
  *(str + len) = '\0';  //末尾填零
  int start = 0;
  int end = len - 1;
  while (start < end) {
    char tmp = *(str + start);
    *(str + start) = *(str + end);
    *(str + end) = tmp;
    start++;
    end--;
  }
  printf(str);
}