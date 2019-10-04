#include "bitmap.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "stdint.h"
#include "string.h"
// 初始化位图
void bitmap_init(struct bitmap* btmp) {
  memset(btmp->bits, 0, btmp->btmp_bytes_len);
}
// 判断bit_idx位是否为1，如果为1，则返回true，否则返回false
bool bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx) {
  uint32_t byte_idx = bit_idx / 8;   //计算是第几个byte
  uint32_t bit_index = bit_idx % 8;  //计算是该byte的第几位
  return (btmp->bits[byte_idx]) & (BITMAP_MASK << bit_index);
}

/* 在位图中申请连续cnt个位,成功则返回其起始位下标，失败返回-1 */
int bitmap_scan(struct bitmap* btmp, uint32_t cnt) {
  bool find = false;
  int start_index = -1;
  int now_index = 0;
  //计算需要几个字节，以及余下要几位
  uint32_t byte_cnt = cnt / 8;  //需要的字节数
  uint8_t bit_cnt = cnt % 8;    //需要的位数
  while (!find && ((btmp->btmp_bytes_len) - now_index) * 8 >= cnt) {
    //如果没有找到，就一直找，直到剩余的位数不够要分配的空间
    //先查找byte
    bool find_byte = false;
    while (!find_byte && (btmp->btmp_bytes_len - now_index) * 8 >= cnt) {
      while ((btmp->bits[now_index] | 0x00)) {
        //不断向后找，直到找到一块全空的
        now_index++;
      }
      start_index = now_index;
      find_byte = true;  //假设找到了
      for (int i = now_index + 1; i < now_index + byte_cnt; i++) {
        if (btmp->bits[i] | 0x00) {
          //当前块不全空
          find_byte = false;
          now_index = i + 1;  //判断下一个
          start_index = -1;
          break;
        }
      }
    }
    // byte寻找完成
    if (find_byte) {  //如果找到了足够的字节数
      //寻找bit
      now_index += (byte_cnt - 1);
      find = true;
      int base_index = now_index * 8;
      for (int i = 0; i < bit_cnt; i++) {
        if (bitmap_scan_test(btmp, base_index + i)) {
          //当前位不为空
          find = false;
          now_index++;
          start_index = -1;
          break;
        }
      }
    }
  }
  return start_index;
}
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);
