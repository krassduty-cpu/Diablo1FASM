; ===========================================================================
; Palette Management (code/engine/palette.inc)
; ===========================================================================

palette_init:
    call BlackPalette
    ret

; ---------------------------------------------------------------------------
; Применить уровень яркости к палитре (Fade Level)
; Вход: EAX = fadeval (0 - черный экран, 256 - полная яркость)
;       ESI = исходная палитра (logical_palette)
;       EDI = целевая палитра (system_palette)
; ---------------------------------------------------------------------------
ApplyFadeLevel:
    push ebx
    push ecx
    push edx
    mov  ecx, 256

.loop:
    ; Красный
    movzx edx, byte [esi]
    imul  edx, eax
    shr   edx, 8
    mov   [edi], dl

    ; Зеленый
    movzx edx, byte [esi+1]
    imul  edx, eax
    shr   edx, 8
    mov   [edi+1], dl

    ; Синий
    movzx edx, byte [esi+2]
    imul  edx, eax
    shr   edx, 8
    mov   [edi+2], dl

    ; Альфа канал всегда 255
    mov   byte [edi+3], 255

    add  esi, 4
    add  edi, 4
    dec  ecx
    jnz  .loop

    pop  edx
    pop  ecx
    pop  ebx
    ret

; ---------------------------------------------------------------------------
; Обновить системную палитру в SDL
; ---------------------------------------------------------------------------
SystemPaletteUpdated:
    push 256
    push 0
    push system_palette
    push [sdlPalette]
    call [SDL_SetPaletteColors]
    add  esp, 16
    ret

; ---------------------------------------------------------------------------
; Полностью черный экран
; ---------------------------------------------------------------------------
BlackPalette:
    push edi
    mov  edi, system_palette
    mov  ecx, 256
.clear_loop:
    mov  dword [edi], 0xFF000000 ; Черный цвет, Альфа=255
    add  edi, 4
    dec  ecx
    jnz  .clear_loop
    pop  edi
    call SystemPaletteUpdated
    ret

; ---------------------------------------------------------------------------
; Загрузка .PAL файла из MPQ
; Вход: EAX = путь к палитре (например, "levels\towndata\town.pal")
; ---------------------------------------------------------------------------
LoadPalette:
    push ebp
    mov  ebp, esp
    sub  esp, 8              ; [ebp-4] = fileSize, [ebp-8] = pBuffer
    push esi edi ebx

    mov  edx, eax
    mov  eax, [main_archive]
    lea  ecx, [ebp-4]
    call Mpq_ReadFile
    test eax, eax
    jz   .done

    mov  [ebp-8], eax        ; Сохраняем оригинальный указатель для free!
    mov  esi, eax            ; ESI = буфер 768 байт (256 * RGB)
    mov  edi, logical_palette
    mov  ecx, 256

.convert_pal:
    movzx ebx, byte [esi]    ; R
    mov   [edi], bl
    movzx ebx, byte [esi+1]  ; G
    mov   [edi+1], bl
    movzx ebx, byte [esi+2]  ; B
    mov   [edi+2], bl
    mov   byte [edi+3], 255  ; A

    add   esi, 3
    add   edi, 4
    dec   ecx
    jnz   .convert_pal

    ; Освобождаем память буфера MPQ
    push dword [ebp-8]
    call [SDL_free]
    add  esp, 4

    ; Переносим в system_palette на 100% яркости (fade = 256)
    mov  eax, 256
    mov  esi, logical_palette
    mov  edi, system_palette
    call ApplyFadeLevel
    call SystemPaletteUpdated

.done:
    pop  ebx edi esi
    mov  esp, ebp
    pop  ebp
    ret