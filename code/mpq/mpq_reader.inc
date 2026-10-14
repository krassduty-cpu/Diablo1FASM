; ===========================================================================
; StormLib MPQ Reader (code/mpq/mpq_reader.inc)
; ===========================================================================

Mpq_OpenArchive:
    push ebp
    mov  ebp, esp
    sub  esp, 4

    mov  dword [ebp-4], 0
    lea  edx, [ebp-4]

    push edx                     ; &hMpq
    push 0x00000100              ; dwFlags = MPQ_OPEN_READ_ONLY
    push 0                       ; dwPriority = 0
    push eax                     ; wszMpqName
    call [SFileOpenArchive]
    test eax, eax
    jz   .fail

    mov  eax, [ebp-4]
    jmp  .done
.fail:
    xor  eax, eax
.done:
    mov  esp, ebp
    pop  ebp
    ret

Mpq_CloseArchive:
    test eax, eax
    jz   @f
    push eax
    call [SFileCloseArchive]
@@:
    ret

Mpq_ReadFile:
    push ebp
    mov  ebp, esp
    sub  esp, 16                 ; [ebp-4]=hFile, [ebp-8]=fileSize, [ebp-12]=bytesRead, [ebp-16]=pOutSize
    push ebx
    push esi
    push edi

    mov  [ebp-16], ecx
    mov  dword [ebp-4], 0

    ; 1. Открываем файл внутри MPQ
    lea  ecx, [ebp-4]
    push ecx                     ; &hFile
    push 0                       ; dwSearchScope
    push edx                     ; szFileName
    push eax                     ; hArchive
    call [SFileOpenFileEx]
    test eax, eax
    jz   .error

    ; 2. Размер файла
    push 0
    push dword [ebp-4]
    call [SFileGetFileSize]
    cmp  eax, -1
    je   .close_error
    mov  [ebp-8], eax

    ; 3. Выделяем память
    push eax
    call [SDL_malloc]
    add  esp, 4
    test eax, eax
    jz   .close_error
    mov  esi, eax

    ; 4. Читаем данные
    lea  ecx, [ebp-12]
    push 0
    push ecx                     ; &bytesRead
    push dword [ebp-8]           ; fileSize
    push esi                     ; dst buffer
    push dword [ebp-4]           ; hFile
    call [SFileReadFile]
    test eax, eax
    jz   .free_error

    push dword [ebp-4]
    call [SFileCloseFile]

    mov  ecx, [ebp-16]
    test ecx, ecx
    jz   @f
    mov  edx, [ebp-8]
    mov  [ecx], edx
@@:
    mov  eax, esi
    jmp  .done

.free_error:
    push esi
    call [SDL_free]
    add  esp, 4

.close_error:
    push dword [ebp-4]
    call [SFileCloseFile]

.error:
    xor  eax, eax
.done:
    pop  edi
    pop  esi
    pop  ebx
    mov  esp, ebp
    pop  ebp
    ret