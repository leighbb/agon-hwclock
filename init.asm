; SPDX-License-Identifier: GPL-2.0-or-later
;
;  init.asm
;
;  Copyright (C) 2023  Leigh Brown
;

		segment code

		xref	__low_bss
		xref	__len_bss

		xref	_main

		.ASSUME	ADL = 1	

argv_ptrs_max:	equ	16			; Size of argv array

		JP	_start			; Jump to start

; MOS Header
_exec_name:	db	"hwclock",0		; Executable name (argv[0])
		align	64			; Magic number starts at 0x40

		db	"MOS"			; Flag for MOS
		db	00h			; MOS header version 0
		db	01h			; Run in ADL mode (1)

; Let's go
_start:		; Save registers for returning to MOS, including MB
		push	af
		push	bc
		push	de
		push	ix
		push	iy
		ld	a,mb
		push 	af

		; Clear MB for MOS API
		xor 	a
		ld 	mb,a

		; Clear BSS, saving HL which contains address of argument string
		push	hl
		call	clear_bss
		pop	hl

		; Push argv onto stack, then parse parameters into it
		ld	ix,argv_ptrs
		push	ix
		call	parse_params		; Parse arguments into argv

		; Push Parameter count returned from parse_params onto stack
		ld	b,0
		push	bc

		; int main(const int argc, const char *argv[])
		call	_main

		; Remove pushed parameters
		pop	de
		pop	de

		; Restore saved registers including MB
		pop	af
		ld	mb,a
		pop	iy
		pop	ix
		pop	de
		pop 	bc
		pop	af
		ret

; Clear BSS (uninitialised variables)
;
; Clever routine (not mine) that uses LDIR to clear memory
		
clear_bss:	; Do nothing if the length of the BSS is zero
		ld	bc,__len_bss
		ld	a,__len_bss >> 16
		or	a,c
		or	a,b
		ret	z

		; It's at least one byte long - set the first byte
		xor	a,a
		ld 	(__low_bss),a

		; Decrement BC and if zero, we are done
		sbc	hl,hl
		dec	bc
		sbc	hl,bc
		ret	z		  	; Just 1 byte ...

		; Use LDIR to propogate the first byte to the end
		ld	hl,__low_bss
		ld	de,__low_bss + 1
		ldir
		ret
		
; Parse the command line following the command and split it into the elements
; of the argv array, returning the length of the array
; Inputs:
; HL: Address of parameter string
; IX: Address for array pointer storage
; Returns:
;  C: Number of parameters parsed
;
parse_params:
	; Save the name of the program as argv[0], then move IX to next element
	ld	bc,_exec_name
	ld	(ix+0),bc
	lea	ix,ix+3
	
	; Use register D for '\r' and register E for ' '
	ld	d,13
	ld	e,' '

	; Count of arguments (start at 1 with argv0 stored)
	ld	bc,1
	ld	b,argv_ptrs_max

	; Check we haven't filled the array
loop:	ld	a,c
	cp	a,b
	ret	z

	; Skip past any whitespace, or exit if no more parameters
	ld	a,(hl)
	or	a,a
	ret	z
	cp	a,d	; '\r'
	ret	z
	cp	a,e	; ' '
	jr	nz,got1
	inc	hl
	jr	loop

	; We have found a parameter, save its address and increment argc
got1:	ld	(ix+0),hl
	lea	ix,ix+3
	inc	c

	; Find the end of the token
scan:	inc	hl
	ld	a,(hl)
	or	a,a
	ret	z
	cp	a,d	; '\r' (BBC BASIC terminates parameters with '\r')
	jr	z,t_done
	cp	a,e	; ' '
	jr	nz,scan

	; found a separator - zero terminate the parameter then continue
term:	xor	a,a
	ld	(hl),a
	inc	hl
	jr	loop

	; For BBC BASIC - zero terminate the last parameter then exit
t_done:	xor	a,a
	ld	(hl),a	
	ret

	segment data

; Storage for the argv array pointers
;
argv_ptrs:
	blkp	argv_ptrs_max, 0

	end
