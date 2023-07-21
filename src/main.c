// See LICENSE  file for license details

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "logging.h"
#include "image_load.h"

#include <hicolour.h>


#define VERSION "version 1.4.0"

void static display_help(void);
int handle_args(int argc, char * argv[]);
static bool matches_extension(char *, char *);
static void init(void);
void cleanup(void);

image_data src_image;
char filename_in[MAX_STR_LEN] = {'\0'};
int  show_help_and_exit = false;


static void init(void) {
    // Handle global init
    src_image.p_img_data = NULL;
    hicolor_init();
}


// Registered as an atexit handler
void cleanup(void) {
    // Handle any cleanup on exit
    // Free the image data
    if (src_image.p_img_data != NULL)
        free(src_image.p_img_data);

}


int main( int argc, char *argv[] )  {

    int ret = EXIT_FAILURE; // Default to failure on exit

    // Register cleanup with exit handler
    atexit(cleanup);

    init();

    #ifdef DRAG_AND_DROP_MODE
        // Non-interactive mode, set some reasonable default
        set_drag_and_drop_mode_defaults();
    #endif

    if (handle_args(argc, argv)) {

        if (show_help_and_exit) {
            ret = EXIT_SUCCESS;
        }
        else {
            // detect file extension
            if (matches_extension(filename_in, (char *)".png")) {

                // Load source image (from first argument)
                if (image_load(&src_image, filename_in, IMG_TYPE_PNG)) {

                    // TODO: strip extension off filename, etc
                    // hicolor_process_image(&src_image, filename_in);
                    hicolor_process_image(&src_image, "test");
                    ret = EXIT_SUCCESS; // Exit with success
                }
            }
        }
    }

    // Override exit code if was set during processing
    if (get_exit_error())
        ret = EXIT_FAILURE;

    if (ret == EXIT_FAILURE)
        log_error("Error: Unable to process image file %s\n", filename_in);

    #ifdef DRAG_AND_DROP_MODE
        // Wait for input to keep the console window open after processing
        log_standard("\n\nPress Any Key to Continue\n");
        getchar();
    #endif

    return ret;
}


