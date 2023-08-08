// See LICENSE  file for license details

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>


#include "common.h"
#include "defines.h"
#include "path_ops.h"
#include "options.h"
#include "logging.h"
#include "image_load.h"

#include <hicolour.h>


#define VERSION "version 1.4.1"

image_data src_image;
static char const * filename_in = NULL;
static char opt_filename_out[MAX_STR_LEN] = "";
// bool opt_strip_output_filename_ext = true;


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


static void display_help(void) {
    ERR( // Not an error, but ensure that it never gets suppressed.
        "Usage: png2hicolorgb input_image.png [options]\n"
        VERSION": bbbbbr. Based on Glen Cook's Windows GUI \"hicolour.exe\" 1.2\n"
        "Convert an image to Game Boy Hi-Color format\n"
        "\n"
        "Options\n"
        "-h         : Show this help\n"
        "-v*        : Set log level: \"-v\" verbose, \"-vQ\" quiet, \"-vE\" only errors, \"-vD\" debug\n"
        "-o <file>  : Set base output filename (otherwise from input image)\n"
        // "--keepext   : Do not strip extension from output filename\n"
        "--csource  : Export C source format with incbins for data files\n"
        "--bank=N   : Set bank number for C source output where N is decimal bank number 1-511\n"
        "--type=N   : Set conversion type where N is one of below \n"
        "              1: Median Cut - No Dither (*Default*)\n"
        "              2: Median Cut - With Dither\n"
        "              3: Wu Quantiser\n"
        "-p         : Show screen attribute pattern options (no processing)\n"
        "-L=N       : Set Left  screen attribute pattern where N is decimal entry (-p to show patterns)\n"
        "-R=N       : Set Right screen attribute pattern where N is decimal entry (-p to show patterns)\n"
        "--vaddrid  : Map uses vram id (128->255->0->127) instead of (*Default*) sequential tile order (0->255)\n"
        "--nodedupe : Turn off tile pattern deduplication\n"
        "\n"
        "Example 1: \"png2hicolorgb myimage.png\"\n"
        "Example 2: \"png2hicolorgb myimage.png --csource -o=my_output_filename\"\n"
        "* Default settings provide good results. Better quality but slower: \"--type=3 -L=2 -R=2\"\n"
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
   );
}


#ifdef DRAG_AND_DROP_MODE
// Default options for Windows Drag and Drop recipient mode
static void set_drag_and_drop_mode_defaults(void) {

    // Set some options here
}
#endif


#define GET_PARAM_OR_ERR(longopt, i, argv) \
    if (!param) { \
        ++*i; \
        param = argv[*i]; \
        if (!param) { \
            ERR("Error: `--" longopt "` requires an argument!\n"); \
            return false; \
        } \
    }

#define REJECT_PARAM(longopt) \
    if (param) { \
        ERR("Error: `--" longopt "` doesn't take an argument!\n"); \
        return false; \
    }

static bool handle_bank(char const * param, int * restrict i, char * argv[]) {
    GET_PARAM_OR_ERR("bank", i, argv);

    char * endptr;
    unsigned long bank = strtoul(param, &endptr, 0);
    if (*param == '\0' || *endptr != '\0') {
        ERR("Error: Invalid bank number \"%s\"\n", param);
        return false;
    }

    if (bank < BANK_NUM_MIN || bank > BANK_NUM_MAX) {
        ERR("Error: Bank number %lu out of range\n", bank);
        return false;
    }
    opt_set_bank_num(bank);
    return true;
}

static bool handle_csource(char const * param) {
    REJECT_PARAM("csource");

    opt_set_c_file_output(true);
    return true;
}

static bool handle_help(char const * param) {
    REJECT_PARAM("help");

    display_help();
    return true;
}

static bool handle_left(char const * param, int * restrict i, char * argv[]) {
    GET_PARAM_OR_ERR("left", i, argv);

    char * endptr;
    unsigned long pattern = strtoul(param, &endptr, 0);
    if (*param == '\0' || *endptr != '\0') {
        ERR("Error: Invalid attribute pattern \"%s\"\n", param);
        return false;
    }

    hicolor_set_convert_left_pattern(pattern);
    return true;
}

