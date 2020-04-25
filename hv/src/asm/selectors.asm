;  
; Operations to obtain selectors
;

.code

__getcs PROC
    mov ax, cs
    ret
__getcs ENDP

__getss PROC
    mov ax, ss
    ret
__getss ENDP

__getds PROC
    mov ax, ds
    ret
__getds ENDP

__getes PROC
    mov ax, es
    ret
__getes ENDP

__getfs PROC
    mov ax, fs
    ret
__getfs ENDP

__getgs PROC
    mov ax, gs
    ret
__getgs ENDP

__getldtr PROC
    sldt ax
    ret
__getldtr ENDP

__gettr PROC
    str ax
    ret
__gettr ENDP

__sgdt PROC
    sgdt [rcx]
    ret
__sgdt ENDP

end