//
// Created by wanyu on 2019/10/24.
//

#include "ide.h"
#include "sync.h"
#include "stdio.h"
#include "stdio-kernel.h"
#include "interrupt.h"
#include "memory.h"
#include "debug.h"
#include "string.h"
#include "io.h"
#include "list.h"
#include "timer.h"

/* 定义硬盘各寄存器的端口号 */
#define reg_data(channel)     (channel->port_base + 0)
#define reg_error(channel)     (channel->port_base + 1)
#define reg_sect_cnt(channel)     (channel->port_base + 2)
#define reg_lba_l(channel)     (channel->port_base + 3)
#define reg_lba_m(channel)     (channel->port_base + 4)
#define reg_lba_h(channel)     (channel->port_base + 5)
#define reg_dev(channel)     (channel->port_base + 6)
#define reg_status(channel)     (channel->port_base + 7)
#define reg_cmd(channel)     (reg_status(channel))
#define reg_alt_status(channel)  (channel->port_base + 0x206)
#define reg_ctl(channel)     reg_alt_status(channel)

/* reg_alt_status寄存器的一些关键位 */
#define BIT_STAT_BSY     0x80          // 硬盘忙
#define BIT_STAT_DRDY     0x40          // 驱动器准备好
#define BIT_STAT_DRQ     0x8          // 数据传输准备好了

/* device寄存器的一些关键位 */
#define BIT_DEV_MBS    0xa0        // 第7位和第5位固定为1
#define BIT_DEV_LBA    0x40
#define BIT_DEV_DEV    0x10

/* 一些硬盘操作的指令 */
#define CMD_IDENTIFY       0xec        // identify指令
#define CMD_READ_SECTOR       0x20     // 读扇区指令
#define CMD_WRITE_SECTOR   0x30        // 写扇区指令

int32_t ext_lba_base = 0;              //总扩展分区的起始lba

uint8_t p_no = 0, l_no = 0;//记录主硬盘以及逻辑硬盘的下标

struct list partition_list;//分区队列
//分区表项
struct partition_table_entry {
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_chs;
    uint8_t fs_type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_chs;

    uint32_t start_lba;//分区偏移的lba数
    uint32_t sec_cnt;

}__attribute__((packed));


//MBR引导扇区数据结构
struct boot_sector {
    uint8_t other[446];//引导代码
    struct partition_table_entry partitionTableEntry[4];//分区表项
    uint16_t signature;//魔数
}__attribute__((packed));



/*硬盘主从盘*/
#define HD_PRIMARY 0
#define HD_SLAVE 1
/* 定义可读写的最大扇区数,调试用的 */
#define max_lba ((80*1024*1024/512) - 1)    // 只支持80MB硬盘

enum disk_opt {
    DISK_READ,
    DISK_WRITE
};
uint8_t channel_cnt;
struct ide_channel channels[2];//两个ide通道


//选择硬盘：主盘还是从盘
static void select_dev(struct disk *hd) {
    ASSERT(hd != NULL)
    uint8_t reg_value = BIT_DEV_MBS | BIT_DEV_LBA;//设置通用的值

    if (hd->dev_no == HD_SLAVE) {
        reg_value |= BIT_DEV_DEV;
    }
    outb(reg_dev(hd->my_channel), reg_value);
}

//选择要读写的扇区地址以及扇区数
static void select_sector(struct disk *hd, uint32_t lba, uint8_t sec_cnt) {
    ASSERT(lba < max_lba)
    struct ide_channel *channel = hd->my_channel;
    //写入要读写的扇区数
    outb(reg_sect_cnt(channel), sec_cnt);
    //写入要读写的扇区地址
    outb(reg_lba_l(channel), (uint8_t) lba);//低地址
    outb(reg_lba_m(channel), (uint8_t) (lba >> 8));//中间地址
    outb(reg_lba_h(channel), (uint8_t) (lba >> 16));//高地址
    uint8_t reg_value = BIT_DEV_MBS | BIT_DEV_LBA;//设置通用的值

    if (hd->dev_no == HD_SLAVE) {
        reg_value |= BIT_DEV_DEV;
    }
    reg_value |= ((uint8_t) (lba >> 24) & 0x0f);
    outb(reg_dev(channel), reg_value);
}

//向通道发送命令cmd
static void cmd_out(struct ide_channel *channel, uint8_t cmd) {
    //只要向硬盘发送了命令，便将此标记为true，中断处理程序通过该标记来判断
    channel->expecting_intr = true;
    outb(reg_cmd(channel), cmd);
}

