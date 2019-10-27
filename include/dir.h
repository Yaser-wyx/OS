#ifndef OS_DIR_H
#define OS_DIR_H

#include "stdint.h"
#include "inode.h"
#include "ide.h"
#include "global.h"
#include "fs.h"
#define MAX_FILE_NAME_LEN 16 //最大文件名长度
//目录结构
struct dir {
    struct inode *inode;
    uint32_t dir_pos;//目录指针
    uint8_t dir_buf[512];//目录缓存
};

//目录项结构
struct dir_entry {
    char filename[MAX_FILE_NAME_LEN];   //文件名
    uint32_t i_node;                    //inode的编号
    enum file_types f_type;	      // 文件类型

};
#endif //OS_DIR_H
