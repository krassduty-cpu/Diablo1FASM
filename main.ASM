format PE GUI 4.0
entry start

include 'win32a.inc'
include 'data/structs.inc'

section '.text' code readable executable

start:
    ; 1. Открытие архива MPQ
    mov  eax, wszDiabDat
    call Mpq_OpenArchive
    test eax, eax
    jnz  .mpq_ok

    push 16
    push szTitleErr
    push szErrMpq
    push 0
    call [MessageBoxA]
    push 1
    call [ExitProcess]

.mpq_ok:
    mov  [main_archive], eax

    ; 2. Инициализация графики (окно и поверхности SDL)
    call dx_init
    test eax, eax
    jnz  .dx_ok

    push 16
    push szTitleErr
    push szErrSDL
    push 0
    call [MessageBoxA]
    push 1
    call [ExitProcess]

.dx_ok:
    ; Создаем фоновую поверхность в 32-битном формате (RGBA Little-Endian)
    push 0xFF000000             ; Amask
    push 0x00FF0000             ; Bmask
    push 0x0000FF00             ; Gmask
    push 0x000000FF             ; Rmask
    push SCREEN_BPP             ; 32
    push SCREEN_HEIGHT          ; 480
    push SCREEN_WIDTH           ; 640
    push 0
    call [SDL_CreateRGBSurface]
    add  esp, 32
    mov  [BackdropSurface], eax

    ; 1. Загружаем палитру для курсора и игровых предметов
    mov  eax, szTownPal
    call LoadPalette

    ; 2. Загружаем заставку на экран со своей собственной палитрой
    mov  eax, szTitlePcx
    call LoadPCX_To_Screen

    ; 3. Запоминаем заставку в фоновый буфер
    push 0
    push [BackdropSurface]
    push 0
    push [PalSurface]
    call [SDL_BlitSurface]
    add  esp, 16

    ; 4. Загружаем курсоры из data\inv\objcurs.cel
    mov  eax, [main_archive]
    mov  edx, szObjCursCel
    lea  ecx, [dwDummySize]
    call Mpq_ReadFile
    test eax, eax
    jnz  .cursor_ok

    push 16
    push szTitleErr
    push szErrCursor
    push 0
    call [MessageBoxA]

.cursor_ok:
    mov  [pCursorCel], eax

    ; Скрываем курсор Windows
    push 0
    call [SDL_ShowCursor]
    add  esp, 4

    ; === Загрузка шрифтов меню (font24g.cel + font24.bin) ===
    call InitFont

    ; ===================================================================
    ; Главный игровой цикл
    ; ===================================================================
.game_loop:
    ; 1. Обработка очереди событий SDL
.event_loop:
    push sdlEvent
    call [SDL_PollEvent]
    add  esp, 4
    test eax, eax
    jz   .render_frame

    mov  eax, [sdlEvent]
    cmp  eax, 0x100              ; SDL_QUIT (нажатие на крестик окна)
    je   .exit_game

    cmp  eax, 0x300              ; SDL_KEYDOWN
    jne  .check_motion
    cmp  dword [sdlEvent + 20], 27 ; Клавиша ESC
    je   .exit_game

.check_motion:
    cmp  eax, 0x400              ; SDL_MOUSEMOTION (движение мыши)
    jne  .event_loop
    mov  edx, [sdlEvent + 20]    ; X координата мыши
    mov  [MouseX], edx
    mov  edx, [sdlEvent + 24]    ; Y координата мыши
    mov  [MouseY], edx
    jmp  .event_loop

    ; 2. Отрисовка кадра
.render_frame:
    ; Восстанавливаем чистый фон заставки
    push 0
    push [PalSurface]
    push 0
    push [BackdropSurface]
    call [SDL_BlitSurface]
    add  esp, 16

    ; === Отрисовка надписей меню золотым шрифтом Diablo ===
    mov  eax, 230               ; X
    mov  ebx, 240               ; Y
    mov  edx, szMenuSingle      ; "SINGLE PLAYER"
    call PrintText

    mov  eax, 235
    mov  ebx, 275
    mov  edx, szMenuMulti       ; "MULTI PLAYER"
    call PrintText

    mov  eax, 235
    mov  ebx, 310
    mov  edx, szMenuReplay      ; "REPLAY INTRO"
    call PrintText

    mov  eax, 230
    mov  ebx, 345
    mov  edx, szMenuCredits     ; "SHOW CREDITS"
    call PrintText

    mov  eax, 235
    mov  ebx, 380
    mov  edx, szMenuExit        ; "EXIT DIABLO"
    call PrintText

    ; === Отрисовка курсора мыши (перчатка поверх меню) ===
    cmp  [pCursorCel], 0
    jz   .present

    mov  eax, [pCursorCel]
    mov  edx, 1                 ; Кадр 1 = золотая перчатка
    call CelGetFramePtr
    test eax, eax
    jz   .present

    ; EAX = pSrcBegin, ECX = pSrcEnd
    mov  edx, ecx               ; EDX = pSrcEnd
    mov  ecx, eax               ; ECX = pSrcBegin
    mov  eax, [MouseX]          ; X
    mov  ebx, [MouseY]          ; Y
    mov  esi, 33                ; FrameWidth = 33 px
    call CelDrawFrame

