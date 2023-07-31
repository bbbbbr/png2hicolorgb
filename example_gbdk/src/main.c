#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbc_hicolor.h>

// GBC HiColor images; header file names align with png file names
#include "example_image.h"
#include "test_pattern.h"


inline void waitpad_lowcpu(void) {
    while(!joypad()) vsync();
}

inline void waitpad_up_lowcpu(void) {
    while(joypad()) vsync();
}


void main(void) {
    // image toggling variable, by default we show example_image
    bool toggle = true;

    SHOW_BKG;

    // Require Game Boy Color
    if (_cpu == CGB_TYPE) {
        // CGB running in the double speed mode is required
        cpu_fast();


        while(true) {
            // Load and display the HiColor image
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

