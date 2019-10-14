#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "thread.h"
void test_run_thread(void *arg) {
  char *para = arg;
  for (;;) {
    console_printf(para);
  }
}

int main(void) {
  printf("\nkernel start done!\n");
  init_all();

  thread_start("test thread a", 18, test_run_thread, "argA ");
  thread_start("test thread b", 20, test_run_thread, "argB ");

  intr_enable();
  while (1) {
    console_printf("Main ");
  }
}
