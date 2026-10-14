; ===========================================================================
; Structures (data/structs.inc)
; ===========================================================================

SCREEN_WIDTH   equ 640
SCREEN_HEIGHT  equ 480
SCREEN_BPP     equ 32

; Контекст для чтения MPQ через SDL RWops
virtual at 0
  StormStreamCtx:
    .hFile      dd ?    ; HANDLE файла внутри архива StormLib
    .fileSize   dd ?    ; Общий размер файла
    .curPos     dd ?    ; Текущая позиция чтения
    .sizeof = $ - StormStreamCtx
end virtual

virtual at 0
  SDL_Color:
    .r  db ?
    .g  db ?
    .b  db ?
    .a  db ?
end virtual