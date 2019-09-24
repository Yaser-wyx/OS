rm boot.bin -f&&
rm loader.bin -f&&
cd kernel&&
rm main.o -f&&
rm kernel.bin -f&&
cd ..&&
nasm -I include/ -o boot.bin ./boot/boot.asm && 
dd if=./boot.bin of=/home/yaser/workspace/yaser.img bs=512 count=1  conv=notrunc&&
nasm -I include/ -o loader.bin ./boot/loader.asm && 
dd if=./loader.bin  of=/home/yaser/workspace/yaser.img bs=512 count=3 seek=2 conv=notrunc&&
gcc -m32 -c -o kernel/main.o  kernel/main.c && 
ld -m elf_i386 kernel/main.o -Ttext 0xc0001500 -e main -o kernel/kernel.bin && 
dd if=kernel/kernel.bin of=/home/yaser/workspace/yaser.img bs=512 count=200 seek=9 conv=notrunc&&
cd /home/yaser/workspace&&
bochs -f yaser 

