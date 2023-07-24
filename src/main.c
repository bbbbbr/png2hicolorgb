// See LICENSE  file for license details

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>


#include "common.h"
#include "path_ops.h"
#include "logging.h"
#include "image_load.h"

#include <hicolour.h>


#define VERSION "version 1.4.0"

void static display_help(void);
int handle_args(int argc, char * argv[]);
static void init(void);
void cleanup(void);
static void set_drag_and_drop_mode_defaults(void);

image_data src_image;
char filename_in[MAX_STR_LEN] = {'\0'};
char opt_base_output_filename[MAX_STR_LEN] = "";

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

                    // If specified use custom base filename, otherwise strip extension from input
                    if (opt_base_output_filename[0] != '\0')
                        hicolor_process_image(&src_image, opt_base_output_filename);
                    else {
                        filename_remove_extension(filename_in);
                        hicolor_process_image(&src_image, filename_in);
                    }

                    ret = EXIT_SUCCESS; // Exit with success
                }
            }
        }
    }

    // Override exit code if was set during processing
    if (get_exit_error())
        ret = EXIT_FAILURE;

    // if (ret == EXIT_FAILURE)
    //     log_error("Error loading, converting or writing out the image: %s\n", filename_in);

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
        "-h    : Show this help\n"
        "-o:*  : Set base output filename\n"
        "-v*   : Set log level: \"-vV\" verbose, \"-vQ\" quiet, \"-vE\" only errors\n"
        "-cM:N : Set conversion method where N is one of below \n"
        "        0: Original (J.Frohwein)\n"
        "        1: Median Cut - No Dither (*Default*)\n"
        "        2: Median Cut - With Dither\n"
        "        3: Wu Quantiser\n"
        "-cL:N : Set Left  screen conversion pattern where N is decimal entry (-sP to show patterns)\n"
        "-cR:N : Set Right screen conversion pattern where N is decimal entry (-sP to show patterns)\n"
        "-sP   : Show screen conversion attribute pattern widths list (no processing)\n"
        "\n"
        "Example 1: \"png2hicolorgb myimage.png\"\n"
        "Example 2: \"png2hicolorgb myimage.png -cM:3 -cL:2 -cR:2 -o:my_base_output_filename\"\n" // -n:somevarname
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

        } else if (strstr(argv[i], "-cM:") == argv[i]) {
            hicolor_set_type( strtol(argv[i] + strlen("-cA:"), NULL, 10));
        } else if (strstr(argv[i], "-cL:") == argv[i]) {
            hicolor_set_convert_left_pattern( strtol(argv[i] + strlen("-cL:"), NULL, 10));
        } else if (strstr(argv[i], "-cR:") == argv[i]) {
            hicolor_set_convert_right_pattern( strtol(argv[i] + strlen("-cR:"), NULL, 10));

        } else if (strstr(argv[i], "-o") == argv[i]) {
            // Require colon and filename to be present
             if (strlen(argv[i]) < strlen("-o:N")) {
                log_standard("Error: -o specified but filename missing or invalid format. Ex: -o:my_base_output_filename\n");
                show_help_and_exit = true;
                return false; // Abort
            }
            snprintf(opt_base_output_filename, sizeof(opt_base_output_filename), "%s", argv[i] + strlen("-o:"));

        // } else if (strstr(argv[i], "--varname=") == argv[i]) {
        //     snprintf(opt_c_source_output_varname, sizeof(opt_c_source_output_varname), "%s", argv[i] + 10);

        // } else if (strstr(argv[i], "--bank=") == argv[i]) {
        //     opt_bank_num = atoi(argv[i] + strlen("--bank="));
        //     if ((opt_bank_num < BANK_NUM_ROM_MIN) || (opt_bank_num > BANK_NUM_ROM_MAX)) {
        //         printf("gbcompress: Warning: Bank Num %d outside of range %d - %d\n", opt_bank_num, BANK_NUM_ROM_MIN, BANK_NUM_ROM_MAX);
        //         return false;
        //     }


        } else if (argv[i][0] == '-') {
            log_error("Unknown argument: %s\n\n", argv[i]);
            display_help();
            return false;
        }

    }

    return true;
}
