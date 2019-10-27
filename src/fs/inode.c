//
// Created by wanyu on 2019/10/27.
//
#include "inode.h"
#include "fs.h"
//#include "file.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "interrupt.h"
#include "list.h"
#include "stdio-kernel.h"
#include "string.h"
#include "super_block.h"

struct inode_position {
    bool tow_sec;//当前inode是否跨扇区
    uint32_t sec_lba;//扇区地址
    uint32_t off_size;//扇区内偏移量
};

//获取inode所在的扇区以及扇区内偏移量
static void inode_locate(struct partition *partition, uint32_t inode_no, struct inode_position *inodePosition) {
    uint32_t inode_size = sizeof(struct inode);
    uint32_t offset_byte = inode_no * inode_size;//计算在inode_table中的偏移字节数
    inodePosition->off_size = offset_byte % SECTOR_SIZE;//计算扇区中的偏移量
    inodePosition->sec_lba = partition->sb->inode_table_lba + offset_byte / SECTOR_SIZE;
    inodePosition->tow_sec = (SECTOR_SIZE - inodePosition->off_size) < inode_size;
}

//将inode写入到指定分区中
void inode_sync(struct partition *partition, struct inode *inode, void *io_buf) {
    struct inode_position inodePosition;
    inode_locate(partition, inode->i_no, &inodePosition);//获取该inode在分区中的信息
    struct inode pure_inode;
    memcpy(&pure_inode, inode, sizeof(struct inode));//复制一份inode数据
    //清除在内存中使用的记录
    list_remove(&pure_inode.inode_tag);
    pure_inode.write_flag = false;
    pure_inode.i_open_cnts = 0;
    //写回到inode_table中
    uint32_t read_cnt = inodePosition.tow_sec ? 2 : 1;

    ide_read(partition->my_disk, inodePosition.sec_lba, io_buf, read_cnt);

    memcpy(io_buf + inodePosition.off_size, &pure_inode, sizeof(struct inode));

    ide_write(partition->my_disk, inodePosition.sec_lba, io_buf, read_cnt);


}