; SPDX-License-Identifier: MIT
;
;  mos-interface.asm
;
;  Copyright (C) 2023  Various
;

	.include "mos_api.inc"

	XDEF _putch
	XDEF _getch
	XDEF _mos_write
	XDEF _waitvblank
	XDEF _mos_fopen
	XDEF _mos_fclose
	XDEF _mos_fgetc
	XDEF _mos_fread
	XDEF _mos_fwrite
	XDEF _mos_flseek
	XDEF _mos_feof
	XDEF _mos_save
	XDEF _mos_del
	XDEF _mos_ren
	XDEF _mos_getrtc
	XDEF _mos_setrtc
	XDEF _mos_sysvars
	XDEF _getsysvar_cursorX
	XDEF _getsysvar_cursorY
	XDEF _getsysvar_scrchar
	XDEF _getsysvar_scrwidth
	XDEF _getsysvar_scrheight
	XDEF _getsysvar_time
	
	segment	CODE
	.assume	ADL=1
	
_putch:
	push	ix
	ld	ix,0
	add	ix,sp
	ld	a,(ix+6)
	rst.lil	10h
	pop	ix
	ret

_getch:
	push	ix
	ld	a,mos_sysvars			; MOS Call for mos_sysvars
	rst.lil	08h					; returns pointer to sysvars in ixu
_getch0:
	ld	a,(ix+sysvar_keycode)	; get current keycode
	or	a,a
	jr	z,_getch0				; wait for keypress
	
	push	af						; debounce key, reload keycode with 0
	xor	a
	ld	(ix+sysvar_keycode),a
	pop	af
	pop	ix
	ret

_mos_write:
	push	ix
	ld	ix,0
	add	ix,sp
	ld	hl,(ix+6)
	ld	bc,(ix+9)
	rst.lil	18h
	pop	ix
	ret

_waitvblank:
	push	ix
	ld	a,mos_sysvars
	rst.lil	08h
	ld	a,(ix + sysvar_time + 0)
$$:	cp	a,(ix + sysvar_time + 0)
	jr	z,$B
	pop	ix
	ret


_getsysvar_time:
	push	ix
	ld	a,mos_sysvars
	rst.lil	08h
	ld	hl,(ix+sysvar_time)
	ld	e,(ix+sysvar_time+3)
	pop	ix
	ret
	
_getsysvar_cursorX:
	push	ix
	ld	a,mos_sysvars			; MOS Call for mos_sysvars
	rst.lil	08h					; returns pointer to sysvars in ixu
	ld	hl,0
	ld	l,(ix+sysvar_cursorX)	; get current cursor x position
	pop	ix
	ret

_getsysvar_cursorY:
	push	ix
	ld	a,mos_sysvars			; MOS Call for mos_sysvars
	rst.lil	08h					; returns pointer to sysvars in ixu
	ld	hl,0
	ld	l,(ix+sysvar_cursorY)	; get current cursor y position
	pop	ix
	ret

_getsysvar_scrchar:
	push	ix
	ld	a,mos_sysvars			; MOS Call for mos_sysvars
	rst.lil	08h				; returns pointer to sysvars in ixu
	ld	a, (ix+sysvar_scrchar)	; get current keycode
	pop	ix
	ret

_getsysvar_scrwidth:
	push	ix
	ld	a,mos_sysvars
	rst.lil	08h
	ld	hl,0
	ld	l,(ix+sysvar_scrwidth)
	ld	h,(ix+sysvar_scrwidth+1)
	pop	ix
	ret

_getsysvar_scrheight:
	push	ix
	ld	a,mos_sysvars
	rst.lil	08h
	ld	hl,0
	ld	l,(ix+sysvar_scrheight)
	ld	h,(ix+sysvar_scrheight+1)
	pop	ix
	ret

_mos_fopen:
	push	ix
	ld	ix,0
	add	ix,sp
	
	ld	hl,(ix+6)	; address to 0-terminated filename in memory
	ld	c,(ix+9)	; mode : fa_read / fa_write etc
	ld	a,mos_fopen
	rst.lil	08h		; returns filehandle in A
	
	ld	sp,ix
	pop	ix
	ret	