static bool handle_nodedupe(char const * param) {
    REJECT_PARAM("nodedupe");

    opt_set_tile_dedupe(false);
    return true;
}

static bool handle_output(char const * param, int * restrict i, char * argv[]) {
    GET_PARAM_OR_ERR("output", i, argv);

    // FIXME: this can truncate.
    // (The proper fix would be to make this a pointer, and use a temporary buffer if NULL where this is used.)
    snprintf(opt_filename_out, sizeof(opt_filename_out), "%s", param);
    return true;
}

static bool handle_pattern_help(char const * param) {
    REJECT_PARAM("pattern-help");

    ERR(HELP_CONV_PATTERN_STR);
    return true;
}

static bool handle_right(char const * param, int * restrict i, char * argv[]) {
    GET_PARAM_OR_ERR("right", i, argv);

    char * endptr;
    unsigned long pattern = strtoul(param, &endptr, 0);
    if (*param == '\0' || *endptr != '\0') {
        ERR("Error: Invalid attribute pattern \"%s\"\n", param);
        return false;
    }

    hicolor_set_convert_right_pattern(pattern);
    return true;
}

static bool handle_type(char const * param, int * restrict i, char * argv[]) {
    GET_PARAM_OR_ERR("type", i, argv);

    char * endptr;
    unsigned long type = strtoul(param, &endptr, 0);
    if (*param == '\0' || *endptr != '\0') {
        ERR("Error: Invalid conversion type \"%s\"\n", param);
        return false;
    }

    if (type < CONV_TYPE_MIN || type > CONV_TYPE_MAX) {
        ERR("Error: Conversion type %lu out of range\n", type);
        return false;
    }
    hicolor_set_type(type);
    return true;
}

static bool handle_vaddrid(char const * param) {
    REJECT_PARAM("vaddrid");

    opt_set_map_tile_order(OPT_MAP_TILE_ORDER_BY_VRAM_ID);
    return true;
}

static bool handle_verbose(char const * param) {
    if (!param) {
        set_log_level(OUTPUT_LEVEL_VERBOSE);
    } else {
        switch (param[0]) {
        case 'Q':
        case 'E':
            set_log_level(OUTPUT_LEVEL_ONLY_ERRORS);
            break;
        case 'D':
            set_log_level(OUTPUT_LEVEL_DEBUG);
            break;
        default:
            ERR("Error: Unknown verbosity level \"%s\"\n", param);
            return false;
        }
        if (param[1] != '\0') {
            ERR("Error: Unknown verbosity level \"%s\"\n", param);
            return false;
        }
    }
    return true;
}

#undef GET_PARAM_OR_ERR
#undef REJECT_PARAM