static void display_help(void) {
    log_standard(
        "png2hicolorgb input_image.png [options]\n"
        VERSION", by bbbbbr\n"
        "Convert an image to Game Boy Hi-Color format\n"
        "\n"
        "Options\n"
        "-h   : Show this help\n"
        // "      shows [Region]_[Max Used Bank] / [auto-sized Max Bank Num]\n"
        // "-F   : Force Max ROM and SRAM bank num for -B. (0 based) -F:ROM:SRAM (ex: -F:255:15)\n"
        // "-m   : Manually specify an Area -m:NAME:HEXADDR:HEXLENGTH\n"
        "-v*  : Set log level: \"-vV\" verbose, \"-vQ\" quiet, \"-vE\" only errors\n"
        "-a:N  : Set conversion algorithm where N is one of below \n"
        "        0:Original Method (J.Frohwein)",
        "        1:Median Cut - No Dither",
        "        2:Median Cut - With Dither",
        "        3:Wu Quantiser"
        "\n"
        "-L:N  : Set Left  screen conversion pattern where N is entry from list below \n"
        "-R:N  : Set Right screen conversion pattern where N is entry from list below \n"
        "\n"
        "Example 1: \"png2hicolorgb myimage.png\"\n"
        // "Example 1: \"png2hicolorgb myimage.png -o:outputfilename -n:somevarname\"\n"
        "\n"
        "Based on hicolour.exe. Historical info:\n"
        "   Original Concept : Icarus Productions\n"
        "   Original Code : Jeff Frohwein\n"
        "   Full Screen Modification : Anon\n"
        "   Adaptive Code : Glen Cook\n"
        "   Windows Interface : Glen Cook\n"
        "   Additional Windows Programming : Rob Jones\n"
        "   Original Quantiser Code : Benny\n"
        "   Quantiser Conversion : Glen Cook\n"
        "\n"
        "Available Conversion Patterns for -L:N and -R:N:\n"
        "    0: Adaptive-1, 1: Adaptive-2, 2: Adaptive-3, 3: 3-2-3-2,    4: 2-3-2-3 \n"
        "    5: 2-2-3-3,    6: 2-3-3-2,    7: 3-2-2-3,    8: 3-3-2-2,    9: 4-2-2-2 \n"
        "   10: 2-2-2-4,   11: 2-2-4-2,   12: 2-4-2-2,   13: 1-1-2-6,   14: 1-1-3-5 \n"
        "   15: 1-1-4-4,   16: 1-1-5-3,   17: 1-1-6-2,   18: 1-2-1-6,   19: 1-2-2-5 \n"
        "   20: 1-2-3-4,   21: 1-2-4-3,   22: 1-2-5-2,   23: 1-2-6-1,   24: 1-3-1-5 \n"
        "   25: 1-3-2-4,   26: 1-3-3-3,   27: 1-3-4-2,   28: 1-3-5-1,   29: 1-4-1-4 \n"
        "   30: 1-4-2-3,   31: 1-4-3-2,   32: 1-4-4-1,   33: 1-5-1-3,   34: 1-5-2-2 \n"
        "   35: 1-5-3-1,   36: 1-6-1-2,   37: 1-6-2-1,   38: 2-1-1-6,   39: 2-1-2-5 \n"
        "   40: 2-1-3-4,   41: 2,1,4,3,   42: 2-1-5-2,   43: 2-1-6-1,   44: 2-2-1-5 \n"
        "   45: 2-2-5-1,   46: 2-3-1-4,   47: 2-3-4-1,   48: 2-4-1-3,   49: 2-4-3-1 \n"
        "   50: 2-5-1-2,   51: 2-5-2-1,   52: 2-6-1-1,   53: 3-1-1-5,   54: 3-1-2-4 \n"
        "   55: 3-1-3-3,   56: 3-1-4-2,   57: 3-1-5-1,   58: 3-2-1-4,   59: 3-2-4-1 \n"
        "   60: 3-3-1-3,   61: 3-3-3-1,   62: 3-4-1-2,   63: 3-4-2-1,   64: 3-5-1-1 \n"
        "   65: 4-1-1-4,   66: 4-1-2-3,   67: 4-1-3-2,   68: 4-1-4-1,   69: 4-2-1-3 \n"
        "   70: 4-2-3-1,   71: 4-3-1-2,   72: 4-3-2-1,   73: 4-4-1-1,   74: 5-1-1-3 \n"
        "   75: 5-1-2-2,   76: 5-1-3-1,   77: 5-2-1-2,   78: 5-2-2-1,   79: 5-3-1-1 \n"
        "   80: 6-1-1-2,   81: 6-1-2-1,   82: 6-2-1-1                               \n"
   );
}


// Default options for Windows Drag and Drop recipient mode
void set_drag_and_drop_mode_defaults(void) {

    // Set some options here
}


int handle_args(int argc, char * argv[]) {

    int i;

    if( argc < 2 ) {
        display_help();
        return false;
    }

    // Copy input filename (if not preceded with option dash)
    if (argv[1][0] != '-')
        snprintf(filename_in, sizeof(filename_in), "%s", argv[1]);

    // Start at first optional argument, argc is zero based
    for (i = 1; i <= (argc -1); i++ ) {

        if (strstr(argv[i], "-h") == argv[i]) {
            display_help();
            show_help_and_exit = true;
            return true;  // Don't parse further input when -h is used
        } else if (strstr(argv[i], "-vV") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_VERBOSE);
        } else if (strstr(argv[i], "-vE") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_ONLY_ERRORS);
        } else if (strstr(argv[i], "-vQ") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_QUIET);
// -a:N     hicolor_type_set()
// -L:N     hicolor_convert_left_pattern_set()
// -R:N     hicolor_convert_right_pattern_set()

        } else if (argv[i][0] == '-') {
            log_error("Unknown argument: %s\n\n", argv[i]);
            display_help();
            return false;
        }

    }

    return true;
}


// Case insensitive
static bool matches_extension(char * filename, char * extension) {

    if (strlen(filename) >= strlen(extension)) {
        char * str_ext = filename + (strlen(filename) - strlen(extension));

        return (strncasecmp(str_ext, extension, strlen(extension)) == 0);
    }
    else
        return false;
}



