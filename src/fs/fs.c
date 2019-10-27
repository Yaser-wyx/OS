//
// Created by wanyu on 2019/10/26.
//

#include "fs.h"
#include "super_block.h"
#include "dir.h"
#include "inode.h"
#include "stdint.h"
#include "stdio-kernel.h"
#include "list.h"
#include "string.h"
#include "ide.h"
#include "global.h"
#include "debug.h"
#include "memory.h"

#define SUPER_BLOCK_USEFUL_SIZE sizeof(struct super_block) - 460
struct partition *cur_part;            //当前操作的分区

//在分区链表中找到指定名字的分区进行装载，赋值给cur_part，同时从硬盘中将该分区的元信息读入
static bool mount_partition(struct list_elem *elem, int arg) {
    char *part_name = (char *) arg;
    struct partition *partition = elem2entry(struct partition, part_tag, elem);//获取分区表
    if (strcmp(part_name, partition->name) == 0) {
        //找到了指定的分区
        //将分区进行挂载操作
        struct disk *hd = partition->my_disk;
        cur_part = partition;
        struct super_block *superBlock = (struct super_block *) sys_malloc(SECTOR_SIZE);//分配一扇区的内存空间作为读取超级块的缓存
        ide_read(hd, partition->start_lba + 1, superBlock, 1);//超级块读取
        cur_part->sb = (struct super_block *) sys_malloc(SUPER_BLOCK_USEFUL_SIZE);
        if (cur_part->sb == NULL) {
            PANIC("alloc memory failed!");
        }
        //将超级块中有用的数据复制
        memcpy(cur_part->sb, superBlock, SUPER_BLOCK_USEFUL_SIZE);
        //加载各种位图数据信息
        //1.读取空闲块位图信息
        cur_part->block_bitmap.bits = (uint8_t *) sys_malloc(superBlock->block_bitmap_sects * SECTOR_SIZE);
        if (cur_part->block_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->block_bitmap.btmp_bytes_len = superBlock->block_bitmap_sects * SECTOR_SIZE;
        ide_read(hd, superBlock->block_bitmap_lba, cur_part->block_bitmap.bits,
                 superBlock->block_bitmap_sects);
        //2.读取inode位图信息
        cur_part->inode_bitmap.bits = (uint8_t *) sys_malloc(superBlock->inode_bitmap_sects * SECTOR_SIZE);
        if (cur_part->inode_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->inode_bitmap.btmp_bytes_len = superBlock->inode_bitmap_sects * SECTOR_SIZE;
        ide_read(hd, superBlock->inode_bitmap_lba, cur_part->inode_bitmap.bits, superBlock->inode_bitmap_sects);

        list_init(&cur_part->open_inodes);
        printk("mount %s done!\n", partition->name);
        printk("%s's start_lba is 0x%x, has %x nums of sects!\n",partition->name,partition->start_lba,partition->sec_cnt);
        return true;
    }
    return false;
}

//格式化分区，创建文件系统
static void partition_format(struct partition *part) {
    uint32_t boot_sector_secs = 1;
    uint32_t super_block_secs = 1;

    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILE_PER_PART, BITS_PER_SECTOR);//计算inode位图占用扇区数，bitmap中一位代表一个文件
    uint32_t inode_table_sects = DIV_ROUND_UP(sizeof(struct inode) * MAX_FILE_PER_PART, SECTOR_SIZE);//inode数组所占的扇区数
    uint32_t used_sects = inode_bitmap_sects + inode_table_sects + super_block_secs + boot_sector_secs;//已经占用的扇区个数
    uint32_t free_sects = part->sec_cnt - used_sects;//计算剩余的空闲扇区
    //计算空闲块的位图空间
    uint32_t block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
    uint32_t block_bitmap_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_len, BITS_PER_SECTOR);

    //初始化超级块
    struct super_block superBlock;
    superBlock.magic = FS_TYPE_Y;
    superBlock.sec_cnt = part->sec_cnt;
    superBlock.inode_cnt = MAX_FILE_PER_PART;
    superBlock.part_lba_base = part->start_lba;

    //设置空闲块
    superBlock.block_bitmap_sects = block_bitmap_sects;
    superBlock.block_bitmap_lba = part->start_lba + 2;

    //设置inode位图
    superBlock.inode_bitmap_sects = inode_bitmap_sects;
    superBlock.inode_bitmap_lba = superBlock.block_bitmap_lba + superBlock.block_bitmap_sects;

    //设置inode数组
    superBlock.inode_table_sects = inode_table_sects;
    superBlock.inode_table_lba = superBlock.inode_bitmap_lba + superBlock.inode_bitmap_sects;

    //设置数据块
    superBlock.data_start_lba = superBlock.inode_table_lba + superBlock.inode_table_sects;

    superBlock.root_inode_no = 0;//第一个inode给根目录使用
    superBlock.dir_entry_size = sizeof(struct dir_entry);
    printk("%s info:\n", part->name);
    printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   data_start_lba:0x%x\n",
           superBlock.magic, superBlock.part_lba_base, superBlock.sec_cnt, superBlock.inode_cnt,
           superBlock.block_bitmap_lba, superBlock.block_bitmap_sects, superBlock.inode_bitmap_lba,
           superBlock.inode_bitmap_sects, superBlock.inode_table_lba, superBlock.inode_table_sects,
           superBlock.data_start_lba);

    struct disk *hd = part->my_disk;

    //将超级块写入1号扇区中
    ide_write(hd, part->start_lba + 1, &superBlock, 1);
    printk("   super_block_lba:0x%x\n", part->start_lba + 1);

    uint32_t buf_size = block_bitmap_sects > inode_bitmap_sects ? block_bitmap_sects : inode_bitmap_sects;
    buf_size = buf_size > inode_table_sects ? buf_size : inode_table_sects;

    uint8_t *buf = sys_malloc(buf_size * SECTOR_SIZE);//分配内存

    //初始化空闲块位图，并写入指定位置
    buf[0] = 1;//预留给根目录
    uint32_t block_bitmap_last_byte = block_bitmap_sects / 8;
    uint32_t block_bitmap_last_bit = block_bitmap_sects % 8;
    uint32_t overflow_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);//超出当前分区扇区容量的大小

    memset(&buf[block_bitmap_last_byte], 0xff, overflow_size);//从最后一个含有有效bit的字节开始，将后面的全部置1，表示无效
    //将最后一字节中有效的bit重置为0
    uint32_t mask = 0xff << block_bitmap_last_bit;
    buf[block_bitmap_last_byte] &= mask;

    ide_write(hd, superBlock.block_bitmap_lba, buf, superBlock.block_bitmap_sects);

    //初始化inode位图，并写入指定位置
    memset(buf, 0, buf_size);//清空缓存
    buf[0] = 1;
    ide_write(hd, superBlock.inode_bitmap_lba, buf, superBlock.inode_table_sects);

    //初始化inode数组，并写入内存
    memset(buf, 0, buf_size);//清空缓存
    struct inode *inode = (struct inode *) buf;
    inode->i_no = 0;
    inode->i_sectors[0] = superBlock.data_start_lba;
    inode->size = superBlock.dir_entry_size * 2;//两个目录，当前目录以及上级目录
    ide_write(hd, superBlock.inode_table_lba, buf, superBlock.inode_table_sects);

    //初始化根目录，并写入数据区
    memset(buf, 0, buf_size);//清空缓存
    struct dir_entry *p_de = (struct dir_entry *) buf;

    memcpy(p_de->filename, ".", 1);
    p_de->i_node = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    memcpy(p_de->filename, "..", 2);
    p_de->i_node = 0;
    p_de->f_type = FT_DIRECTORY;

    ide_write(hd, superBlock.data_start_lba, buf, 1);

    printk("   root_dir_lba:0x%x\n", superBlock.data_start_lba);
    printk("%s format done\n", part->name);
    sys_free(buf);
}