static bool handle_args(int argc, char * argv[static argc + 1]) {
    if (argc < 2) {
        display_help();
        return false;
    }

    bool opts_enabled = true;
    for (int i = 1; i != argc; ++i) {
        assert(i < argc);
        char const * arg = argv[i];

        if (*arg == '-' && opts_enabled) {
            // This is an option!
            ++arg; // Skip that dash.

            if (*arg == '-') {
                // This is a long option!
                ++arg; // Skip that dash.

                if (*arg == '\0') {
                    // A double-dash terminates option processing.
                    opts_enabled = false;
                    continue;
                }

                // Extract the option part—it goes up to the end of `arg`, or up to the first '='.
                char const * end = strchr(arg, '=');
                size_t opt_len = end ? (size_t)(end - arg) : strlen(arg);
                char const * param = end ? end + 1 : NULL; // Note: `argv` is NULL-terminated.

                // To accelerate matching option names, we first dispatch based on the first character.
#define IS_OPT(name_str) \
    (opt_len == sizeof(name_str) - 1 \
    && !memcmp(arg, (char[sizeof(name_str) - 1]){name_str}, sizeof(name_str) - 1))
#define HANDLE_OPT(name_str, handler_expr, on_success) \
    if (IS_OPT(name_str)) { \
        if (!(handler_expr)) { \
            return false; \
        } \
        on_success; \
    }
                switch (*arg) {
                    case 'b':
                        HANDLE_OPT("bank", handle_bank(param, &i, argv), break);
                        // fallthrough
                    case 'c':
                        HANDLE_OPT("csource", handle_csource(param), break);
                        // fallthrough
                    case 'h':
                        HANDLE_OPT("help", handle_help(param), exit(EXIT_SUCCESS));
                        // fallthrough
                    case 'L':
                        HANDLE_OPT("left", handle_left(param, &i, argv), break);
                        // fallthrough
                    case 'n':
                        HANDLE_OPT("nodedupe", handle_nodedupe(param), break);
                        // fallthrough
                    case 'o':
                        HANDLE_OPT("output", handle_output(param, &i, argv), break);
                        // fallthrough
                    case 'p':
                        HANDLE_OPT("pattern-help", handle_pattern_help(param), exit(EXIT_SUCCESS));
                        // fallthrough
                    case 'R':
                        HANDLE_OPT("right", handle_right(param, &i, argv), break);
                        // fallthrough
                    case 't':
                        HANDLE_OPT("type", handle_type(param, &i, argv), break);
                        // fallthrough
                    case 'v':
                        HANDLE_OPT("vaddrid", handle_vaddrid(param), break);
                        HANDLE_OPT("verbose", handle_verbose(param), break);
                        // fallthrough
                    default:
                        ERR("Error: Unknown option `--%s`\n", arg);
                        return false;
                }
#undef HANDLE_OPT
#undef IS_OPT
            } else if (*arg != '\0') {
                // This is a short option! Or maybe multiple, packed in a single arg.

                do {
                    switch (*arg) {
                        case 'h':
                        case '?':
                            if (!handle_help(NULL)) {
                                return false;
                            }
                            exit(EXIT_SUCCESS);
                        case 'L':
                            if (!handle_left(arg[1] == '\0' ? NULL : &arg[1], &i, argv)) {
                                return false;
                            }
                            goto end_arg;
                        case 'o':
                            if (!handle_output(arg[1] == '\0' ? NULL : &arg[1], &i, argv)) {
                                return false;
                            }
                            goto end_arg;
                        case 'p':
                            if (!handle_pattern_help(NULL)) {
                                return false;
                            }
                            exit(EXIT_SUCCESS);
                        case 'R':
                            if (!handle_right(arg[1] == '\0' ? NULL : &arg[1], &i, argv)) {
                                return false;
                            }
                            goto end_arg;
                        case 'v':
                            if (!handle_verbose(arg[1] == '\0' ? NULL : &arg[1])) {
                                return false;
                            }
                            goto end_arg; // No nested `break` :(
                        default:
                            ERR("Error: Unknown option `-%c`", *arg);
                            return false;
                    }
                    ++arg;
                } while (*arg != '\0');
end_arg: ;
            } else {
                // This is a single dash, meaning to read the image from stdin.
                ERR("Error: reading the image from stdin is not supported (yet)\n");
                return false;
            }
        } else {
            if (filename_in == NULL) {
                filename_in = arg;
            } else {
                ERR("Error: input image path specified twice! (First \"%s\", then \"%s\")\n", filename_in, arg);
                return false;
            }
        }
    }

    if (!filename_in) {
        ERR("Error: input image path not specified!\n");
        return false;
    }

    return true;
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
        // detect file extension
        if (matches_extension(filename_in, (char *)".png")) {

            // Load source image (from first argument)
            if (image_load(&src_image, filename_in, IMG_TYPE_PNG)) {

                // If output filename is not specified, use source filename
                if (opt_filename_out[0] == '\0')
                    strcpy(opt_filename_out, filename_in);

                filename_remove_extension(opt_filename_out);
                hicolor_process_image(&src_image, opt_filename_out);
                ret = EXIT_SUCCESS; // Exit with success
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
