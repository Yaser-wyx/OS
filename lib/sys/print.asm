TI_GDT  equ 0
RPL0    equ 0
SELECTOR_VIDEO  equ (0x0003<<3) + TI_GDT + RPL0
ENTER   equ 0xd     
LINE_FEED    equ 0xa
BACKSPACE  equ 0x8
[bits 32]
section .text
;--------------------- put_char--------------------------
;功能：打印一个字符
;--------------------------------------------------------
global put_char
put_char:
    pushad                      ;保存所有的32位寄存器
    ;将视频段选择子送入gs寄存器
    mov ax,SELECTOR_VIDEO
    mov gs,ax

    ;读取光标的位置
    ;读取光标位置的高8位
    ;设置寄存器索引值
    mov dx,0x3D4                
    mov al,0x0e                 ;设置要读取的寄存器的索引值
    out dx,al
    ;从中读取数据
    mov dx,0x3D5
    in al,dx                    ;读取导数据，即光标地址的高8位
    mov ah,al

    ;读取光标位置的低8位
    ;设置寄存器索引值
    mov dx,0x3D4                
    mov al,0x0f                 ;设置要读取的寄存器的索引值
    out dx,al
    ;从中读取数据
    mov dx,0x3D5
    in al,dx                    ;读取导数据，即光标地址的高8位
    
    mov bx,ax                   ;将光标的位置存放到bx中

    ;处理传入的字符是什么字符
    mov ecx,[ebp + 36]          ;读取参数
    ;处理回车
    cmp cl,ENTER
    jz .is_enter
    ;处理换行
    cmp cl,LINE_FEED
    jz .is_line_feed
    ;处理退格
    cmp cl,BACKSPACE
    jz .is_backspace
    ;处理其他字符
    jmp .put_other

    .is_backspace:
        