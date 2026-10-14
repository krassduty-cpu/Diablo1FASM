; ===========================================================================
; StormLib MPQ Data (data/mpq_data.inc)
; ===========================================================================

align 4
main_archive        dd 0    ; HANDLE открытого архива DIABDAT.MPQ
pref_archive        dd 0    ; HANDLE архива сохранений/настроек

; Флаги StormLib
STREAM_FLAG_READ_ONLY = 0x00000100
MPQ_OPEN_READ_ONLY    = 0x00000100
MPQ_CREATE_ARCHIVE_V1 = 0x00000000