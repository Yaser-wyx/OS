#include "debug.h"
#include "init.h"
#include "memory.h"
#include "print.h"
void main(void) {
  printf("\nkernel start done!\n");
  init_all();
  // __asm__ volatile("sti"); //打开所有中断，将IF位置1
  void *addr = get_kernel_pages(3);
  printf("kernel pages addr:");
  printInt((unsigned long)addr);
  printf("\n");
  int len = sizeof(long);
  printf("the length of long:");
  printInt(len);
  while (1) {
  }
}