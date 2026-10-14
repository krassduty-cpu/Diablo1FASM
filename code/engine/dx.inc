; ===========================================================================
; Graphics Pipeline & Presentation (code/engine/dx.inc)
; ===========================================================================

dx_init:
    push ebp
    mov  ebp, esp

    ; 1. Инициализация видео SDL
    push 0x00000020          ; SDL_INIT_VIDEO
    call [SDL_Init]
    add  esp, 4
    test eax, eax
    jnz  .fail

    ; 2. Создание окна 640x480
    push 0                   ; flags
    push SCREEN_HEIGHT
    push SCREEN_WIDTH
    push 0x1FFF0000          ; SDL_WINDOWPOS_CENTERED
    push 0x1FFF0000          ; SDL_WINDOWPOS_CENTERED
    push szWindowTitle
    call [SDL_CreateWindow]
    add  esp, 24
    test eax, eax
    jz   .fail
    mov  [ghMainWnd], eax

    ; 3. Создаем буфер экрана PalSurface в 32 бита (RGBA / Little-Endian)
    push 0xFF000000          ; Amask
    push 0x00FF0000          ; Bmask
    push 0x0000FF00          ; Gmask
    push 0x000000FF          ; Rmask
    push SCREEN_BPP          ; 32 бита
    push SCREEN_HEIGHT       ; 480
    push SCREEN_WIDTH        ; 640
    push 0                   ; flags
    call [SDL_CreateRGBSurface]
    add  esp, 32
    test eax, eax
    jz   .fail
    mov  [PalSurface], eax

    mov  eax, 1
    jmp  .done

.fail:
    xor  eax, eax
.done:
    pop  ebp
    ret

dx_cleanup:
    cmp  [PalSurface], 0
    jz   @f
    push [PalSurface]
    call [SDL_FreeSurface]
    add  esp, 4
    mov  [PalSurface], 0
@@:
    cmp  [ghMainWnd], 0
    jz   @f
    push [ghMainWnd]
    call [SDL_DestroyWindow]
    add  esp, 4
    mov  [ghMainWnd], 0
@@:
    call [SDL_Quit]
    ret

RenderPresent:
    push ebp
    mov  ebp, esp

    push [ghMainWnd]
    call [SDL_GetWindowSurface]
    add  esp, 4
    test eax, eax
    jz   .exit

    push 0
    push eax                 ; WindowSurface
    push 0
    push [PalSurface]
    call [SDL_BlitSurface]
    add  esp, 16

    push [ghMainWnd]
    call [SDL_UpdateWindowSurface]
    add  esp, 4

.exit:
    pop  ebp
    ret