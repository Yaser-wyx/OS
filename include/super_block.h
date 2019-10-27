#ifndef OS_SUPER_BLOCK_H
#define OS_SUPER_BLOCK_H

#include "stdint.h"

struct super_block {
    uint32_t magic;             //魔数，用于标记文件系统类型
    uint32_t sec_cnt;           //当前分区的扇区个数
    uint32_t inode_cnt;         //当前分区中inode的数量
    uint32_t part_lba_base;     //分区起始的lba地址

    uint32_t block_bitmap_lba;  //空闲块的位图地址
    uint32_t block_bitmap_sects;//空闲块位图所占的扇区数

    uint32_t inode_table_lba;   //inode数组的地址
    uint32_t inode_table_sects; //inode数组所占用的扇区数

    uint32_t inode_bitmap_lba;        // i结点位图起始扇区lba地址
    uint32_t inode_bitmap_sects;        // i结点位图占用的扇区数量

    uint32_t data_start_lba;    //空闲块开始的扇区号
    uint32_t root_inode_no;     //根目录所在的inode节点编号
    uint32_t dir_entry_size;    //根目录的大小

    uint8_t pad[460];           //凑够512字节
}__attribute__((packed));
#endif //OS_SUPER_BLOCK_H
