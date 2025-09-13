.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj ; # Cant use extern C since it is a class method (C has no knowledge of classes)
.global ignoreInterruptRequest


.macro handleExceptions num
.global _ZN16InterruptManager16handleExceptions\num\()Ev
_ZN16InterruptManager16handleExceptions\num\()Ev:
    movb $\num, (interrupt_number)
    jmp int_bottom
.endm

.macro handleInterruptRequest num
.global handleInterruptRequest\num\()
handleInterruptRequest\num\():
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

    ; pushl %ebp
    ; pushl %edi
    ; pushl %esi

    ; pushl %edx
    ; pushl %ecx
    ; pushl %ebx
    ; pushl %eax

    pushl %esp
    push (interrupt_number)
    call _ZN16InterruptManager15handleInterruptEhj
    movl %eax, %esp

    ; popl %eax
    ; popl %ebx
    ; popl %ecx
    ; popl %edx

    ; popl %esi
    ; popl %edi
    ; popl %ebp

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa
    ; add $4, %esp

ignoreInterruptRequest:
    iret 

.data
    interrupt_number: .byte 0
