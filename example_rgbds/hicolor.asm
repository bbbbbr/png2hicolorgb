INCLUDE "hardware.inc"
	rev_Check_hardware_inc 4.0

def NB_PALETTES_PER_LINE equ 4 ; How many palettes are transferred per scanline.
def NB_BYTES_PER_LINE equ NB_PALETTES_PER_LINE * 4 * 2 ; 4 colors/palette × 2 bytes/color

SECTION "High-color routines", ROM0

LoadHiColorPicture::
	xor a ; Switch to VRAM bank 0.
	ld [rVBK], a

	; First, write the tilemap.
	ld hl, $9C00
	call .writeScreen

	; Then, write the first tile "block".
	ld a, [de]
	ld c, a
	inc de
	ld a, [de]
	ld b, a
	inc de

	bit 4, b
	jr z, .onlyOneTileBlock
	res 4, b
	push bc
	ld hl, $8800 ; Assuming "LCDCF_BLK21" was selected.
	ld bc, 16 * 256 ; Transfer 256 tiles.
	call .copyTiles

	; Write the remainder into the second tile "block".
	pop bc
	ld a, 1 ; Switch to VRAM bank 1 for the "residual" copy.
	ld [rVBK], a
.onlyOneTileBlock
	ld hl, $8800
	call .copyTiles

	; Write the attribute map also...
	ld a, 1
	ldh [rVBK], a
	ld hl, $9C00
	call .writeScreen

	; And finally, register the pointer to the palette data.
	ld a, e
	ld [wPalettesAddr], a
	ld a, d
	ld [wPalettesAddr + 1], a

	; Enable the STAT interrupt.
	ld a, $FF
	ldh [rLYC], a ; ...but don't trigger it until the next VBlank!
	ld a, STATF_LYC
	ldh [rSTAT], a

	; Note that BCPS will only be set by the VBlank interrupt, but that's fine because the first
	; frame after turning the LCD on is not shown.
	ret


.writeScreen
	ld b, SCRN_Y_B

.nextRow
	ld c, SCRN_X_B
:
	ld a, [de]
	ld [hli], a
	inc de
	dec c
	jr nz, :-

	; Go to the next tilemap row.
	; hl += <tilemap width> - <screen width>
	ld a, l
	add SCRN_VX_B - SCRN_X_B
	ld l, a
	adc a, h
	sub l
	ld h, a

	dec b
	jr nz, .nextRow
	ret


.copyTiles
	dec bc
	inc b
	inc c
:
	ld a, [de]
	ld [hli], a
	inc de
	dec c
	jr nz, :-
	dec b
	jr nz, :-
	ret


;***********************************************
;* VBlank Interrupt routine for Hicol Software *
;***********************************************

hicolor_vbl::
	; Reset the address to which palettes will be written.
	ld a, BCPSF_AUTOINC
	ldh [rBCPS], a

	; Copy the initial palettes.
	ld hl, .ret ; Return address.
	push hl
	ld hl, wPalettesAddr
	ld a, [hli]
	ld h, [hl]
	ld l, a
	push hl ; Where to jump to.

	; Write the pointer the interrupts will resume from.
	add a, 8 * 4 * 2 * 2 + 1 ; 8 palettes × 4 colors/palette × 2 bytes/palette × `ld [hl], <byte>` + `ret`
	ld [wPaletteCurPtr], a
	ld a, [wPalettesAddr + 1]
	adc a, 0
	ld [wPaletteCurPtr + 1], a

	ld hl, rBCPD
	ret ; Jump to the palette code.
.ret
	xor a ; TODO
	ldh [rLYC], a

	pop hl
	pop af
	reti


hicolor_stat::
	ldh a, [rIE]
	ldh [$FFE0], a ; TODO: temporary
	ld a, IEF_STAT
	ldh [rIE], a
	ld a, STATF_MODE00
	ldh [rSTAT], a
	
.loop
	ld hl, .ret
	push hl

	; Read the current palette read pointer.
	ld hl, wPaletteCurPtr
	ld a, [hli]
	ld h, [hl]
	ld l, a
	push hl ; Where the `ret` will jump to.

	;ld a, [wPaletteCurPtr] (already loaded)
	add a, 1 + NB_BYTES_PER_LINE * 2 + 1
	ld [wPaletteCurPtr], a
	ld a, [wPaletteCurPtr + 1]
	adc a, 0
	ld [wPaletteCurPtr + 1], a

	ld hl, rBCPD
	ret
.ret
	xor a
	ldh [rIF], a
	ldh a, [rLY]
	cp $8F
	jr c, .loop

	ld a, STATF_LYC
	ldh [rSTAT], a
	ldh a, [$FFE0]
	ldh [rIE], a

	; Clean up and return.
	pop hl
	pop af
	reti


SECTION "Hi-Color variables", WRAM0

; Base address where the palette data lives.
wPalettesAddr: dw
; This pointer advances through the palette data (see `wPalettesAddr`) as the frame progresses.
wPaletteCurPtr: dw
