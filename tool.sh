# 写入引导程序
nasm -I include/ -o ./build/boot.bin ./boot/boot.asm && 
dd if=./build/boot.bin of=/home/yaser/workspace/yaser.img bs=512 count=1  conv=notrunc&&
nasm -I include/ -o ./build/loader.bin ./boot/loader.asm && 
dd if=./build/loader.bin  of=/home/yaser/workspace/yaser.img bs=512 count=3 seek=2 conv=notrunc&&

# 编译文件
gcc -m32 -std=c99 -I lib/kernel/ -I kernel/ -c -fno-builtin -o build/main.o kernel/main.c&&
nasm -f elf -o build/print.o lib/kernel/print.asm&&
nasm -f elf -o build/kernel.o kernel/kernel.asm&&
gcc -m32 -std=c99 -I lib/kernel/ -c -fno-builtin -o build/print_c.o lib/kernel/print.c&&
gcc -m32 -std=c99 -I lib/kernel/ -I kernel/ -c -fno-builtin -o build/interrupt.o kernel/interrupt.c&&
gcc -m32 -std=c99 -I lib/kernel/ -I kernel/ -c -fno-builtin -o build/init.o kernel/init.c&&

# 链接
ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernel.bin build/main.o build/init.o build/interrupt.o build/print_c.o build/print.o build/kernel.o&& 

# 写入文件
dd if=build/kernel.bin of=/home/yaser/workspace/yaser.img bs=512 count=200 seek=9 conv=notrunc&&
# 运行
cd /home/yaser/workspace&&
bochs -f yaser 

