//
// Created by wanyu on 2019/10/26.
//

#ifndef OS_INODE_H
#define OS_INODE_H

#include "stdint.h"
#include "list.h"

struct inode {
    uint32_t i_no;
    uint32_t size;
    bool write_flag;
    uint32_t i_sectors[13];
    struct list_elem inode_tag;
    uint32_t i_open_cnts;   // 记录此文件被打开的次数
};
#endif //OS_INODE_H