//从硬盘中中读取指定扇区数的数据到缓冲区
static void read_from_sector(struct disk *hd, void *buf, uint8_t sec_cnt) {
    uint32_t byte_size;
    if (sec_cnt == 0) {
        byte_size = 256 * 512;
    } else {
        byte_size = sec_cnt * 512;
    }
    insw(reg_data(hd->my_channel), buf, byte_size / 2);
}

//将指定扇区个数的数据写入到硬盘中
static void write2sector(struct disk *hd, void *buf, uint32_t sec_cnt) {
    uint32_t byte_size;
    if (sec_cnt == 0) {
        byte_size = 256 * 512;
    } else {
        byte_size = sec_cnt * 512;
    }
    outsw(reg_data(hd->my_channel), buf, byte_size / 2);
}

//等待硬盘最多30秒
static bool busy_wait(struct disk *hd) {
    struct ide_channel *channel = hd->my_channel;
    uint32_t wait_time = 30 * 1000;
    while (wait_time -= 10) {
        uint8_t status = inb(reg_status(channel));
        if (status & BIT_STAT_BSY) {
            //如果忙，则睡眠10毫秒
            mil_sleep(10);
        } else {
            return status & BIT_STAT_DRQ;
        }
    }
    return false;//失败
}

void disk_opt(enum disk_opt opt, struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    ASSERT(lba <= max_lba)
    ASSERT(sec_cnt > 0)
    lock_acquire(&hd->my_channel->lock);
    select_dev(hd);//选择磁盘
    uint32_t sec_done = 0;
    uint32_t sec_opt = 0;
    while (sec_done < sec_cnt) {
        if (sec_done + 256 <= sec_cnt) {
            sec_opt = 256;
        } else {
            sec_opt = sec_cnt - sec_done;
        }
        select_sector(hd, lba + sec_done, sec_opt);
        cmd_out(hd->my_channel, opt == DISK_READ ? CMD_READ_SECTOR : CMD_WRITE_SECTOR);
        if (opt == DISK_READ) {
            sema_down(&hd->my_channel->disk_done);
        }
        if (!busy_wait(hd)) {
            //读取失败
            char error[64];
            sprintf(error, "%s read/write sector %d failed!!!!!!\n", hd->name, lba);
            PANIC(error);
        }
        if (opt == DISK_READ) {
            //将数据从硬盘缓冲区读出
            read_from_sector(hd, (void *) ((uint32_t) buf + sec_done * 512), sec_opt);
        } else {
            write2sector(hd, (void *) ((uint32_t) buf + sec_done * 512), sec_opt);
        }
        if (opt == DISK_WRITE) {
            sema_down(&hd->my_channel->disk_done);
        }
        sec_done += sec_opt;
    }
    lock_release(&hd->my_channel->lock);

}

//从硬盘中读取sec_cnt个扇区到buf中
void ide_read(struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    disk_opt(DISK_READ, hd, lba, buf, sec_cnt);
}

void ide_write(struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    disk_opt(DISK_WRITE, hd, lba, buf, sec_cnt);
}

//硬盘中断处理程序
void intr_hd_handler(uint8_t irq_no) {
    ASSERT(irq_no == 0x2e || irq_no == 0x2f)
    uint8_t channel_no = irq_no - 0x2e;
    struct ide_channel *channel = &channels[channel_no];
    if (channel->expecting_intr) {
        channel->expecting_intr = false;
        sema_up(&channel->disk_done);
        inb(reg_status(channel));//告知硬盘本次中断处理完成
    }
}

static void swap_pairs_bytes(const char *dst, char *buf, uint32_t len) {
    uint32_t index;
    for (index = 0; index < len; index += 2) {
        buf[index] = dst[index + 1];
        buf[index + 1] = dst[index];
    }
    buf[index] = '\0';
}

