; ===========================================================================
; CEL Decoder & Blitter 32-bit (code/engine/cel_decode.inc)
; ===========================================================================

CelGetFramePtr:
    push ebx
    test eax, eax
    jz   .fail
    test edx, edx
    jz   .fail

    mov  ebx, [eax]
    cmp  edx, ebx
    ja   .fail

    mov  ecx, [eax + edx * 4 + 4]
    add  ecx, eax               ; ECX = srcEnd
    mov  ebx, [eax + edx * 4]
    add  eax, ebx               ; EAX = srcBegin

    cmp  word [eax], 10
    jne  .ok
    add  eax, 10
.ok:
    pop  ebx
    ret

.fail:
    xor  eax, eax
    xor  ecx, ecx
    pop  ebx
    ret

; ---------------------------------------------------------------------------
; CelDrawFrame
; Отрисовывает 8-битный CEL кадр в 32-битный буфер PalSurface
; Вход:
;   EAX = X
;   EBX = Y
;   ECX = pSrcBegin
;   EDX = pSrcEnd
;   ESI = FrameWidth (33)
; ---------------------------------------------------------------------------
CelDrawFrame:
    push ebp
    mov  ebp, esp
    sub  esp, 24
    push ebx esi edi

    mov  [ebp-4], eax            ; X
    mov  [ebp-8], ebx            ; Y
    mov  [ebp-12], esi           ; FrameWidth
    mov  [ebp-16], edx           ; pSrcEnd
    mov  dword [ebp-20], SCREEN_WIDTH
    mov  dword [ebp-24], 0

    test ecx, ecx
    jz   .exit

    mov  esi, ecx
    cld

    mov  edi, [PalSurface]
    test edi, edi
    jz   .exit
    mov  edi, [edi + 20]
    test edi, edi
    jz   .exit

    ; Стартовая строка курсора: Y + 32 (снизу вверх)
    mov  edx, [ebp-8]
    add  edx, 32
    cmp  edx, SCREEN_HEIGHT
    jge  .exit
    imul edx, [ebp-20]
    add  edx, [ebp-4]
    shl  edx, 2                 ; Умножаем на 4 (32-битные пиксели!)
    add  edi, edx

.line_loop:
    cmp  esi, [ebp-16]
    jae  .exit

    mov  edx, [ebp-12]          ; Оставшаяся ширина строки

.pixel_loop:
    test edx, edx
    jle  .end_line

    cmp  esi, [ebp-16]
    jae  .exit

    xor  eax, eax
    lodsb

    test al, 0x80
    jnz  .transparent_run

    ; Непрозрачные пиксели
    movzx ecx, al
    sub  edx, ecx

.copy_opaque:
    xor  eax, eax
    lodsb
    ; Берем цвет из палитры town.pal (массив logical_palette)
    mov  eax, dword [logical_palette + eax * 4]
    mov  [edi], eax
    add  edi, 4
    dec  ecx
    jnz  .copy_opaque
    jmp  .pixel_loop

.transparent_run:
    neg  al
    movzx ecx, al
    sub  edx, ecx
    shl  ecx, 2                 ; 4 байта на пиксель пропуска
    add  edi, ecx
    jmp  .pixel_loop

.end_line:
    ; Переход на строку выше: EDI = EDI - (Width + SCREEN_WIDTH) * 4
    mov  eax, [ebp-12]
    add  eax, [ebp-20]
    shl  eax, 2
    sub  edi, eax

    inc  dword [ebp-24]
    cmp  dword [ebp-24], 40
    jge  .exit
    jmp  .line_loop

.exit:
    pop  edi esi ebx
    mov  esp, ebp
    pop  ebp
    ret