_mos_fclose:
	push	ix
	ld	ix,0
	add	ix,sp
	
	ld	c,(ix+6)	; filehandle, or 0 to close all files
	ld	a,mos_fclose
	rst.lil	08h		; returns number of files still open in A
	
	ld	sp,ix
	pop	ix
	ret	

_mos_fgetc:
	push	ix
	ld	ix,0
	add	ix,sp
	
	ld	c,(ix+6)	; filehandle
	ld	a,mos_fgetc
	rst.lil	08h		; returns character in A
	
	ld	sp,ix
	pop	ix
	ret	

_mos_fputc:
	push	ix
	ld	ix,0
	add	ix,sp
	
	ld	c,(ix+6)	; filehandle
	ld	b,(ix+9)	; character to write
	ld	a,mos_fputc
	rst.lil	08h		; returns nothing
	
	ld	sp,ix
	pop	ix
	ret	

_mos_fread:
	push	ix
	ld	ix,0
	add	ix,sp

	ld	c,(ix+6)	; UINT8 filehandle
	ld	hl,(ix+9)	; UINT24 buffer
	ld	de,(ix+12)	; UINT24 bytes to read

	ld	a,mos_fread
	rst.lil	08h

	; Move 24-bit result from DE into HL
	push	de
	pop	hl

	; Return
	ld	sp,ix
	pop	ix
	ret

_mos_fwrite:
	push	ix
	ld	ix,0
	add	ix,sp

	ld	c,(ix+6)	; UINT8 filehandle
	ld	hl,(ix+9)	; UINT24 buffer
	ld	de,(ix+12)	; UINT24 bytes to write

	ld	a,mos_fwrite
	rst.lil	08h

	; Move 24-bit result from DE into HL
	push	de
	pop	hl

	; Return
	ld	sp,ix
	pop	ix
	ret

_mos_flseek:
	push	ix
	ld	ix,0
	add	ix,sp

	ld	c,(ix+6)	; UINT8 filehandle
	ld	hl,(ix+9)	; DWORD buffer (low three bytes)
	ld	e,(ix+12)	; DWORD buffer (top byte)

	ld	a,mos_flseek
	rst.lil	08h

	; Return
	ld	sp,ix
	pop	ix
	ret

_mos_feof:
	push	ix
	ld	ix,0
	add	ix, sp
	
	ld	c, (ix+6)	; filehandle
	ld	a, mos_feof
	rst.lil	08h		; returns A: 1 at End-of-File, 0 otherwise
	
	ld	sp,ix
	pop	ix
	ret	


_mos_del:
	push	ix
	ld	ix,0
	add 	ix, sp

	ld	hl, (ix+6)	; filename address (zero terminated)
	ld	a,	mos_del
	rst.lil	08h			; save file to SD card

	ld	sp,ix
	pop	ix
	ret
	
_mos_ren:
	push	ix
	ld	ix,0
	add 	ix, sp

	ld	hl, (ix+6)	; filename1 address (zero terminated)
	ld	de, (ix+9)	; filename2 address (zero terminated)
	ld	a,	mos_ren
	rst.lil	08h			; save file to SD card

	ld	sp,ix
	pop	ix
	ret
	_mos_save:
	push	ix
	ld		ix,0
	add 	ix, sp

	ld		hl, (ix+6)	; filename address (zero terminated)
	ld	de, (ix+9)	; address to save from
	ld	bc, (ix+12)	; number of bytes to save
	ld	a,	mos_save
	rst.lil	08h			; save file to SD card

	ld	sp,ix
	pop	ix
	ret

_mos_getrtc:
	push	ix
	ld	ix,0
	add	ix,sp

	ld	hl,(ix+6)
	ld	a,mos_getrtc
	rst.lil	08h			; In: HLU; Out: A

	; convert unsigned char to int
	or	a
	sbc	hl,hl
	ld	l,a

	ld	sp,ix
	pop	ix
	ret

_mos_setrtc:
	push	ix
	ld	ix,0
	add	ix,sp

	ld	hl,(ix+6)
	ld	a,mos_setrtc
	rst.lil	08h			; In HLU; Out: (nothing)

	ld	sp,ix
	pop	ix
	ret

_mos_sysvars:
	push	ix
	ld	a,mos_sysvars
	rst.lil	08h
	lea	hl,ix+0
	pop	ix
	ret

	end
