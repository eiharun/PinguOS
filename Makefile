GCCPARAMS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = kernel.o loader.o

%.o:%.cpp
	gcc $(GCCPARAMS) -c -o $@ $<

%.o:%.S
	as $(ASPARAMS) -o $@ $<

pingukernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)


pingukernel.iso: pingukernel.bin
	mkdir -p iso
	mkdir -p iso/boot
	cp $< iso/boot/pingukernel.bin
	mkdir -p iso/boot/grub
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "PinguOS" {' >> iso/boot/grub/grub.cfg
	echo '	multiboot /boot/pingukernel.bin' >> iso/boot/grub/grub.cfg
	echo '	boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=$@ iso
	rm -r iso

run: pingukernel.iso
# kill current vm if necesarry
	qemu-system-x86_64 -drive format=raw,file=pingukernel.iso 

.PHONY: clean
clean:
	rm *.o   
	rm *.bin 
	rm *.iso 