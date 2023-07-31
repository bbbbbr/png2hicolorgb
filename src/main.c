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
#include "c_source.h"

#include <hicolour.h>


#define VERSION "version 1.4.0"

static void init(void);
static void cleanup(void);
static void display_help(void);
static int  handle_args(int argc, char * argv[]);
static void set_drag_and_drop_mode_defaults(void);

image_data src_image;
char filename_in[MAX_STR_LEN] = {'\0'};
char opt_filename_out[MAX_STR_LEN] = "";
// bool opt_strip_output_filename_ext = true;
bool opt_c_file_output = false;
int  opt_bank_num = BANK_NUM_UNSET;

int  show_help_and_exit = false;


static void init(void) {
    // Handle global init
    src_image.p_img_data = NULL;
    hicolor_init();
}


// Registered as an atexit handler
static void cleanup(void) {
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

                    // If output filename is not specified, use source filename
                    if (opt_filename_out[0] == '\0')
                        strcpy(opt_filename_out, filename_in);
                    filename_remove_extension(opt_filename_out);

                    hicolor_process_image(&src_image, opt_filename_out);
                    // TODO: ? move this into process image? or move file output out of it?
                    if (opt_c_file_output)
                        file_c_output_write(opt_filename_out, opt_bank_num, &src_image);

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
        "\n"
        "png2hicolorgb input_image.png [options]\n"
        VERSION": bbbbbr. Based on Glen Cook Windows hicolor.exe 1.2\n"
        "Convert an image to Game Boy Hi-Color format\n"
        "\n"
        "Options\n"
        "-h        : Show this help\n"
        "-v*       : Set log level: \"-v\" verbose, \"-vQ\" quiet, \"-vE\" only errors, \"-vD\" debug\n"
        "-o=*      : Set base output filename (otherwise from input image)\n"
        // "--keepext   : Do not strip extension from output filename\n"
        "--csource : Export C source format with incbins for data files\n"
        "--bank=N  : Set bank number for C source output where N is decimal bank number 1-511\n"
        "--type=N  : Set conversion type where N is one of below \n"
        "             1: Median Cut - No Dither (*Default*)\n"
        "             2: Median Cut - With Dither\n"
        "             3: Wu Quantiser\n"
        "-p        : Show screen attribute pattern options (no processing)\n"
        "-L=N      : Set Left  screen attribute pattern where N is decimal entry (-p to show patterns)\n"
        "-R=N      : Set Right screen attribute pattern where N is decimal entry (-p to show patterns)\n"
        "\n"
        "Example 1: \"png2hicolorgb myimage.png\"\n"
        "Example 2: \"png2hicolorgb myimage.png --type=3 -L=2 -R=2 --csource -o=my_output_filename\"\n"
        "\n"
        "Historical credits and info:\n"
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
static void set_drag_and_drop_mode_defaults(void) {

    // Set some options here
}


static int handle_args(int argc, char * argv[]) {

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

        if ((strstr(argv[i], "-h") == argv[i]) || (strstr(argv[i], "-?") == argv[i])) {
            display_help();
            show_help_and_exit = true;
            return true;  // Don't parse further input when -h is used
        } else if (strstr(argv[i], "-p") == argv[i]) {
            log_standard(HELP_CONV_PATTERN_STR);
            show_help_and_exit = true;
            return true;  // Don't parse further input when -h is used

        } else if (strstr(argv[i], "-vD") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_DEBUG);
        } else if (strstr(argv[i], "-vE") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_ONLY_ERRORS);
        } else if (strstr(argv[i], "-vQ") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_QUIET);
        } else if (strstr(argv[i], "-v") == argv[i]) {
            log_set_level(OUTPUT_LEVEL_VERBOSE);

        } else if (strstr(argv[i], "--type=") == argv[i]) {
            uint8_t new_type = strtol(argv[i] + strlen("--type="), NULL, 10);
            if ((new_type < CONV_TYPE_MIN) || (new_type > CONV_TYPE_MAX)) {
                log_standard("Error: --type specified with invalid conversion setting: %d\n", new_type);
                display_help();
                show_help_and_exit = true;
                return false; // Abort
            }
            else
                hicolor_set_type(new_type);
        } else if (strstr(argv[i], "-L=") == argv[i]) {
            hicolor_set_convert_left_pattern( strtol(argv[i] + strlen("-L="), NULL, 10));
        } else if (strstr(argv[i], "-R=") == argv[i]) {
            hicolor_set_convert_right_pattern( strtol(argv[i] + strlen("-R="), NULL, 10));

        } else if (strstr(argv[i], "-o=") == argv[i]) {
            // Require colon and filename to be present
             if (strlen(argv[i]) < strlen("-o=N")) {
                log_standard("Error: -o specified but filename missing or invalid format. Ex: -o=my_base_output_filename\n");
                show_help_and_exit = true;
                return false; // Abort
            }
            snprintf(opt_filename_out, sizeof(opt_filename_out), "%s", argv[i] + strlen("-o="));
        // } else if (strstr(argv[i], "--keepext") == argv[i]) {
        //     opt_strip_output_filename_ext = false;
        } else if (strstr(argv[i], "--csource") == argv[i]) {
            opt_c_file_output = true;
        } else if (strstr(argv[i], "--bank=") == argv[i]) {
            opt_bank_num = strtol(argv[i] + strlen("--bank="), NULL, 10);
            if ((opt_bank_num < BANK_NUM_MIN) || (opt_bank_num > BANK_NUM_MAX)) {
                log_standard("Error: Invalid bank number specified with --bank=%d\n", opt_bank_num);
                display_help();
                show_help_and_exit = true;
                return false; // Abort
            }

        } else if (argv[i][0] == '-') {
            log_error("Unknown argument: %s\n\n", argv[i]);
            display_help();
            return false;
        }

    }

    return true;
}

