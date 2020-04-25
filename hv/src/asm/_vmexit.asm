; 
; Stub to handle the VmExit operation
; Taken from Alex Ionescu Simplevisor, I like his approach of using RtlCaptureContext
; instead of saving the whole context manually
;

.code

extern VmExitHandler:proc
extern CaptureContext:proc

VmExitStub PROC
	push	rcx
	lea		rcx, [rsp+8]
	call    CaptureContext
	jmp     VmExitHandler
VmExitStub ENDP

VmExitRestoreCtx PROC
	ret
VmExitRestoreCtx ENDP

end