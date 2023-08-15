INCLUDE "hardware.inc"


SECTION "VBlank interrupt handler", ROM0[$40]

	push af
	push hl
	jp hicolor_vbl

SECTION "STAT interrupt handler", ROM0[$48]

	push af
	push hl
	jp hicolor_stat


SECTION "Load hi-color image", ROM0

LoadPicture:
	ld [rROMB0], a

	; Turn the screen off, as we'll be loading a *lot* of data!
	di ; TODO: bit of a bodge to suppress the VBlank interrupt
.waitVBlank
	ldh a, [rLY]
	cp 144
	jr c, .waitVBlank
	xor a
	ldh [rLCDC], a
	ldh [rIF], a
	ei

	call LoadHiColorPicture

	ld a, LCDCF_ON | LCDCF_BGON | LCDCF_BLK21 | LCDCF_BG9C00
	ldh [rLCDC], a
	ret

SECTION "Header", ROM0[$100]

	jp EntryPoint
	ds $150 - @

SECTION "Main Code", ROM0

EntryPoint:
	cp BOOTUP_A_CGB
	jp nz, CgbOnlyScreen

	; Switch to double-speed mode.
	xor a
	ldh [rIE], a
	ld a, $30
	ldh [rP1], a
	ld a, 1
	ldh [rKEY1], a
	stop

	xor a
	ldh [rSTAT], a
	ldh [rIF], a
	ld a, IEF_VBLANK | IEF_STAT
	ldh [rIE], a

	ei

	ld a, BANK(ExampleImage)
	ld de, ExampleImage
	call LoadPicture

.wait
	halt
	jr .wait

SECTION "Hi-color example image", ROMX

ExampleImage:
	incbin "obj/example_image.map"
	dw .tilesEnd - .tiles
.tiles
	incbin "obj/example_image.til"
.tilesEnd
	incbin "obj/example_image.atr"
	incbin "obj/example_image.pal"

SECTION "CGB-only screen", ROM0

CgbOnlyScreen:
	jr @ ; TODO