.present:
    call RenderPresent

    ; Ограничение 60 FPS (~16 мс)
    push 16
    call [SDL_Delay]
    add  esp, 4

    cmp  [bRunning], 1
    je   .game_loop

.exit_game:
    cmp  [pCursorCel], 0
    jz   @f
    push [pCursorCel]
    call [SDL_free]
    add  esp, 4
@@:
    cmp  [BackdropSurface], 0
    jz   @f
    push [BackdropSurface]
    call [SDL_FreeSurface]
    add  esp, 4
@@:
    call dx_cleanup
    mov  eax, [main_archive]
    call Mpq_CloseArchive

    push 0
    call [ExitProcess]

; --- Подключение модулей движка ---
include 'code/mpq/mpq_reader.inc'
include 'code/mpq/mpq_writer.inc'
include 'code/mpq/mpq_sdl_rwops.inc'
include 'code/engine/dx.inc'
include 'code/engine/surface.inc'
include 'code/engine/palette.inc'
include 'code/engine/load_pcx.inc'
include 'code/engine/cel_decode.inc'
include 'code/engine/font.inc'

section '.data' data readable writeable

wszDiabDat       du 'DIABDAT.MPQ', 0
szTitlePcx       db 'ui_art\title.pcx', 0
szTownPal        db 'levels\towndata\town.pal', 0
szObjCursCel     db 'data\inv\objcurs.cel', 0

szTitleErr       db 'Diablo Engine', 0
szErrMpq         db 'Не удалось открыть DIABDAT.MPQ!', 0
szErrSDL         db 'Ошибка SDL2!', 0
szErrCursor      db 'Не удалось найти data\inv\objcurs.cel в MPQ архиве!', 0

szFont24Cel      db 'ui_art\font24g.cel', 0   ; Золотой шрифт 24px
szFont24Bin      db 'ui_art\font24.bin', 0   ; Таблица ширин символов (256 байт)

; Строки главного меню
szMenuSingle     db 'SINGLE PLAYER', 0
szMenuMulti      db 'MULTI PLAYER', 0
szMenuReplay     db 'REPLAY INTRO', 0
szMenuCredits    db 'SHOW CREDITS', 0
szMenuExit       db 'EXIT DIABLO', 0

pFontCel         dd 0
pFontBin         dd 0

dwDummySize      dd 0
BackdropSurface  dd 0

include 'data/mpq_data.inc'
include 'data/dx_data.inc'
include 'data/palette_data.inc'

section '.idata' import data readable

library kernel32, 'KERNEL32.DLL',\
        user32,   'USER32.DLL',\
        sdl2,     'SDL2.DLL',\
        storm,    'STORMLIB.DLL'

import kernel32,\
       ExitProcess,              'ExitProcess'

import user32,\
       MessageBoxA,              'MessageBoxA'

import sdl2,\
       SDL_Init,                 'SDL_Init',\
       SDL_Quit,                 'SDL_Quit',\
       SDL_CreateWindow,         'SDL_CreateWindow',\
       SDL_DestroyWindow,        'SDL_DestroyWindow',\
       SDL_GetWindowSurface,     'SDL_GetWindowSurface',\
       SDL_UpdateWindowSurface,  'SDL_UpdateWindowSurface',\
       SDL_CreateRGBSurface,     'SDL_CreateRGBSurface',\
       SDL_FreeSurface,          'SDL_FreeSurface',\
       SDL_AllocPalette,         'SDL_AllocPalette',\
       SDL_FreePalette,          'SDL_FreePalette',\
       SDL_SetPaletteColors,     'SDL_SetPaletteColors',\
       SDL_SetSurfacePalette,    'SDL_SetSurfacePalette',\
       SDL_BlitSurface,          'SDL_UpperBlit',\
       SDL_Delay,                'SDL_Delay',\
       SDL_PollEvent,            'SDL_PollEvent',\
       SDL_ShowCursor,           'SDL_ShowCursor',\
       SDL_malloc,               'SDL_malloc',\
       SDL_free,                 'SDL_free',\
       SDL_FreeRW,               'SDL_FreeRW'

import storm,\
       SFileOpenArchive,         'SFileOpenArchive',\
       SFileCloseArchive,        'SFileCloseArchive',\
       SFileOpenFileEx,          'SFileOpenFileEx',\
       SFileGetFileSize,         'SFileGetFileSize',\
       SFileReadFile,            'SFileReadFile',\
       SFileCloseFile,           'SFileCloseFile',\
       SFileSetFilePointer,      'SFileSetFilePointer',\
       SFileCreateArchive,       'SFileCreateArchive',\
       SFileRemoveFile,          'SFileRemoveFile'