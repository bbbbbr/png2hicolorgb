#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbc_hicolor.h>

// GBC HiColor images, header file names align with png file names
#include "example_image.h"
#include "test_pattern.h"


void waitpad_lowcpu(void) {
    uint8_t buttons = joypad();
    while(!buttons) {
        vsync();
        buttons = joypad();
    }
}

void waitpad_up_lowcpu(void) {
    uint8_t buttons = joypad();
    while(buttons) {
        vsync();
        buttons = joypad();
    }
}


_Noreturn void main(void) {

    bool toggle = true;

    SHOW_BKG;
    SHOW_SPRITES;

    // Reqiore Game Boy COlor
    if (_cpu == CGB_TYPE) {
        // CGB running in 2x mode is required
        cpu_fast();


        while(TRUE) {
            // Load and display a HiColor image
            vsync();
            DISPLAY_OFF;
            if (toggle)
                hicolor_start(&HICOLOR_VAR(example_image));
            else
                hicolor_start(&HICOLOR_VAR(test_pattern));
            DISPLAY_ON;

            waitpad_lowcpu();
            waitpad_up_lowcpu();

            // Change which image to show
            toggle = !toggle;
        }
    }
}

