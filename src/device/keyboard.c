//
// Created by wanyu on 2019/10/16.
//
#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "ioqueue.h"

#define esc '\033'
#define backspace '\b'
#define tab '\t'
#define  enter '\r'
#define delete '\177'

/* 以上不可见字符一律定义为0 */
#define char_invisible    0
#define ctrl_l_char    char_invisible
#define ctrl_r_char    char_invisible
#define shift_l_char    char_invisible
#define shift_r_char    char_invisible
#define alt_l_char    char_invisible
#define alt_r_char    char_invisible
#define caps_lock_char    char_invisible


/* 定义控制字符的通码和断码 */
#define shift_l_make    0x2a
#define shift_r_make    0x36
#define alt_l_make    0x38
#define alt_r_make    0xe038
#define alt_r_break    0xe0b8
#define ctrl_l_make    0x1d
#define ctrl_r_make    0xe01d
#define ctrl_r_break    0xe09d
#define caps_lock_make    0x3a
struct ioqueue keyboard_buf;//键盘环形缓冲区
static bool ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;


/* 以通码make_code为索引的二维数组 */
static char keymap[][2] = {
/* 扫描码   未与shift组合  与shift组合*/
/* ---------------------------------- */
/* 0x00 */    {0,    0},
/* 0x01 */
              {esc,            esc},
/* 0x02 */
              {'1',  '!'},
/* 0x03 */
              {'2',  '@'},
/* 0x04 */
              {'3',  '#'},
/* 0x05 */
              {'4',  '$'},
/* 0x06 */
              {'5',  '%'},
/* 0x07 */
              {'6',  '^'},
/* 0x08 */
              {'7',  '&'},
/* 0x09 */
              {'8',  '*'},
/* 0x0A */
              {'9',  '('},
/* 0x0B */
              {'0',  ')'},
/* 0x0C */
              {'-',  '_'},
/* 0x0D */
              {'=',  '+'},
/* 0x0E */
              {backspace,      backspace},
/* 0x0F */
              {tab,            tab},
/* 0x10 */
              {'q',  'Q'},
/* 0x11 */
              {'w',  'W'},
/* 0x12 */
              {'e',  'E'},
/* 0x13 */
              {'r',  'R'},
/* 0x14 */
              {'t',  'T'},
/* 0x15 */
              {'y',  'Y'},
/* 0x16 */
              {'u',  'U'},
/* 0x17 */
              {'i',  'I'},
/* 0x18 */
              {'o',  'O'},
/* 0x19 */
              {'p',  'P'},
/* 0x1A */
              {'[',  '{'},
/* 0x1B */
              {']',  '}'},
/* 0x1C */
              {enter,          enter},
/* 0x1D */
              {ctrl_l_char,    ctrl_l_char},
/* 0x1E */
              {'a',  'A'},
/* 0x1F */
              {'s',  'S'},
/* 0x20 */
              {'d',  'D'},
/* 0x21 */
              {'f',  'F'},
/* 0x22 */
              {'g',  'G'},
/* 0x23 */
              {'h',  'H'},
/* 0x24 */
              {'j',  'J'},
/* 0x25 */
              {'k',  'K'},
/* 0x26 */
              {'l',  'L'},
/* 0x27 */
              {';',  ':'},
/* 0x28 */
              {'\'', '"'},
/* 0x29 */
              {'`',  '~'},
/* 0x2A */
              {shift_l_char,   shift_l_char},
/* 0x2B */
              {'\\', '|'},
/* 0x2C */
              {'z',  'Z'},
/* 0x2D */
              {'x',  'X'},
/* 0x2E */
              {'c',  'C'},
/* 0x2F */
              {'v',  'V'},
/* 0x30 */
              {'b',  'B'},
/* 0x31 */
              {'n',  'N'},
/* 0x32 */
              {'m',  'M'},
/* 0x33 */
              {',',  '<'},
/* 0x34 */
              {'.',  '>'},
/* 0x35 */
              {'/',  '?'},
/* 0x36	*/
              {shift_r_char,   shift_r_char},
/* 0x37 */
              {'*',  '*'},
/* 0x38 */
              {alt_l_char,     alt_l_char},
/* 0x39 */
              {' ',  ' '},
/* 0x3A */
              {caps_lock_char, caps_lock_char}
/*其它按键暂不处理*/
};

#define KBD_BUF_PORT 0x60//键盘缓冲区寄存器端口号为0x60

static void intr_keyboard_handler(void) {
    bool ctrl_down_last = ctrl_status;
    bool shift_down_last = shift_status;
    bool caps_lock_last = caps_lock_status;
    bool break_code;
    printf("\nget_intr_status: ");
    printInt(get_intr_status());
    printf("\n");
    uint16_t scan_code = inb(KBD_BUF_PORT);//从输出缓冲区读取数据，刷新键盘中断处理

    if (scan_code == 0xe0) {
        //如果是扩展码
        ext_scancode = true;
        return;
    }
    if (ext_scancode) {
        //如果是扩展码，则将两次的扫描码合并
        scan_code = (0xe000 | scan_code);
        ext_scancode = false;
    }
    break_code = ((scan_code & 0x0080) != 0);//断码=通码+0x80
    if (break_code) {
        //如果是断码
        uint16_t make_code = (scan_code &= 0xff7f);//获取断开的扫描码对应的通码

        if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
            //断码是ctrl
            ctrl_status = false;
        } else if (make_code == shift_l_make || make_code == shift_r_make) {
            //断码是shift
            shift_status = false;
        } else if (make_code == alt_l_make || make_code == alt_r_make) {
            //断码是alt
            alt_status = false;
        }
        return;//断码不再处理
    } else if ((scan_code > 0x00 && scan_code < 0x3b) || (scan_code == alt_r_make) || (scan_code == ctrl_r_make)) {
        //如果不是断码，而是通码
        bool shift = false;//判断组合键
        if (scan_code < 0x0e || scan_code == 0x29 || \
        (scan_code == 0x1a) || (scan_code == 0x1b) || \
        (scan_code == 0x2b) || (scan_code == 0x27) || \
        (scan_code == 0x28) || (scan_code == 0x33) || \
        (scan_code == 0x34) || (scan_code == 0x35)) {
            if (shift_down_last) {
                shift = true;
            }
        } else {
            if (shift_down_last && caps_lock_last) {
                shift = false;
            } else if (shift_down_last || caps_lock_last) {
                shift = true;
            } else {
                shift = false;
            }
        }
        uint8_t index = (scan_code &= 0x00ff);
        char cur_char = keymap[index][shift];
        if (cur_char) {
            if ((ctrl_down_last && cur_char == 'l') || (ctrl_down_last && cur_char == 'u')) {
                cur_char -= 'a';
            }

            if (!ioq_full(&keyboard_buf)) {
                ioq_putchar(&keyboard_buf, cur_char);
            } else {
                printf("\nioqueue is full!\n");
            }
            return;
        }
        if (scan_code == ctrl_r_make || scan_code == ctrl_l_make) {
            ctrl_status = true;
        } else if (scan_code == shift_r_make || scan_code == shift_l_make) {
            shift_status = true;
        } else if (scan_code == caps_lock_make) {
            caps_lock_status = !caps_lock_last;
        } else if (scan_code == alt_r_make || scan_code == alt_l_make) {
            alt_status = true;
        }
    } else {
        printf("unknown key!\n");
    }
}

void keyboard_init() {
    printf("keyboard init start\n");
    register_handler(0x21, intr_keyboard_handler);
    ioqueue_init(&keyboard_buf);
    printf("keyboard init done\n");
}