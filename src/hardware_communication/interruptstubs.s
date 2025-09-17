.set IRQ_BASE, 0x20

.section .note.GNU-stack,"",@progbits

.section .text

.extern _ZN22hardware_communication16InterruptManager15handleInterruptEhj # # Cant use extern C since it is a class method (C has no knowledge of classes)
.global _ZN22hardware_communication16InterruptManager22ignoreInterruptRequestEv


.macro handleException num
.global _ZN22hardware_communication16InterruptManager16handleException\num\()Ev
_ZN22hardware_communication16InterruptManager16handleException\num\()Ev:
    movb $\num, (interrupt_number)
    jmp int_bottom
.endm

.macro handleInterruptRequest num
.global _ZN22hardware_communication16InterruptManager26handleInterruptRequest\num\()Ev
_ZN22hardware_communication16InterruptManager26handleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interrupt_number)
    jmp int_bottom
.endm

handleException 0x00
handleException 0x01
handleException 0x02
handleException 0x03
handleException 0x04
handleException 0x05
handleException 0x06
handleException 0x07
handleException 0x08
handleException 0x09
handleException 0x0A
handleException 0x0B
handleException 0x0C
handleException 0x0D
handleException 0x0E
handleException 0x0F
handleException 0x10
handleException 0x11
handleException 0x12
handleException 0x13

handleInterruptRequest 0x00 # PIC
handleInterruptRequest 0x01 # Keyboard
handleInterruptRequest 0x02
handleInterruptRequest 0x03
handleInterruptRequest 0x04
handleInterruptRequest 0x05
handleInterruptRequest 0x06
handleInterruptRequest 0x07
handleInterruptRequest 0x08
handleInterruptRequest 0x09
handleInterruptRequest 0x0A
handleInterruptRequest 0x0B
handleInterruptRequest 0x0C # Mouse
handleInterruptRequest 0x0D
handleInterruptRequest 0x0E
handleInterruptRequest 0x0F
handleInterruptRequest 0x31

handleInterruptRequest 0x80

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
    call _ZN22hardware_communication16InterruptManager15handleInterruptEhj
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

_ZN22hardware_communication16InterruptManager22ignoreInterruptRequestEv:
    iret 

.data
    interrupt_number: .byte 0
