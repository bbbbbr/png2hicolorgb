INCLUDE "hardware.inc"
	rev_Check_hardware_inc 4.0

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
	ld [wPaletteCurPtr], a
	ld a, d
	ld [wPalettesAddr + 1], a
	ld [wPaletteCurPtr + 1], a

	; Enable the STAT interrupt.
	ld a, STATF_MODE10
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
	ld a, BCPSF_AUTOINC
	ldh [rBCPS], a
	; Perform one copy, as two are needed before the beginning of the frame,
	; but the queued interrupt will only perform one.
	ld a, [wPalettesAddr]
	ld l, a
	ld a, [wPalettesAddr + 1]
	ld h, a
	ld bc, 16 << 8 | LOW(rBCPD)
.copy
	ld a, [hli]
	ldh [c], a
	ld a, [hli]
	ldh [c], a
	dec b
	jr nz, .copy

	; Write the pointer the interrupts will resume from.
	ld a, l
	ld [wPaletteCurPtr], a
	ld a, h
	ld [wPaletteCurPtr + 1], a

	; Note that a Mode 2 interrupt is queued at the beginning of scanline $90!
	; It will copy the second half of the palettes in time for the first scanline.
	pop hl
	pop bc
	pop af
	reti


hicolor_stat::
	ld [wSPBackup], sp ; Save SP so we can return to normalcy later.

	; Point SP to the palette data.
	ld sp, wPaletteCurPtr
	pop hl
	ld sp, hl

	pop de ; Read one color.
	pop bc ; Read a second one.

	ld hl, rSTAT
.waitBlank
	bit 1, [hl] ; Wait for HBlank *or* VBlank.
	jr nz, .waitBlank ; ; Write 32 palette bytes (16 colors, which is 4 palettes)

	ld l, LOW(rBCPD) ; hl = rBCPD
	; Write the first color.
	ld [hl], e
	ld [hl], d
	; Write the second color.
	ld [hl], c
	ld [hl], b
	; Write the rest.
REPT 16 - 2 ; 16 colors, minus the two already written above.
	pop de ; One color is two bytes.
	ld [hl], e
	ld [hl], d
ENDR

	; Save the read pointer for the next scanline.
	ld [wPaletteCurPtr], sp

	; Restore SP.
	ld sp, wSPBackup
	pop hl
	ld sp, hl

	; Clean up and return.
	pop hl
	pop de
	pop bc
	pop af
	reti


SECTION "Hi-Color variables", WRAM0

; Temporary storage for SP while the STAT handler is running.
wSPBackup: dw

; Base address where the palette data lives.
wPalettesAddr: dw
; This pointer advances through the palette data (see `wPalettesAddr`) as the frame progresses.
wPaletteCurPtr: dw
