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

/* 定义可读写的最大扇区数,调试用的 */
#define max_lba ((80*1024*1024/512) - 1)    // 只支持80MB硬盘

uint8_t channel_cnt;
struct ide_channel channels[2];//两个ide通道
void ide_init() {
    printk("ide init start!\n");
    uint8_t hd_cnt = *((uint8_t *) 0x475);
    ASSERT(hd_cnt > 0)
    channel_cnt = DIV_ROUND_UP(hd_cnt, 2);
    struct ide_channel *channel;
    uint8_t channel_index = 0;
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
        channel->expecting_intr = false;
        lock_init(&channel->lock);//初始化通道锁
        sema_init(&channel->disk_done, 0);
        channel_index++;
    }
    printk("ide init done!");
}