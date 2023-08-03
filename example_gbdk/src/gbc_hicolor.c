// TODO: Permissive License

#include <gbdk/platform.h>
#include <gbdk/incbin.h>
#include <gb/isr.h>

#include <gbc_hicolor.h>

#define MIN(A,B) ((A)<(B)?(A):(B))

static uint16_t SP_SAVE;
static uint8_t STAT_SAVE;
static uint8_t * p_hicolor_palettes;
static uint8_t p_hicolor_height;


// ISR function which updates 4 CGB palettes per scanline
// alternating between Palettes 0-3 and 4-7
void hicolor_palette_isr(void) NONBANKED {
__asm
        ld (_SP_SAVE), sp           ; save SP

        ld hl, #_p_hicolor_palettes ; load address of picture palettes buffer
        ld a, (hl+)
        ld d, (hl)
        ld e, a

        ldh a, (_SCY_REG)
        swap a
        ld l, a
        and #0x0f
        ld h, a
        ld a, #0xf0
        and l
        ld l, a
        add hl, hl
        add hl, de                  ; offset address by SCY * (4 * 4 * 2)
        ld sp, hl

        rlca                        ; compensate odd/even line
        and #0x20                   ; if odd then start from 4-th palette; offset == 32
        or  #0x80                   ; set auto-increment

        ld hl, #_BCPS_REG
        ld (hl+), a                 ; HL now points to BCPD

        .rept (8*4)                 ; read and set the the colors that come from previous lines
            pop de
            ld (hl), e
            ld (hl), d
        .endm

0$:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr z, 0$                    ; wait for mode 3

        ldh a, (_STAT_REG)
        ld (_STAT_SAVE), a

        ld a, #STATF_MODE00
        ldh (_STAT_REG), a
        xor a
        ldh (_IF_REG), a

1$:
        pop de                      ; preload the first two colors
        pop bc

        xor a
        ldh (_IF_REG), a
        halt                        ; wait for mode 0

        ld (hl), e                  ; set the first two colors
        ld (hl), d
        ld (hl), c
        ld (hl), b

        .rept (4*4)-2
            pop de                  ; read and set the rest of the colors
            ld (hl), e
            ld (hl), d
        .endm

        ld a, (_p_hicolor_height)
        ld c, a
        ldh a, (_LY_REG)
        cp c
        jr c, 1$                    ; load the next 4 palettes

        ld a, (_STAT_SAVE)
        ldh (_STAT_REG), a
        xor a
        ldh (_IF_REG), a

        ld sp, #_SP_SAVE
        pop hl
        ld sp, hl                   ; restore SP

        ret
__endasm;
}


// Loads Tile Patterns, Map and Map Attributes for the HiColor image,
// then installs the HiColor ISR handler which updates palettes per scanline.
//
// The intput argument should be a pointer to the struct generated by the
// png2hicolorgb program with C source output mode enabled
void hicolor_start(const hicolor_data * p_hicolor) NONBANKED {
    // prevent installing HiColor ISR twice
    CRITICAL {
        remove_LCD(hicolor_palette_isr);
    }

   if (!p_hicolor) return;

    // Copy address of palette into local var used by HiColor ISR
    p_hicolor_palettes = p_hicolor->p_palette;

    // TODO: if less than screen height, then converter must emit tail palettes and the cutting scanline must be moved accordingly
    p_hicolor_height = (p_hicolor->height_in_tiles > DEVICE_SCREEN_HEIGHT) ? (DEVICE_SCREEN_PX_HEIGHT - 1) : ((p_hicolor->height_in_tiles << 3) - 1);

    // Load the first 256 tiles or less and set BG Map
    VBK_REG = VBK_BANK_0;
    set_bkg_data(0u, MIN(p_hicolor->tile_count, 256), p_hicolor->p_tiles);
    set_bkg_tiles(0u, 0u, DEVICE_SCREEN_WIDTH, p_hicolor->height_in_tiles, p_hicolor->p_map);

    // Load remaining 256 tiles and set Attribute Map into alternate bank
    VBK_REG = VBK_BANK_1;
    if (p_hicolor->tile_count > 256) set_bkg_data(0u, (p_hicolor->tile_count - 256), p_hicolor->p_tiles + (256 * 16));
    set_bkg_tiles(0, 0, DEVICE_SCREEN_WIDTH, p_hicolor->height_in_tiles, p_hicolor->p_attribute_map);
    VBK_REG = VBK_BANK_0;

    // Set up and install the HiColor ISR
    CRITICAL {
        LYC_REG = 152;
        STAT_REG = STATF_LYC;
        // install the HiColor ISR
        add_LCD(hicolor_palette_isr);
    }
    set_interrupts(IE_REG | LCD_IFLAG);
}
