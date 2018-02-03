EXTERN draw_overlay: PROC
EXTERN hud_postrender_orig: QWORD

_TEXT SEGMENT

hud_postrender_hook PROC
	push rcx
	push rbp
	mov rbp, rsp

	sub rsp, 20h
	and rsp, 0FFFFFFFFFFFFFFF0h
	call draw_overlay

	mov rsp, rbp
	pop rbp
	pop rcx

	; Overwritten instructions
	push rbp
	push rdi
	lea rbp, [rsp-4Fh]
	sub rsp, 0C8h

	mov rax, hud_postrender_orig
	add rax, 0Fh
	jmp rax
hud_postrender_hook ENDP

_TEXT ENDS

END