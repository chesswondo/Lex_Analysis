.8086
_TEXT	segment	use16
assume	cs:_TEXT, ds:_TEXT
	org	100h
	
sizeHi	equ	word ptr ss:[bp-130+1Ch]
sizeLo	equ	word ptr ss:[bp-130+1Ah]
attr	equ	byte ptr ss:[bp-130+15h]
DTAs	equ	word ptr ss:[bp-134]
DTAo	equ	word ptr ss:[bp-132]

locals	@@

extrn	N_LDIV@:near
extrn	N_LUMOD@:near
	
start:
	lea	dx,copy
	mov	ah,9
	int	21h		;Пишем Copyright
	mov	ah,0Fh
	int	10h
	mov	bl,al
	cmp	al,7
	je	MonoMode
	mov	ax,0B800h
	jmp	Params
MonoMode:
	mov	ax,0B000h
Params:
	mov	vseg,ax
	mov	ah,03h
	mov	bh,0
	int	10h
	mov	al,dh
	mov	ah,0
	mov	cl,80
	mul	cl
	cmp	bl,2
	jbe	FindParam
	shl	ax,1
FindParam:
	mov	voff,ax
	mov	cl,ds:[80h]	;символів параметрів
	cmp	cl,0
	je	NoParam		;Параметрів немає
	mov	ch,0
	mov	di,81h
	mov	al,' '
CheckParam:
	repe	scasb
	jne	IsParam
NoParam:
	lea	dx,usage
	mov	ah,9
	int	21h
	ret
IsParam:
	mov	si,di
	dec	si
	lea	di,param
	inc	cl
MoveParam:
	lodsb
	cmp	al,' '
	je	EndMoveParam
	stosb
	loop	MoveParam
EndMoveParam:
	lea	di,param
	call	addBackSlash
	lea	si,param
	call	Main
	call	wipeStr
	lea	di,total_d
	mov	cx,10
	mov	ax,total
	xor	dx,dx
first_div:
	div	cx
	add	dl,'0'
	mov	es:[di],dl
	dec	di
	xor	dx,dx
	cmp	ax,0
	jne	first_div
	lea	di,totald_d
	mov	cx,10
	mov	ax,totaldir
	xor	dx,dx
second_div:
	div	cx
	add	dl,'0'
	mov	es:[di],dl
	dec	di
	xor	dx,dx
	cmp	ax,0
	jne	second_div
	lea	di,totals_d
	mov	ax,word ptr totalsize
	mov	dx,word ptr totalsize+2
third_div:
	xor	cx,cx
	push	cx
	mov	bx,10
	push	bx
	push	dx
	push	ax
	push	cx
	push	bx
	push	dx
	push	ax
	call	N_LUMOD@
	add	al,'0'
	mov	es:[di],al
	dec	di
	call	N_LDIV@
	cmp	ax,0
	jne	third_div
	cmp	dx,0
	jne	third_div
	lea	dx,total_t
	mov	ah,9
	int	21h
	ret

wipeStr	proc	near
	push	es
	push	di
	mov	cx,80
	mov	di,voff
	mov	ax,vseg
	mov	es,ax
	mov	al,' '
@@wipe:
	stosb
	inc	di
	loop	@@wipe
	pop	di
	pop	es
	ret
wipeStr	endp

writeFile	proc	near
	push	es
	push	di
	push	si
	lea	di,file
	mov	si,ss:[bp+2]
@@StrToFile:
	lodsb
	stosb
	cmp	al,0
	jne	@@StrToFile
	sub	di,2
	call	addBackSlash
	mov	si,bp
	sub	si,130-1Eh
	dec	di
	call	StrCat
	lea	si,file
	mov	ax,vseg
	mov	es,ax
	mov	di,voff
	call	wipeStr
@@write:
	lodsb
	stosb
	inc	di
	cmp	al,0
	jne	@@write
	pop	si
	pop	di
	pop	es
	ret
writeFile	endp
	
addBackSlash	proc	near	;адреса ASCIIZ-рядка в es:di
	mov	al,0
