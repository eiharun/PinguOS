.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj ; # Cant use extern C since it is a class method (C has no knowledge of classes)

.macro handleExceptions num
.global _ZN16InterruptManager16handleExceptions\num\()Ev
    movb $\num, (interrupt_number)
    jmp int_bottom
.endm

.macro handleInterruptRequest num
.global _ZN16InterruptManager26handleInterruptRequest\num\()Ev
    movb $\num + IRQ_BASE, (interrupt_number)
    jmp int_bottom
.endm

handleInterruptRequest 0x00
handleInterruptRequest 0x01

int_bottom:
    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushl %esp
    push (interrupt_number)
    call _ZN16InterruptManager15handleInterruptEhj
    movl %eax, %esp

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa

.data
    interrupt_number: .byte 0
