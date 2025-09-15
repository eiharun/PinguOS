.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj # # Cant use extern C since it is a class method (C has no knowledge of classes)
.global _ZN16InterruptManager22ignoreInterruptRequestEv


.macro handleExceptions num
.global _ZN16InterruptManager16handleExceptions\num\()Ev
_ZN16InterruptManager16handleExceptions\num\()Ev:
    movb $\num, (interrupt_number)
    jmp int_bottom
.endm

.macro handleInterruptRequest num
.global _ZN16InterruptManager26handleInterruptRequest\num\()Ev
_ZN16InterruptManager26handleInterruptRequest\num\()Ev:
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

    # pushl %ebp
    # pushl %edi
    # pushl %esi

    # pushl %edx
    # pushl %ecx
    # pushl %ebx
    # pushl %eax

    pushl %esp
    push (interrupt_number)
    # movl %esp, %eax
    # addl $48, %eax
    # pushl %eax
    # movzbl interrupt_number, %eax
    # pushl %eax
    call _ZN16InterruptManager15handleInterruptEhj
    movl %eax, %esp
    # addl $8, %esp

    # popl %eax
    # popl %ebx
    # popl %ecx
    # popl %edx

    # popl %esi
    # popl %edi
    # popl %ebp
    # add $4, %esp

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa
    iret

_ZN16InterruptManager22ignoreInterruptRequestEv:
    iret 

.data
    interrupt_number: .byte 0
