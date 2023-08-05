#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbc_hicolor.h>

// GBC HiColor images; header file names align with png file names
#include "example_image.h"
#include "test_pattern_tall.h"
#include "test_pattern_short.h"


#define BG_LAST_TILE  255u
const uint8_t blank_tile[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

#define ARRAY_LEN(A)  sizeof(A) / sizeof(A[0])

uint8_t buttons, buttons_prev;
#define UPDATE_BUTTONS()            (buttons_prev = buttons, buttons = joypad())
#define BUTTON_TOGGLED(BUTTON_MASK) ((buttons & (~buttons_prev)) & (BUTTON_MASK))
#define BUTTON_PRESSED(BUTTON_MASK) (buttons & (BUTTON_MASK))

// Array of pointers to the generated hicolor data structures
const hicolor_data * hicolors[] = {
    &HICOLOR_VAR(test_pattern_tall),
    &HICOLOR_VAR(example_image),
    &HICOLOR_VAR(test_pattern_short)
};


void main(void) {
    // Image toggling variable, by default show the "example_image"
    uint8_t  img_select = 0;
    bool     first_pass = true;
    uint8_t  scroll_limit = 0;
    const    hicolor_data * p_hicolor;

    SHOW_BKG;

    // Require Game Boy Color
    if (_cpu == CGB_TYPE) {
        // CGB running in the double speed mode is required
        cpu_fast();

        while(true) {

            vsync();
            UPDATE_BUTTONS();

            // Change displayed Hi Color image when pressing A or B
            if (BUTTON_TOGGLED(J_A | J_B) || first_pass) {

                // Set current image to show
                p_hicolor = hicolors[img_select];

                // Reset Y scroll and set scroll range based on converted image height
                SCY_REG = 0u;
                if ((p_hicolor->height_in_tiles * 8u) > DEVICE_SCREEN_PX_HEIGHT)
                    scroll_limit = ((p_hicolor->height_in_tiles * 8u) - DEVICE_SCREEN_PX_HEIGHT);
                else scroll_limit = 0;

                // Load and display the HiColor image
                vsync();
                DISPLAY_OFF;
                hicolor_start(p_hicolor);

                // Optional:
                // If the Hi Color image is shorter than screen height
                // then fill the remaining screen area with a tile.
                //
                // Put the tile at the end of CGB tile pattern vram since
                // the short Hi Color image will be too small to use all of it.
                if ((p_hicolor->height_in_tiles * 8u) < DEVICE_SCREEN_PX_HEIGHT) {
                    VBK_REG = VBK_BANK_1;
                    set_bkg_data(BG_LAST_TILE, 1u, blank_tile);
                    fill_bkg_rect(0u, (p_hicolor->height_in_tiles), DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, BKGF_BANK1);
                    VBK_REG = VBK_BANK_0;
                    fill_bkg_rect(0u, (p_hicolor->height_in_tiles), DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, BG_LAST_TILE);
                }

                DISPLAY_ON;

                // Cycle through which image to show next
                img_select++;
                if (img_select == ARRAY_LEN(hicolors)) img_select = 0;

                first_pass = false;
            }
            // Scroll Up/Down if available
            else if (BUTTON_PRESSED(J_UP)) {
                if (SCY_REG) SCY_REG--;
            } else if (BUTTON_PRESSED(J_DOWN)) {
                if (SCY_REG < scroll_limit) SCY_REG++;
            }
        }
    }
}