//从硬盘上读取超级块，如果没有，则进行分区格式化
void fs_init() {

    uint8_t channel_no = 0, dev_no, part_idx = 0;
    struct super_block *superBlock = (struct super_block *) sys_malloc(SECTOR_SIZE);
    if (superBlock == NULL) {
        PANIC("alloc memory failed!");
    }
    printk("searching filesystem!\n");
    while (channel_no < channel_cnt) {//检索每一个通道
        dev_no = 0;//对于每一个通道，都有两块硬盘
        while (dev_no < 2) {
            if (dev_no == 0) {
                dev_no++;
                continue;
            }
            struct disk *hd = &channels[channel_no].devices[dev_no];//获取当前硬盘信息
            struct partition *partition = hd->prim_parts;
            //遍历当前硬盘上所有的分区表信息
            while (part_idx < 12) {
                if (part_idx == 4) {
                    partition = hd->logic_parts;
                }
                if (partition->sec_cnt != 0) {
                    memset(superBlock, 0, SECTOR_SIZE);
                    ide_read(hd, partition->start_lba + 1, superBlock, 1);//读取超级块信息
                    if (superBlock->magic != FS_TYPE_Y) {
                        //如果当前文件系统无法识别
                        printk("formatting %s`s partition %s......\n", hd->name, partition->name);
                        partition_format(partition);//格式化当前分区，创建文件系统
                    } else {
                        printk("%s has filesystem\n", partition->name);
                    }
                }
                part_idx++;
                partition++;
            }
            dev_no++;
        }
        channel_no++;
    }
    sys_free(superBlock);

    char default_name[8] = "sdb1\0";
    list_traversal(&partition_list, mount_partition, (int) default_name);
}