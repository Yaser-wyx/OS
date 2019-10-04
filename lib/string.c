#include "string.h"
#include "debug.h"
#include "global.h"

/*将字符串从src复制到dist*/
char* strcpy(char* _dist_, const char* _src_) {
  ASSERT(_dist_ != NULL && _src_ != NULL);
  char* dist = _dist_;
  while ((*_dist_++ = *_src_++)) {
  }
  return dist;
}
/*返回字符串的长度*/
uint32_t strlen(const char* src) {
  ASSERT(src != NULL);
  char* index = src;
  while (*index++) {
  }
  return (index - src - 1);
}
/* 比较两个字符串,若_dist_中的字符大于_src_中的字符返回1,相等时返回0,否则返回-1.
 */
int8_t strcmp(const char* _dist_, const char* _src_) {
  ASSERT(_dist_ != NULL && _src_ != NULL);
  while (*_dist != 0 && *_dist_ == *_src_) {
    _dist_++;
    _src_++;
  }
  return *_dist_ > *_src_ ? 1 : *_dist_ == *_src_ ? 0 : -1;
}
/* 从左到右查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char* strchr_first(const char* _src_, const char ch) {
  ASSERT(_src_ != NULL);
  while (*_src_ != 0 && *_src_ != ch) {
    _src_++;
  }

  return *_src_ != ch ? NULL : (char*)_src_;
}
/* 从后往前查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char* strrchr(const char* _src_, const char ch) {
  ASSERT(_src_ != NULL);
  char* last_index_of_char = NULL;
  while (*_src_ != 0) {
    if (*_src_ == ch) {
      last_index_of_char = src;
    }
    _src_++;
  }
  return (char*)last_index_of_char;
}
/* 将字符串src_拼接到dst_后,将回拼接的串地址 */
char* strcat(char* _dist_, const char* _src_) {
  ASSERT(_dist_ != NULL && _src_ != NULL);
  char* dist = _dist_ while (*dist != 0) { dist++; }
  while (*_src_ != 0) {
    *dist++ = *_src_++;
  }
  return _dist_;
}
/* 在字符串str中查找指定字符ch出现的次数 */
uint32_t strchrs(const char* str, uint8_t ch) {
  ASSERT(str != NULL);
  uint32_t count = 0;
  while (*str != 0) {
    if (*str == ch) {
      count++;
    }
    str++;
  }
  return count;
}