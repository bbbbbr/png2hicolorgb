// image_load.c

//
// Handles loading PNG images and reformatting them to a usable state
//

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "logging.h"
#include "image_info.h"
#include "options.h"

#include <lodepng.h>


static bool image_validate(image_data *);
static void image_convert_RGBA24_to_BGRX24(image_data *);
static bool image_load_png(image_data *, const char *);


#define VALIDATE_WIDTH  160u
#define VALIDATE_HEIGHT 144u

// Validates incoming image properties
// Return: true if success
static bool image_validate(image_data * p_decoded_image) {

    if (p_decoded_image->width != VALIDATE_WIDTH) {
        log_error("Error: Image width is %d, must be %d\n", p_decoded_image->width, VALIDATE_WIDTH);
        return false;
    }

    if (p_decoded_image->height != VALIDATE_HEIGHT) {
        log_error("Error: Image height is %d, must be %d\n", p_decoded_image->height, VALIDATE_HEIGHT);
        return false;
    }

    // if ((p_decoded_image->height % 4) != 0u) {
    //     log_verbose("Error: Image height %d is not a multiple of 4 (it must be)\n", p_decoded_image->height);
    //     return false;
    // }

    return true;
}

// TODO: ok to remove?
/*
// Converts RGBA24 to BGRX24 (Windows RGBQUAD)
// Swaps [R] <-> [B], ignores [A]
static void image_convert_RGBA24_to_BGRX24(image_data * p_decoded_image) {

    uint8_t red_temp;
    uint8_t * p_data = p_decoded_image->p_img_data;

    for (int c = 0; c < p_decoded_image->size; c++) {
        red_temp = *p_data;       // redtemp   <- RGBA.red
        *p_data = *(p_data + 2);  // BGRX.blue <- RGBA.blue
        *(p_data + 2) = red_temp; // BGRX.red  <- red_temp
    }
}

*/
// Loads a PNG image image into RGBA24 format
// Return: true if success
static bool image_load_png(image_data * p_decoded_image, const char * filename) {

    bool status = true;
    LodePNGState png_state;
    lodepng_state_init(&png_state);

    // // Decode with auto conversion to RGBA32
    // unsigned error = lodepng_decode32_file(&p_decoded_image->p_img_data, &p_decoded_image->width, &p_decoded_image->height, filename);
    // Decode with auto conversion to RGBA24
    unsigned error = lodepng_decode24_file(&p_decoded_image->p_img_data, &p_decoded_image->width, &p_decoded_image->height, filename);

    if (error) {
        status = false;
        log_error("Error: PNG load: %u: %s\n", error, lodepng_error_text(error));
    } else {
        p_decoded_image->bytes_per_pixel = RGB_24SZ;
        p_decoded_image->size = p_decoded_image->width * p_decoded_image->height * p_decoded_image->bytes_per_pixel;
    }

    // Free resources
    lodepng_state_cleanup(&png_state);

    return status;
}


// Loads an image image
// Return: true if success
bool image_load(image_data * p_decoded_image, const char * filename, const uint8_t image_type) {

    bool status = true;

    log_verbose("Loading image from file: %s, type: %d\n", filename, image_type);

    switch (image_type) {
        case IMG_TYPE_PNG:
            status = image_load_png(p_decoded_image, filename);
            break;

        default:
            status = false;
            log_error("Invalid image format. No image will be loaded\n");
            break;
    }

    if (status)
        status = image_validate(p_decoded_image);

    if (status) {
        // image_convert_RGBA24_to_BGRX24(p_decoded_image); // TODO: remove?
        log_verbose("Decoded image.width: %d\n", p_decoded_image->width);
        log_verbose("Decoded image.height: %d\n", p_decoded_image->height);
        log_verbose("Decoded image.size: %d\n", p_decoded_image->size);
        log_verbose("Decoded image.bytes_per_pixel: %d\n", p_decoded_image->bytes_per_pixel);
    }

    return status;
}

