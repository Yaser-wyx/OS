TI_GDT  equ 0
RPL0    equ 0
SELECTOR_VIDEO  equ (0x0003<<3) + TI_GDT + RPL0
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
    mov ecx,[esp + 36]          ;读取参数
    ;处理回车
    cmp cl,0xd
    jz .is_enter
    ;处理换行
    cmp cl,0xa
    jz .is_line_feed
    ;处理退格
    cmp cl,0x8
    jz .is_backspace
    ;处理其他字符
    jmp .put_other

    .is_backspace:                      ;删除一个字符
        dec bx                          
        shl bx,1                        ;光标位置乘以二就是当前字符在显存中的位置
        mov word [gs:bx],0x0720         ;在删除字符的位置添加一处空白
        shr bx,1                        ;复原光标值
        jmp .set_cursor
    
    .put_other:                         ;打印字符
        shl bx,1
        mov al,cl
        mov ah,0x07
        mov [gs:bx],ax
        shr bx,1
        inc bx
        cmp bx,2000
        jl .set_cursor
        jmp .roll_screen
    
    ;换行与回车，统一处理
    .is_line_feed:
    .is_enter:
        xor dx,dx                       ;清空高8位
        mov ax,bx                       ;设置被除数
        mov cx,80                       ;设置除数
        div cx                          ;计算当前是第几行
        sub bx,dx
        add bx,80                       ;移动到下一行
        cmp bx,2000
        jl .set_cursor                  ;如果没有满一屏的数据，则对光标位置进行设置

    .roll_screen:                       ;滚动屏幕
        ;将第1~24行的数据，搬运至0~23行
        cld
        mov ecx,960                     ;一次搬运4个字节，1920个字符为3840字节，所以要搬运960次
        mov esi,0xc00b_80a0             ;第一行行首
        mov edi,0xc00b_8000             ;第零行行首
        rep movsd                       ;重复搬运
        ;将第24行数据全部填充为空字符
        mov ecx,80
        mov bx,3840
        .clear_last_line:
            mov word [gs:bx],0x0720
            add bx,2
        loop .clear_last_line
        mov bx,1920                     ;将光标放置在最后一行的起始位置

    .set_cursor:                        ;设置光标位置
        ;设置光标高八位
        mov dx,0x3d4
        mov al,0x0e
        out dx,al
        mov dx,0x3d5
        mov al,bh
        out dx,al
        ;设置光标低八位
        mov dx,0x3d4
        mov al,0x0f
        out dx,al
        mov dx,0x3d5
        mov al,bl
        out dx,al
    popad
    ret
       