; Gameboy project
;
; Glen Cook

	INCLUDE "hardware.inc"
	rev_Check_hardware_inc 1.5

    INCLUDE "memory1.s"
	rev_Check_memory1_asm 1.0

	INCLUDE "defines.inc"



DELAY_VALUE	equ	$04


SECTION "V Blank",HOME[$40]

	call		hicolor_vbl
	reti

SECTION "LCDC Status Interupt",HOME[$48]

	reti

SECTION "Timer Interupt",HOME[$50]

	reti

SECTION "Serial Interupt",HOME[$58]

	reti

SECTION "Keypad Interupt",HOME[$60]

	reti

SECTION "Org $100",HOME[$100]

	nop
	jp      begin

	NINTENDO_LOGO                   	; Nintendo graphic logo

;Rom Header Info
;			 123456789abcdef
	DB		"GB HiCol"				; Game Name
	DB		0,0,0,0,0,0,0			; Padding to 15 characters
	DB		$c0						; 0 - MGB / $80 - Color compatible /
	DB		0,0						; Game Maker Code
	DB		0                   	; Game Unit Code
	DB		CART_ROM_MBC1_RAM_BAT   ; Cart type
	DB		CART_ROM_32K            ; CART_ROM_512K           ; ROM Size (in bits)
	DB		CART_RAM_NONE           ; RAM Size (in bits)
	DB		1,1                     ; Maker ID
	DB 		0                       ; Version=0
	DB 		$e2                     ; Complement check (important)
	DW 		$c40e                   ; Checksum (not important)


	SECTION "Main Code",HOME

begin:

	di
	and		a   				; 1:DMG , FF:MGB , 11:CGB
	cp		$11
	ld		a,0   				; Not CGB(0) (XOR A impossible since Zero-Flag changes)
	jr		NZ,init_NotColor
	inc		a   				; We have a Color Gameboy

init_NotColor:

	ld		[CGBFlag],a


SoftReset:

	call	Initialise			; Set up the gameboy

	call	DoubleSpeed

	ld		a,1
	call	DisplayPicture

.wait

	call	ReadJoystick
	ld		a,[_PadDebounce]
	or		a
	jr		z,.wait


	jr		.wait


DisplayPicture:

	ld		[$2000],a			; Switch to relevant bank

	call	ScreenOff			; Turn off the screen so we can write characters

	xor		a					; Switch to character bank 0
	ld		[rVBK],a

	ld		de,$8800			; Address for BG characters
	ld		bc, 16 * 256		; Transfer 256 characters

	ld		a,[$4000]			; Get address of where characters stored
	ld		l,a
	ld		a,[$4001]

	ld		h,a
	call	TransferCharData	; Transfer characters

	ld		a,1					; Switch to alternate character bank
	ld		[rVBK],a

	ld		de,$8800
	ld		bc, 16 * 104		; Transfer 104 characters

	call	TransferCharData

	ld		hl,$9800			; Screen Address (attributes)
	ld		a,[$4002]
	ld		c,a
	ld		a,[$4003]
	ld		b,a					; BC = Attribute memory

	call	WriteScreen			; Transfer character attributesWriteScreen:

	xor		a
	ld		[rVBK],a

	ld		hl,$9800
	ld		a,[$4004]
	ld		c,a
	ld		a,[$4005]
	ld		b,a					; BC = Map memory

	call	WriteScreen

	ld		a, LCDCF_ON | LCDCF_OBJON | LCDCF_BGON | LCDCF_WIN9800                ; LCD Controller = On
	call	ScreenOnVB

	ret









DoubleSpeed::

	di
	ld		hl,rIE
	ld		a,[hl]
	push	af

	xor		a
	ld		[hl],a
	ld		[rIF],a

	ld		a,$30
	ld		[rP1],a

	ld		a,1
	ld		[rKEY1],a

	stop

	pop		af
	ld		[hl],a
	ei
	ret


Initialise:

	di

