; ===========================================================================
; Diablo Font Renderer (code/engine/font.inc)
; ===========================================================================

InitFont:
    push ebp
    mov  ebp, esp
    sub  esp, 4

    ; 1. Читаем font24g.cel
    mov  eax, [main_archive]
    mov  edx, szFont24Cel
    lea  ecx, [ebp-4]
    call Mpq_ReadFile
    mov  [pFontCel], eax

    ; 2. Читаем font24.bin (таблица ширин 256 байт)
    mov  eax, [main_archive]
    mov  edx, szFont24Bin
    lea  ecx, [ebp-4]
    call Mpq_ReadFile
    mov  [pFontBin], eax

    mov  esp, ebp
    pop  ebp
    ret

; ---------------------------------------------------------------------------
; PrintText - Печать строки с центрированием или по координатам
; Вход:
;   EAX = X
;   EBX = Y
;   EDX = Указатель на ASCIIZ строку
; ---------------------------------------------------------------------------
PrintText:
    push ebp
    mov  ebp, esp
    sub  esp, 16
    push ebx esi edi

    mov  [ebp-4], eax            ; CurX
    mov  [ebp-8], ebx            ; CurY
    mov  [ebp-12], edx           ; pString

    mov  esi, edx

.char_loop:
    xor  eax, eax
    lodsb
    test al, al
    jz   .done

    movzx ebx, al                ; ebx = код символа (номер кадра CEL)

    ; Получаем ширину символа из font24.bin
    mov  edi, [pFontBin]
    movzx ecx, byte [edi + ebx]
    mov  [ebp-16], ecx           ; Сохраняем ширину текущего глифа

    ; Пробел: кадр пустой, просто двигаем курсор
    cmp  al, ' '
    je   .advance

    ; Получаем указатель на кадр глифа в CEL
    mov  eax, [pFontCel]
    mov  edx, ebx
    call CelGetFramePtr
    test eax, eax
    jz   .advance

    ; Отрисовываем глиф: EAX=X, EBX=Y, ECX=pSrcBegin, EDX=pSrcEnd, ESI=Width
    push esi
    mov  edx, ecx                ; pSrcEnd
    mov  ecx, eax                ; pSrcBegin
    mov  eax, [ebp-4]            ; X
    mov  ebx, [ebp-8]            ; Y
    mov  esi, [ebp-16]           ; Glyph Width
    call CelDrawFrame
    pop  esi

.advance:
    mov  eax, [ebp-16]
    add  [ebp-4], eax            ; CurX += CharWidth
    jmp  .char_loop

.done:
    pop  edi esi ebx
    mov  esp, ebp
    pop  ebp
    ret