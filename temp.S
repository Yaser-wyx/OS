%include "boot.inc"
SECTION loader vstart=LOADER_BASE_ADDR
;栈指针
LOADER_STACK_TOP    equ  LOADER_BASE_ADDR
jmp loader_start                                                ;跳转至代码执行

GDT_BASE:       dd      0x00000000,0x00000000                   ;第0个描述符无效，不使用，置空
CODE_DESC:      dd      0x0000FFFF,DESC_CODE_HIGHT_4_BYTE
DATA_DESC:      dd      0x0000FFFF,DESC_DATA_HIGHT_4_BYTE
VEDIO_DESC:     dd      0x80000007,DESC_VIDEO_HIGHT_4_BYTE
    
GDT_SIZE       equ     $ - GDT_BASE                            ;GDT大小
GDT_LIMIT      equ     GDT_SIZE - 1                            ;GDT界限，即GDT大小减一
times   60  dq  0                                               ;预留60个描述符的位置
;GDT选择子
SELECTOR_CODE   equ     (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA   equ     (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO  equ     (0x0003<<3) + TI_GDT + RPL0
;总的内存大小
total_mem_bytes dd      0
;GDT指针 
gdt_ptr         dw      GDT_LIMIT
                dd      GDT_BASE

ards_buf    times       244     db      0
ards_num     dw          0               ;记录ARDS结构体的数量

    loader_start:
        ;设置栈地址
        mov sp,LOADER_STACK_TOP
        ;三种内存大小获取方法
        ;1.使用eax功能号为0xE820的方法来获取内存大小
        xor ebx,ebx
        mov di,ards_buf
        mov edx,0x534d4150
        .mem_e820_get:
            mov eax,0xe820
            mov ecx,20
            int 0x15
            jc .mem_e801_get
            add di,cx               ;设置下一个ards储存的位置
            inc word [ards_num]     ;将个数加一
            cmp ebx,0
        jnz .mem_e820_get
        
        ;遍历ards数组，取出最大值,edx储存最大值
        xor edx,edx
        mov cx,[ards_num]           ;循环次数
        mov ebx,ards_buf            ;遍历数组的起始位置
        .find_mem_max:
            mov eax,[ebx]           ;读取基地址低32位
            add eax,[ebx+8]         ;将基地址加上内存长度低32位
            add ebx,20              ;指向数组中的下一个
            cmp eax,edx
            jle .next_area
            mov edx,eax
        .next_area:
        loop .find_mem_max
        jmp .mem_get_acc

        .mem_e801_get:
            mov ax,0xe801
            int 0x15
            jc .mem_88_get
            ;将ax寄存器中的内存大小，从kb转化为byte
            mov cx,0x400            ;1KB
            mul cx
            shl edx,16              ;将edx中的结果左移16位，腾出空间给eax中的计算结果
            and eax,0x0000ffff      ;将eax的高16位清空
            or edx,eax              ;结果合成
            add edx,0x10_0000       ;最后结果需要加上1MB
            push edx                ;暂时保存计算结果
            ;将bx中的内存大小，从kb转为byte
            mov ecx,0x1_0000        ;64KB
            mov ax,bx
            and eax,0x0000ffff
            mul ecx
            pop edx
            add edx,eax
            jmp .mem_get_acc
        .mem_88_get:
            mov ah,0x88
            int 0x15
            jc .mem_get_failure
            mov cx,0x400
            mul cx
            shl edx,16
            and eax,0x0000ffff
            or edx,eax
            add edx,0x10_0000
            jmp .mem_get_acc
        .mem_get_failure:
            mov ax,cs
            mov es,ax
            mov ax,0x1301
            mov cx,mem_failure_msg_len
            mov bx,0x00A4
            mov bp,mem_failure_msg
            mov dx,0x0200
            int 0x10
        hlt
        .mem_get_acc:
            mov [total_mem_bytes],edx   ;保存最后的内存大小
        ;准备进入保护模式
        ;1.打开A20地址线
        ;2.加载gdt
        ;3.开启cr0的PE位
        ;------提示信息--------
        mov sp,LOADER_STACK_TOP
        mov bp,msg
        mov ax,0x1301           ;设置13号功能，且使用更新光标位置
        mov cx,msg_len          ;设置字符串长度
        mov bx,0x001f           ;打印在第0页，字符属性为1f
        mov dx,0x1800           ;打印在第24行，第0列

        int 0x10                ;中断


        ;具体步骤：
        ;1.打开A20
        in al,0x92
        or al,0000_0010b
        out 0x92,al

        ;2.加载gdt
        lgdt [gdt_ptr]

        ;3.开启cr0第0位
        mov eax,cr0
        or eax,1
        mov cr0,eax
        jmp dword SELECTOR_CODE:p_model
        
        [bits 32]
        p_model:
            mov ax,SELECTOR_DATA
            mov ds,ax
            mov es,ax
            mov ss,ax
            mov esp,LOADER_STACK_TOP
            mov ax,SELECTOR_VIDEO
            mov gs,ax

            mov al,'P'
            mov ah,0x1f
            mov [gs:160],ax
            
        .setup_page:
            ;保存寄存器的值
            push eax
            push ebx
            push ecx
            push edx
            push esi
            ;逐字节清除内存
            mov ecx,1024
            mov esi,0
            .clear_memory:
                mov dword [PAGE_DIR_TABLE_POS+esi],0
                add esi,4
            loop .clear_memory
            ;开始创建页目录项（PDE）
            .create_pde:
                mov eax,PAGE_DIR_TABLE_POS
                add eax,0x1000                          ;eax作为页目录项
                mov ebx,eax                             ;ebx为页表的基地址

                or eax,PG_US_U | PG_RW_W | PG_P         ;设置页目录的属性信息
                mov [PAGE_DIR_TABLE_POS + 0x0],eax      ;将该页目录放置到第0个以及虚拟空间顶端的1GB空间里
                mov [PAGE_DIR_TABLE_POS + 0xc00],eax

                sub eax,0x1000                          
                mov [PAGE_DIR_TABLE_POS + 4092],eax     ;将页目录表的第一个作为页目录项，保存在页目录表的最后一个
            
            ;开始创建页表项（PTE）
            mov ecx,256
            mov edx,PG_US_U | PG_RW_W | PG_P
            mov esi,0
            .create_pte:
                mov [ebx+esi*4],edx
                add edx,4096
                inc esi
            loop .create_pte
            ;创建内核空间其它的PDE项
            mov eax,PAGE_DIR_TABLE_POS      ;eax作为页表项值
            mov ebx,eax                     ;ebx作为基地址
            add eax,0x2000                  ;下一个可用的页表地址
            or eax,PG_US_U | PG_RW_W | PG_P ;设置页表项的值
            mov ecx,254                     ;还要设置的页表项的数目
            mov esi,769                     ;还要设置的页表项的起始位置索引
            .create_kernel_pde:
                mov [ebx+esi*4],eax
                add eax,0x1000
                inc esi
            loop .create_kernel_pde
            ;还原寄存器的值
            pop esi
            pop edx
            pop ecx
            pop ebx
            pop eax
            
            ret




















    msg db 'Loader load success, start to open protect model!'
    msg_len equ $-msg
    protect_model_open_msg  db  'Protect model open success!'
    protect_model_open_msg_len equ  $ - protect_model_open_msg
    mem_failure_msg     db 'memory detect error!'
    mem_failure_msg_len equ $-mem_failure_msg