@@CheckNext:
	scasb
	jne	@@CheckNext
	dec	di
	cmp	byte ptr es:[di-1],'\'
	je	IsBack
	mov	al,'\'
	stosb
	mov	byte ptr es:[di],0
IsBack:
	ret
addBackSlash	endp

StrCat	proc	near		;до рядка es:di додаємо ds:si
	mov	al,0
@@CheckNext:
	scasb
	jne	@@CheckNext
	dec	di
@@Perenos:
	lodsb
	stosb
	cmp	al,0
	jne	@@Perenos
	ret
StrCat	endp
	

param	db	80	dup(0)	;Мiсце для рядку параметрiв
file:
copy	db	13,10,'Directory size',13,10,'Copyright (c) KIV without Co',13,10,'$'
usage	db	'USAGE: DS <pathname>',13,10,'$'
	db	20 dup('@')
vseg	dw	0
voff	dw	0
total		dw	0
totaldir	dw	0
totalsize	dd	0
mask	db	'*.*',0
total_t		db	'Total files -         '
total_d		db	' ',10,13
totald_t	db	'Total dirs  -         '
totald_d	db	' ',10,13
totals_t	db	'Total size  -         '
totals_d	db	' ',10,13,'$'

Main	proc	near		;головна процедура - пошук в каталозi
							;рядок, де шукати - ds:si
	push	si
	push	bp
	mov	bp,sp
	sub	sp,136				;локальнi змiннi:
							;[bp-85] - рядок пошуку
							;[bp-130]- DTA
							;[bp-134]- save DTA
	mov	di,bp
	sub	di,85
@@PishemStroku:
	lodsb
	stosb
	cmp	al,0
	jne	@@PishemStroku		;рядок пошуку - в [bp-85]
	sub	di,3
	call	addBackSlash
	sub	di,2
	lea	si,mask
	call	StrCat			;робимо маску пошуку
	push	es
	mov	ah,2Fh
	int	21h
	push	es
	pop	ax
	mov	DTAs,ax
	mov	DTAo,bx
	pop	es					;Save DTA
	mov	dx,bp
	sub	dx,130
	mov	ah,1Ah
	int	21h					;Set DTA
	mov	cx,27h				;Any file attributes
	mov	dx,bp
	sub	dx,85
	mov	ah,4Eh
	int	21h					;FindFirst
	jc	@@NoFiles
@@NextFile:
	call	writeFile
	inc	total
	mov	ax,sizeLo
	add	word ptr totalsize,ax
	mov	ax,sizeHi
	adc	word ptr totalsize+2,ax
	mov	dx,bp
	sub	dx,130
	mov	ah,4Fh
	int	21h					;FindNext
	jnc	@@NextFile
@@NoFiles:
	mov	cx,10h				;directories
	mov	dx,bp
	sub	dx,85
	mov	ah,4Eh
	int	21h					;FindFirst
	jc	@@NoDirs
@@NextDir:
	test	attr,10h
	jz	@@Non_Dir
	cmp	byte ptr ss:[bp-130+1Eh],'.'
	je	@@Non_Dir
	inc	totaldir
	mov	di,bp
	sub	di,85
	mov	si,ss:[bp+2]
@@PishemStrokuDir:
	lodsb
	stosb
	cmp	al,0
	jne	@@PishemStrokuDir	;новий рядок пошуку - в [bp-85]
	sub	di,3
	call	addBackSlash
	sub	di,2
	push	si
	mov	si,bp
	sub	si,130-1Eh
	call	StrCat
	mov	si,bp
	sub	si,85
	call	Main
	pop	si
@@Non_Dir:
	mov	dx,bp
	sub	dx,130
	mov	ah,4Fh
	int	21h					;FindNext
	jnc	@@NextDir
@@NoDirs:
	push	ds
	mov	ax,DTAs
	push	ax
	pop	ds
	mov	dx,DTAo
	mov	ah,1Ah
	int	21h
	pop	ds					;Restore DTA
	mov	sp,bp
	pop	bp
	pop	si
	ret
Main	endp
_TEXT	ends
	end	start
