#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Writes a buffer to a file
bool file_write_from_buffer(char const * filename, char const * p_buf, size_t data_len) {

    bool status = false;
    FILE * file_out = fopen(filename, "wb");

    if (file_out) {
        if (data_len == fwrite(p_buf, 1, data_len, file_out))
            status = true;
        else
            printf("Warning: File write size didn't match expected for %s\n", filename);

        fclose(file_out);
    } else
      printf("Warning: Unable to open file: %s\n", filename);
    return status;
}
