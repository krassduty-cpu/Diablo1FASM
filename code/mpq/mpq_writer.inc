; ===========================================================================
; StormLib MPQ Writer (code/mpq/mpq_writer.inc)
; ===========================================================================

; ---------------------------------------------------------------------------
; Mpq_CreateArchive: Создание нового MPQ (для сейвов)
; Вход: EAX = szMpqName, EDX = maxFiles
; Выход: EAX = HANDLE
; ---------------------------------------------------------------------------
Mpq_CreateArchive:
    push ebp
    mov  ebp, esp
    sub  esp, 4

    lea  ecx, [ebp-4]
    push ecx                     ; &hMpq
    push edx                     ; dwMaxFileCount
    push MPQ_CREATE_ARCHIVE_V1   ; dwFlags
    push eax                     ; szMpqName
    call [SFileCreateArchive]
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

; ---------------------------------------------------------------------------
; Mpq_AddFile: Добавление файла из памяти в MPQ
; Вход: EAX = hMpq, EDX = szArchivedName, ECX = pData, EBX = dataSize
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; Mpq_AddFile: Добавление файла (заглушка на этапе инициализации)
; ---------------------------------------------------------------------------
Mpq_AddFile:
    xor  eax, eax
    ret

; ---------------------------------------------------------------------------
; Mpq_RemoveFile: Удаление файла из архива
; ---------------------------------------------------------------------------
Mpq_RemoveFile:
    push 0                       ; dwSearchScope
    push edx                     ; szFileName
    push eax                     ; hMpq
    call [SFileRemoveFile]
    ret