; --- Multiboot Header ---
section .multiboot
align 4
    dd 0x1BADB002              ; magic number
    dd 0x00                    ; flags
    dd -(0x1BADB002 + 0x00)    ; checksum

; --- Stack Setup ---
section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

; --- Kernel Start ---
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top        ; Set up the stack
    call kernel_main          ; Call your C code
    cli
.hang:
    hlt                       ; Halt the CPU if kernel returns
    jmp .hang

; Removes the "executable stack" warning
section .note.GNU-stack noalloc noexec nowrite progbits