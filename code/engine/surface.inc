; ===========================================================================
; Software Surface Blitter (code/engine/surface.inc)
; ===========================================================================

; ---------------------------------------------------------------------------
; Surface_Blit: Быстрое копирование прямоугольника из одного 8-бит буфера в другой
; Вход: ESI = pSrcPixels, EDI = pDstPixels
;       EAX = width,     EBX = height
;       ECX = srcPitch,  EDX = dstPitch
; ---------------------------------------------------------------------------
Surface_Blit:
    push ebp
    push ebx
    push esi
    push edi

    sub  ecx, eax            ; смещение шага строки источника
    sub  edx, eax            ; смещение шага строки приемника

.row_loop:
    push eax                 ; сохраняем width
    mov  ebp, eax
    shr  ecx, 2              ; копируем dword'ами если возможно
    ; Для простоты копируем побайтно
@@:
    movsb
    dec  ebp
    jnz  @b

    pop  eax                 ; восстанавливаем width
    add  esi, ecx            ; перенос строки src
    add  edi, edx            ; перенос строки dst
    dec  ebx
    jnz  .row_loop

    pop  edi
    pop  esi
    pop  ebx
    pop  ebp
    ret

; ---------------------------------------------------------------------------
; Surface_BlitSkipZero: Блиттер с прозрачностью (цвет 0 не копируется)
; Вход: ESI = pSrcPixels, EDI = pDstPixels
;       EAX = width,     EBX = height
;       ECX = srcPitch,  EDX = dstPitch
; ---------------------------------------------------------------------------
Surface_BlitSkipZero:
    push ebp
    push ebx
    push esi
    push edi

    sub  ecx, eax
    sub  edx, eax

.row_loop:
    push eax
    mov  ebp, eax

.col_loop:
    mov  al, [esi]
    test al, al              ; Проверяем индекс 0 (прозрачность)
    jz   .skip_pixel
    mov  [edi], al
.skip_pixel:
    inc  esi
    inc  edi
    dec  ebp
    jnz  .col_loop

    pop  eax
    add  esi, ecx
    add  edi, edx
    dec  ebx
    jnz  .row_loop

    pop  edi
    pop  esi
    pop  ebx
    pop  ebp
    ret