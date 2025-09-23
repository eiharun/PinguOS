GCCPARAMS = -m32 -g -Iinclude -ffreestanding -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

BUILD_DIR := build
SRC_DIR := src
objects =  	loader gdt \
			hardware_communication/port \
			hardware_communication/interruptstubs \
			hardware_communication/interrupts \
			hardware_communication/pci \
			drivers/driver drivers/keyboard drivers/mouse \
			drivers/vga \
			gui/widget gui/desktop gui/window \
			kernel
OBJS := $(objects:%=$(BUILD_DIR)/%.o)

# mkdir_build:
# 	mkdir -p $(BUILD_DIR)

# $(OBJS): mkdir_build

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s 
	mkdir -p $(@D)
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
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/pingukernel.iso 



.PHONY: debug kill build clean

build: $(BUILD_DIR)/pingukernel.bin

debug: $(BUILD_DIR)/pingukernel.iso
	tmux new-session -d -s qemu_debug \
		"qemu-system-i386 -s -S -drive format=raw,file=$(BUILD_DIR)/pingukernel.iso -monitor stdio" \; \
		split-window -h "sleep 2; gdb -ex 'target remote :1234' -ex 'set architecture i386' -ex 'file $(BUILD_DIR)/pingukernel.bin' " \; \
		select-pane -t 0 \; \
		attach

kill:
	tmux kill-session -t qemu_debug 2>/dev/null || echo "No session 'qemu_debug' found"

clean:
	rm -r build