init_WaitVBL:

	ldh		a,[rLY]					; $ff44=LCDC Y-Pos
	cp		$90						; $90 and bigger = in VBL
	jr		c,init_WaitVBL			; Loop until it is $90 or >

	ld		a,LCDCF_OFF
	ld		[rLCDC],a               ; Turn screen off for memory copy

	xor		a						; Clear all of RAM
	ld		hl,$c000
	ld		bc,$2000
	call	mem_Set

	xor		a
	ld		[rIF],a				; Interrupt request RESET
	ld		[rSTAT],A			; LCDC status information RESET
	ld		[rSCX],a			; Scroll X register RESET
	ld		[rSCY],a			; Scroll Y register RESET
	ld		[rWY],a
	ld		a,7
	ld		[rWX],a

; Color Specific Reset Code

	xor		a
	ld		[rVBK],a			; VramBank=0
	ld		[rSVBK],a			; WorkRamBank=0
	ld		[rRP],a				; Infrared communication port (CLEAR to save power

	inc		a
	ld		[rVBK],a			; Switch to VRAM Bank 1

	xor		a					; Clear all of VRAM (Bank 1)
	ld		hl,$8000
	ld		bc,$2000
	call	mem_Set

	xor		a
	ld		[rVBK],a			; Switch to VRAM Bank 0

	ld		hl,$8000
	ld		bc,$2000
	call	mem_Set

	ld		hl, _OAMRAM                     ; Sprite Attribute Area
	ld		a,$a0				; Number of bytes to clear

init_ClearSprites:

	ld		[hl],0            		; Transfer byte to VRAM
	inc		hl

	dec		a                  		; decrement counter
	or		a                  		; or with c. Check to see if both are zero
	jr		nz, init_ClearSprites

	ld		hl, OAM_LOCATION
	ld		a, $a0


init_ClearSpriteMem:

	ld		[hl],0		   	; Transfer byte to VRAM
	inc		hl

	dec		a				; decrement counter
	or		a				; or with c. Check to see if both are zero
	jr		nz, init_ClearSpriteMem

; Copy the sprite routine into high ram

	ld		hl,SPRITE_ROUTINE
	ld		de,SpriteRoutine
	ld		b,SpriteRoutineEnd-SpriteRoutine

init_RoutineLoop:

	ld		a,[de]
	ld		[hl],a			; Transfer byte to SPRITE_ROUTINE

	inc		de
	inc		hl

	ld		a,b
	dec		a				; decrement counter
	or		a				; or with a. Check to see if both are zero
	ld		b, a

	jr		nz, init_RoutineLoop		; If not zero repeat

	ei
	ret

; Routine that is transfered to SPRITE_ROUTINE

SpriteRoutine:

	push	af
	push	hl

	ld		hl,OAM_LOCATION
	ld		a,h
	ldh		[rDMA], a

	ld		a, $28

Wait:

	dec		a
	jr		nz, Wait

	pop		hl
	pop		af

	ret

SpriteRoutineEnd:










; ********************************************************
; Transfers CharData to VRAM
; Load DE with the VRAM address
; Load HL with address of the char data
; Load BC with the bytes to transfer
; ********************************************************

TransferCharData::

	push	af

CharLoop:

	ld		a,[hli]            ; Load char data
	ld		[de], a
	inc		de                 ; increment memory location
	dec		bc                 ; decrement counter
	ld		a, b               ; transfer upper byte of counter to a
	or		c                  ; or with c. Check to see if both are zero
	jr		nz, CharLoop       ; If not zero repeat

	pop  	af

	ret



; ********************************************************
; Turn Screen off
; Load A with value for 0xff41
; ********************************************************

ScreenOff::

	push	bc
	ld		b,a
	ld		a,[rLCDC]
	bit		7,a
	jr		z,.scfin

	di

.ScOff:

	ld   	a, [rLY]
	cp   	$90
	jr   	nz,.ScOff

	xor		a
	ld   	[rLCDC], a
	ei

.scfin

	ld		a,b
	pop		bc

	ret


VBLANK:

	push	af
	push	bc
	push	de
	push	hl


	pop		hl
	pop		de
	pop		bc
	pop		af
	reti


; ********************************************************
; Turn Screen on with Vertical Blanking interrupts
; Load A with value for 0xff41
; ********************************************************

ScreenOnVB::

	push	af

	ld		[rLCDC],a

.loop

	ld		a,[rLY]
	cp		144
	jr		c,.loop


	ld		a,[rSTAT]
	or		STATF_BUSY
	ld		[rSTAT], a

	ld		a,[rIF]
	or		IEF_VBLANK
	ld		[rIF], a

	ld		a,IEF_VBLANK|IEF_TIMER
	ld		[rIE], a

	pop		af

	ret


WriteScreen:

	ld		a,18

.yloop

	ld		e,20
	push	af

.xloop

	ld		a,[bc]
	inc		bc
	ld		[hli],a
	dec		e
	jr		nz,.xloop
	ld		de,12
	add		hl,de
	pop		af
	dec		a
	jr		nz,.yloop
	ret




ReadJoystick:

	ld		a,P1F_5
	ld		[rP1],a        ;turn on P15

	ld		a,[rP1]        ;delay
	ld		a,[rP1]
	ld		a,[rP1]
	ld		a,[rP1]
	cpl
	and     $0f
	swap	a
	ld		b,a

	ld      a,P1F_4
	ld      [rP1],a     ;turn on P14
	ld      a,[rP1]     ;delay
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	ld      a,[rP1]
	cpl
	and     $0f
	or		b
	ld      b,a

	ld		a,[_Pad]
	xor		b
	and		b
	ld		[_PadDebounce],a
	ld		a,b
	ld		[_Pad],a

	ld		a,P1F_5|P1F_4
	ld      [rP1],a

	ret


;***********************************************
;* VBlank Interrupt routine for Hicol Software *
;***********************************************

hicolor_vbl:

	ld		[spbak],sp			;store SP.

	ld		a,1
	ld		[rVBK],a

	ld		hl,GBPalette1		;point SP to palette data.
	ld		sp,hl				;

	ld		a,$80				;
	ld		hl,rBCPS			;setup palette write.
	ld		[hl],a				;


	ld		hl,rLY         		;
	xor		a

_wait:

	cp		[HL] 				;
	jr		nz,_wait

_palloop:

	pop		de            		;get 2 palette values.

	ld		l,rSTAT & 255		;point HL to STAT.

_waitstat2:

	bit		1,[hl]				;wait for HBlank.
	jr		nz,_waitstat2 		;

                                ; Write 32 palette bytes (16 colors, which is 4 palettes)
fti:

	ld		l,rBCPD & 255

	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de					;
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d

	pop		de            		;get 2 palette values.
	ld		[hl],e        		;
	ld		[hl],d        		;write 2 palette values.

	pop		de
	ld		[hl],e				;
	ld		[hl],d


	ld		a,[rLY]        		;
;	cp		142					;test for bottom of image.
    cp      143                 ;test for bottom of image.
	jr		nz,_palloop			;

	ld		hl,spbak
	ld		a,[hli]       		;
	ld		h,[hl]				;restore SP.
	ld		l,a					;
	ld		sp,hl

	ret




SECTION "Work Ram",BSS

Sprite_oam	 			ds	$A0			; Sprite Data Area ** THIS VARIABLE CAN NOT BE MOVED **
CGBFlag		 			db				; Is it a Color gameboy?
_PadData				db
_PadDataEdge			db
_Pad					db
_PadDebounce			db
spbak					dw



section "Data Rom Bank",DATA,BANK[1]

	dw	GBCharacters1
	dw	GBAttributes1
	dw	GBMap1
	dw	GBPalette1

GBCharacters1:

        incbin  "obj/example_image.til" ; 20 x 18 x 16 = 5760

GBAttributes1:						; 20 x 18 = 360

        incbin  "obj/example_image.atr"

GBMap1:								; 20 x 18 = 360

        incbin  "obj/example_image.map"

GBPalette1:

        incbin  "obj/example_image.pal" ; 72 * 8 * 4 * 2 = 4608




