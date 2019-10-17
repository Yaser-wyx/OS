BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I include/
ASFLAGS = -f elf
CFLAGS = -m32 -std=c99 -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes \
         -Wmissing-prototypes 
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
      $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
      $(BUILD_DIR)/print_c.o $(BUILD_DIR)/debug.o  $(BUILD_DIR)/memory.o \
      $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/string.o $(BUILD_DIR)/thread.o \
      $(BUILD_DIR)/switch.o $(BUILD_DIR)/thread_list.o $(BUILD_DIR)/console.o\
	  $(BUILD_DIR)/sync.o $(BUILD_DIR)/philosopher.o $(BUILD_DIR)/rand.o \
	  $(BUILD_DIR)/keyboard.o

##############     c代码编译     ###############
$(BUILD_DIR)/main.o: src/kernel/main.c include/thread.h include/interrupt.h include/print.h  include/stdint.h include/init.h\
        include/rand.h include/timer.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: src/kernel/init.c include/init.h include/print.h \
         include/stdint.h include/interrupt.h include/timer.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/print_c.o: lib/kernel/print.c include/stdint.h include/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: src/kernel/interrupt.c include/interrupt.h \
         include/stdint.h include/global.h include/io.h include/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: src/device/timer.c include/timer.h include/stdint.h\
         include/io.h include/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: src/kernel/debug.c include/debug.h \
        include/print.h include/stdint.h include/interrupt.h
	$(CC) $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/string.o: lib/string.c include/string.h  include/global.h \
	include/stdint.h include/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c include/bitmap.h \
    	include/global.h  include/string.h include/stdint.h \
     	include/print.h include/interrupt.h include/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: src/kernel/memory.c include/memory.h  include/bitmap.h \
   	include/global.h  include/debug.h include/print.h \
	include/io.h include/interrupt.h include/string.h include/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread_list.o: lib/kernel/thread_list.c  include/global.h include/stdint.h \
        include/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: lib/kernel/thread.c include/thread.h include/stdint.h include/global.h include/bitmap.h include/memory.h include/string.h include/print.h include/interrupt.h include/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: src/device/console.c include/console.h include/stdint.h \
        include/print.h include/sync.h include/thread_list.h include/global.h \
     	include/thread.h 
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: lib/kernel/sync.c include/sync.h include/thread_list.h include/global.h \
       	include/stdint.h include/thread.h include/string.h include/stdint.h include/debug.h \
		include/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/philosopher.o: src/test/philosopher.c include/sync.h include/philosopher.h include/console.h \
        include/stdint.h
	$(CC) $(CFLAGS) $< -o $@
$(BUILD_DIR)/rand.o: lib/kernel/rand.c include/rand.h include/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: src/device/keyboard.c include/keyboard.h include/print.h \
        include/stdint.h include/interrupt.h include/io.h include/thread.h \
	include/thread_list.h include/global.h include/sync.h
	$(CC) $(CFLAGS) $< -o $@
##############    汇编代码编译    ###############
$(BUILD_DIR)/kernel.o: src/kernel/kernel.asm
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/print.o: lib/kernel/print.asm
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/switch.o: lib/kernel/switch.asm
	$(AS) $(ASFLAGS) $< -o $@
##############    链接所有目标文件    #############
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd clean all 

mk_dir:
	if [[ ! -d $(BUILD_DIR) ]];then mkdir $(BUILD_DIR);fi

hd:
	dd if=$(BUILD_DIR)/kernel.bin \
           of=/home/yaser/workspace/yaser.img \
           bs=512 count=200 seek=9 conv=notrunc

clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/kernel.bin

all: mk_dir clean build hd 
