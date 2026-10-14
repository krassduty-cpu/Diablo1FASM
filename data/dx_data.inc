; ===========================================================================
; Graphics & Window Data (data/dx_data.inc)
; ===========================================================================

align 4
ghMainWnd           dd 0    ; SDL_Window*
sdlRenderer         dd 0    ; SDL_Renderer*
sdlTexture          dd 0    ; SDL_Texture*
PalSurface          dd 0    ; SDL_Surface*
sdlPalette          dd 0    ; SDL_Palette*

szWindowTitle       db 'Diablo (FASM Engine)', 0

; --- Переменные мыши и игрового цикла ---
MouseX              dd 320
MouseY              dd 240
MouseButton         dd 0
bRunning            dd 1

pCursorCel          dd 0
CursorFrame         dd 1
CursorWidth         dd 32
CursorHeight        dd 32

; Буфер под структуру SDL_Event
align 4
sdlEvent:           rb 64