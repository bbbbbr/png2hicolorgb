// TODO: Permissive License

#ifndef GBC_HICOLOR_H
#define GBC_HICOLOR_H

#define HICOLOR_VAR(varname) varname ## _data

typedef struct hicolor_data {
        uint16_t  tile_count;
        uint8_t   height_in_tiles;
        uint8_t * p_tiles;
        uint8_t * p_map;
        uint8_t * p_attribute_map;
        uint8_t * p_palette;
} hicolor_data;

void hicolor_start(hicolor_data * p_hicolor) NONBANKED;
void hicolor_stop(void) NONBANKED;


#endif