//硬盘分区表扫描
static void partition_scan(struct disk *hd, uint32_t ext_lba) {
    struct boot_sector *bootSector = sys_malloc(sizeof(struct boot_sector));//分配空间用来保存硬盘的分区表信息
    ide_read(hd, ext_lba, bootSector, 1);//读取硬盘的引导扇区
    uint8_t part_index = 0;//分区表索引
    struct partition_table_entry *partitionTableEntry = bootSector->partitionTableEntry;//读取引导扇区分区表

    while (part_index < 4) {
        if (partitionTableEntry[part_index].fs_type == 0x5) {//当前为扩展分区
            if (ext_lba_base == 0) {//第一次进入总扩展分区
                ext_lba_base = partitionTableEntry[part_index].start_lba;//总扩展分区的偏移lba地址
                partition_scan(hd, ext_lba_base);//扫描总扩展分区信息
            } else {
                partition_scan(hd, partitionTableEntry[part_index].start_lba + ext_lba_base);
            }
        } else if (partitionTableEntry[part_index].fs_type != 0) {
            if (ext_lba == 0) {
                //主分区
                hd->prim_parts[p_no].start_lba = ext_lba + partitionTableEntry[part_index].start_lba;//分区表起始地址
                hd->prim_parts[p_no].my_disk = hd;
                hd->prim_parts[p_no].sec_cnt = partitionTableEntry[part_index].sec_cnt;
                list_append(&partition_list, &hd->prim_parts[p_no].part_tag);
                sprintf(hd->prim_parts[p_no].name, "%s%d", hd->name, p_no + 1);
                p_no++;
                ASSERT(p_no < 4)

            } else {
                //逻辑分区
                hd->logic_parts[l_no].start_lba = ext_lba + partitionTableEntry[part_index].start_lba;//分区表起始地址
                hd->logic_parts[l_no].my_disk = hd;
                hd->logic_parts[l_no].sec_cnt = partitionTableEntry[part_index].sec_cnt;
                list_append(&partition_list, &hd->logic_parts[l_no].part_tag);
                sprintf(hd->logic_parts[l_no].name, "%s%d", hd->name, l_no + 5);
                l_no++;
                if (l_no >= 8) {
                    return;
                }
            }
        }
        part_index++;
    }
    sys_free(bootSector);
}

static void identify_disk(struct disk *hd) {
    char id_info[512];
    select_dev(hd);
    cmd_out(hd->my_channel, CMD_IDENTIFY);
/* 向硬盘发送指令后便通过信号量阻塞自己,
 * 待硬盘处理完成后,通过中断处理程序将自己唤醒 */
    sema_down(&hd->my_channel->disk_done);

/* 醒来后开始执行下面代码*/
    if (!busy_wait(hd)) {     //  若失败
        char error[64];
        sprintf(error, "%s identify failed!!!!!!\n", hd->name);
        PANIC(error);
    }
    //读取硬盘信息
    read_from_sector(hd, id_info, 1);

    char buf[64];
    uint8_t sn_start = 10 * 2, sn_len = 20, md_start = 27 * 2, md_len = 40;
    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);
    printk("   disk %s info:\n      SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    swap_pairs_bytes(&id_info[md_start], buf, md_len);
    printk("      MODULE: %s\n", buf);
    uint32_t sectors = *(uint32_t *) &id_info[60 * 2];
    printk("      SECTORS: %d\n", sectors);
    printk("      CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}

/* 打印分区信息 */
static bool partition_info(struct list_elem *pelem, int arg UNUSED) {
    struct partition *part = elem2entry(struct partition, part_tag, pelem);
    printk("   %s start_lba:0x%x, sec_cnt:0x%x\n", part->name, part->start_lba, part->sec_cnt);

/* 在此处return false与函数本身功能无关,
 * 只是为了让主调函数list_traversal继续向下遍历元素 */
    return false;
}

void ide_init() {
    printk("ide init start!\n");
    uint8_t hd_cnt = *((uint8_t *) 0x475);
    ASSERT(hd_cnt > 0)
    channel_cnt = DIV_ROUND_UP(hd_cnt, 2);
    struct ide_channel *channel;
    uint8_t channel_index = 0;
    uint32_t dev_no = 0;
    //初始化通道
    while (channel_index < channel_cnt) {
        channel = &channels[channel_index];
        sprintf(channel->name, "ide%d", channel_index);
        switch (channel_index) {
            case 0:
                channel->port_base = 0x1f0;
                channel->irq_no = 0x20 + 14;//中断号
                break;
            case 1:
                channel->port_base = 0x170;
                channel->irq_no = 0x20 + 15;
                break;
        }
        register_handler(channel->irq_no, intr_hd_handler);
        channel->expecting_intr = false;
        lock_init(&channel->lock);//初始化通道锁
        sema_init(&channel->disk_done, 0);//初始化信号量
        while (dev_no < 2) {
            struct disk *hd = &channel->devices[dev_no];
            hd->my_channel = channel;
            hd->dev_no = dev_no;
            sprintf(hd->name, "sd%c", 'a' + channel_index * 2 + dev_no);
            identify_disk(hd);
            if (dev_no != 0) {
                //不扫描内核所在的硬盘
                partition_scan(hd, 0);//扫描硬盘
            }
            dev_no++;
        }

        dev_no = 0;
        channel_index++;
    }
    printk("\n   all partition info\n");
    list_traversal(&partition_list, partition_info, (int) NULL);
    printk("ide init done!");
}