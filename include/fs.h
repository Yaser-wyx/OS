#ifndef OS_FS_H
#define OS_FS_H

#include "stdint.h"
#include "ide.h"

#define MAX_FILE_PER_PART 4096  //分区最大文件数
#define BITS_PER_SECTOR 4096    //一个扇区的位数
#define SECTOR_SIZE 512         //扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE  //块字节大小

enum file_types {
    FT_UNKNOWN,	  // 不支持的文件类型
    FT_REGULAR,	  // 普通文件
    FT_DIRECTORY	  // 目录
};

void fs_init();
#endif //OS_FS_H
