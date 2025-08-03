.code
my_mem_copy PROC
;just experiment on how to create an asm function
;link it, and use in c/c++ code
    ; RCX contains the count of bytes to copy
    ; RDX contains the source address (RSI)
    ; R8 contains the destination address (RDI)

    mov rax, rcx    ; Store destination address in RAX for return value
    mov rdi, rcx    ; Destination pointer to RDI
    mov rsi, rdx    ; Source pointer to RSI
    mov rcx, r8     ; Count to RCX (for REP instruction)

    

    rep movsb           ; Repeat MOVSB until RCX is zero

    ret                 ; Return from the procedure

my_mem_copy ENDP
end