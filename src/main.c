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
        "-v*   : Set log level: \"-vV\" verbose, \"-vQ\" quiet, \"-vE\" only errors\n"
        "-cA:N : Set conversion algorithm where N is one of below \n"
        "        0:Original Method (J.Frohwein)\n"
        "        1:Median Cut - No Dither\n"
        "        2:Median Cut - With Dither\n"
        "        3:Wu Quantiser\n"
        "-cL:N : Set Left  screen conversion pattern where N is decimal entry (-sP to show patterns)\n"
        "-cR:N : Set Right screen conversion pattern where N is decimal entry (-sP to show patterns)\n"
        "-sP   : Show available screen conversion pattern list (no processing)\n"
        "\n"
        "Example 1: \"png2hicolorgb myimage.png\"\n"
        "Example 2: \"png2hicolorgb myimage.png -cA:3 0cL:// -o:outputfilename -n:somevarname\"\n"
        "\n"
        "Based on win32 hicolour.exe. Historical info:\n"
        "   Original Concept : Icarus Productions\n"
        "   Original Code : Jeff Frohwein\n"
        "   Full Screen Modification : Anon\n"
        "   Adaptive Code : Glen Cook\n"
        "   Windows Interface : Glen Cook\n"
        "   Additional Windows Programming : Rob Jones\n"
        "   Original Quantiser Code : Benny\n"
        "   Quantiser Conversion : Glen Cook\n"
        "\n"
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
        } else if (strstr(argv[i], "-sP") == argv[i]) {
            log_standard(HELP_CONV_PATTERN_STR);
            show_help_and_exit = true;
            return true;  // Don't parse further input when -h is used

        } else if (strstr(argv[i], "-vD") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_DEBUG);
        } else if (strstr(argv[i], "-vV") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_VERBOSE);
        } else if (strstr(argv[i], "-vE") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_ONLY_ERRORS);
        } else if (strstr(argv[i], "-vQ") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_QUIET);

        } else if (strstr(argv[i], "-cA:") == argv[i]) {
            hicolor_set_type( strtol(argv[i] + strlen("-cA:"), NULL, 10));
        } else if (strstr(argv[i], "-cL:") == argv[i]) {
            hicolor_set_convert_left_pattern( strtol(argv[i] + strlen("-cL:"), NULL, 10));
        } else if (strstr(argv[i], "-cR:") == argv[i]) {
            hicolor_set_convert_right_pattern( strtol(argv[i] + strlen("-cR:"), NULL, 10));


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
