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
	ld [wHiColorPalAddr], a
	ld a, d
	ld [wHiColorPalAddr + 1], a
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

	ld [spbak], sp ;store SP.

	ld a, 1
	ld [rVBK], a

	; Point SP to the palette data.
	ld sp, wHiColorPalAddr
	pop hl
	ld sp, hl

	ld a, $80 ;
	ld hl, rBCPS ;setup palette write.
	ld [hl], a ;


	ld hl, rLY ;
	xor a

_wait:

	cp [HL] ;
	jr nz, _wait

_palloop:

	pop de ;get 2 palette values.

	ld l, rSTAT & 255 ;point HL to STAT.

_waitstat2:

	bit 1, [hl] ;wait for HBlank.
	jr nz, _waitstat2 ; ; Write 32 palette bytes (16 colors, which is 4 palettes)
fti:

	ld l, rBCPD & 255

	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de ;
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d

	pop de ;get 2 palette values.
	ld [hl], e ;
	ld [hl], d ;write 2 palette values.

	pop de
	ld [hl], e ;
	ld [hl], d


	ld a, [rLY] ; ; cp  142 ;test for bottom of image.
				cp 143 ;test for bottom of image.
	jr nz, _palloop ;

	ld hl, spbak
	ld a, [hli] ;
	ld h, [hl] ;restore SP.
	ld l, a ;
	ld sp, hl

	ret




SECTION "Hi-Color variables", WRAM0

wHiColorPalAddr: dw
spbak: dw ; TODO
