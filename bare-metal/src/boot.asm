; Multiboot2 header + 32-bit entry stub for GRUB.
; https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

MAGIC equ 0xE85250D6
ARCH  equ 0
HEADER_LENGTH equ (header_end - header_start)
CHECKSUM equ -(MAGIC + ARCH + HEADER_LENGTH)

section .multiboot_header
align 8
header_start:
    dd MAGIC
    dd ARCH
    dd HEADER_LENGTH
    dd CHECKSUM

    ; End tag (required)
    align 8
    dw 0
    dw 0
    dd 8
header_end:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
