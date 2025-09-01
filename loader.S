.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC ; #Magic number
    .long FLAGS
    .long CHECKSUM 

.section .text
    .extern pingu_kernel_main
    .extern call_constructors
    .global loader

loader:
    mov $kernel_stack, %esp; #Set stack pointer

    call call_constructors

    push %eax
    push %ebx
    call pingu_kernel_main

_stop:
    cli ; #clear flags
    hlt ; #halt
    jmp _stop

.section .bss
.space 2*1024*1024; #2MB
kernel_stack:
