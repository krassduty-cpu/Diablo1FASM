; ===========================================================================
; StormLib SDL RWops Stream Adapter (code/mpq/mpq_sdl_rwops.inc)
; ===========================================================================

align 4
StormStream_Size:
    mov  eax, [esp+4]            ; rwops
    mov  eax, [eax+32]           ; ctx
    test eax, eax
    jz   .err
    mov  edx, 0
    mov  eax, [eax + StormStreamCtx.fileSize]
    ret
.err:
    mov  eax, -1
    mov  edx, -1
    ret

align 4
StormStream_Seek:
    mov  ecx, [esp+4]            ; rwops
    mov  ecx, [ecx+32]           ; ctx
    mov  eax, [esp+8]            ; offset
    mov  edx, [esp+16]           ; whence (0=SET, 1=CUR, 2=END)

    cmp  edx, 0
    je   .set
    cmp  edx, 1
    je   .cur
    cmp  edx, 2
    je   .end
    jmp  .err

.set:
    mov  [ecx + StormStreamCtx.curPos], eax
    jmp  .apply
.cur:
    add  [ecx + StormStreamCtx.curPos], eax
    jmp  .apply
.end:
    mov  edx, [ecx + StormStreamCtx.fileSize]
    add  edx, eax
    mov  [ecx + StormStreamCtx.curPos], edx

.apply:
    ; SFileSetFilePointer(hFile, curPos, NULL, FILE_BEGIN=0)
    push 0
    push 0
    push dword [ecx + StormStreamCtx.curPos]
    push dword [ecx + StormStreamCtx.hFile]
    call [SFileSetFilePointer]
    mov  edx, 0
    ret
.err:
    mov  eax, -1
    mov  edx, -1
    ret

align 4
StormStream_Read:
    push ebp
    mov  ebp, esp
    push ebx esi edi
    sub  esp, 4                  ; bytesRead

    mov  eax, [ebp+8]            ; rwops
    mov  esi, [eax+32]           ; ctx
    mov  edi, [ebp+12]           ; ptr
    mov  eax, [ebp+16]           ; size
    mul  dword [ebp+20]          ; maxnum (eax = total bytes)
    mov  ebx, eax

    lea  ecx, [ebp-4]
    push 0
    push ecx                     ; &bytesRead
    push ebx                     ; total bytes
    push edi                     ; buffer
    push dword [esi + StormStreamCtx.hFile]
    call [SFileReadFile]
    test eax, eax
    jz   .fail

    mov  eax, [ebp-4]
    xor  edx, edx
    div  dword [ebp+16]          ; делим обратно на size объектов
    jmp  .done
.fail:
    xor  eax, eax
.done:
    pop  edi esi ebx
    mov  esp, ebp
    pop  ebp
    ret

align 4
StormStream_Close:
    mov  eax, [esp+4]            ; rwops
    test eax, eax
    jz   .exit
    mov  ecx, [eax+32]           ; ctx
    test ecx, ecx
    jz   .free_rw

    push dword [ecx + StormStreamCtx.hFile]
    call [SFileCloseFile]

    push ecx
    call [SDL_free]
    add  esp, 4

.free_rw:
    push dword [esp+4]
    call [SDL_FreeRW]
    add  esp, 4
.exit:
    xor  eax, eax
    ret