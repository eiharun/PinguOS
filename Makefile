GCCPARAMS = -m32 -g -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
ASPARAMS = --32
LDPARAMS = -melf_i386

BUILD_DIR := build
objects = kernel.o loader.o gdt.o port.o interrupts.o
OBJS := $(objects:%=$(BUILD_DIR)/%)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o:%.cpp $(BUILD_DIR)
	gcc $(GCCPARAMS) -c -o $@ $<

$(BUILD_DIR)/%.o:%.S $(BUILD_DIR)
	as $(ASPARAMS) -o $@ $<

$(BUILD_DIR)/pingukernel.bin: linker.ld $(OBJS) 
	ld $(LDPARAMS) -T $< -o $@ $(OBJS)


$(BUILD_DIR)/pingukernel.iso: $(BUILD_DIR)/pingukernel.bin
	mkdir -p $(BUILD_DIR)/iso
	mkdir -p $(BUILD_DIR)/iso/boot
	cp $< $(BUILD_DIR)/iso/boot/pingukernel.bin
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	echo 'set timeout=0' > $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'set default=0' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo 'menuentry "PinguOS" {' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '	multiboot /boot/pingukernel.bin' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '	boot' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/iso/boot/grub/grub.cfg
	grub-mkrescue --output=$@ $(BUILD_DIR)/iso
	rm -r $(BUILD_DIR)/iso

run: $(BUILD_DIR)/pingukernel.iso
# kill current vm if necesarry
	qemu-system-x86_64 -drive format=raw,file=$(BUILD_DIR)/pingukernel.iso 

.PHONY: clean
clean:
# 	rm *.o   
# 	rm *.bin 
# 	rm *.iso 
	rm -r build