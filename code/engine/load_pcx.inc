; ===========================================================================
; PCX Loader to 32-bit Screen (code/engine/load_pcx.inc)
; ===========================================================================

LoadPCX_To_Screen:
    push ebp
    mov  ebp, esp
    ; Выделяем 1024 байта на стеке под быструю 32-битную палитру + переменные
    sub  esp, 1024 + 16
    push ebx esi edi

    ; [ebp-4]    = fileSize
    ; [ebp-8]    = pRawBuffer
    ; [ebp-1040] = 32-битная палитра PCX (256 * 4 байта)

    mov  edx, eax
    mov  eax, [main_archive]
    lea  ecx, [ebp-4]
    call Mpq_ReadFile
    test eax, eax
    jz   .fail

    mov  [ebp-8], eax

    ; 1. Находим 768 байт палитры в конце файла PCX
    mov  esi, [ebp-4]
    sub  esi, 768
    add  esi, eax                ; esi = палитра PCX (RGB)

    ; 2. Преобразуем 768 байт RGB во временную 32-битную RGBA палитру на стеке
    lea  edi, [ebp-1040]
    mov  ecx, 256
.convert_palette:
    movzx eax, byte [esi]        ; R
    mov   [edi + 0], al
    movzx eax, byte [esi + 1]    ; G
    mov   [edi + 1], al
    movzx eax, byte [esi + 2]    ; B
    mov   [edi + 2], al
    mov   byte [edi + 3], 255    ; A = 255
    add   esi, 3
    add   edi, 4
    dec   ecx
    jnz   .convert_palette

    ; 3. Получаем буфер пикселей экрана (PalSurface->pixels)
    mov  edi, [PalSurface]
    test edi, edi
    jz   .cleanup
    mov  edi, [edi + 20]
    test edi, edi
    jz   .cleanup

    ; Пиксели PCX начинаются со 128-го байта
    mov  esi, [ebp-8]
    add  esi, 128
    mov  ecx, (SCREEN_WIDTH * SCREEN_HEIGHT)

.decode_loop:
    test ecx, ecx
    jle  .cleanup

    xor  eax, eax
    lodsb

    mov  edx, eax
    and  edx, 0xC0
    cmp  edx, 0xC0
    jne  .single_pixel

    ; Серия повторяющихся пикселей (RLE Run)
    and  eax, 0x3F
    mov  edx, eax                ; edx = количество повторов
    lodsb                        ; al = индекс цвета

    ; Берем готовый 32-битный RGBA цвет
    movzx eax, al
    mov  eax, [ebp - 1040 + eax * 4]

.run_loop:
    mov  [edi], eax
    add  edi, 4
    dec  ecx
    dec  edx
    jnz  .run_loop
    jmp  .decode_loop

.single_pixel:
    ; Одиночный пиксель
    movzx eax, al
    mov  eax, [ebp - 1040 + eax * 4]
    mov  [edi], eax
    add  edi, 4
    dec  ecx
    jmp  .decode_loop

.cleanup:
    push dword [ebp-8]
    call [SDL_free]
    add  esp, 4

    mov  eax, 1
    jmp  .done

.fail:
    xor  eax, eax

.done:
    pop  edi esi ebx
    mov  esp, ebp
    pop  ebp
    ret