//
// options.c
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


 // Use sequential tile order number (0->255) for map index instead of VRAM tile index (128->255->0->127)
bool opt_map_use_sequential_tile_index = true;

void opt_set_map_tile_order(bool newval) { opt_map_use_sequential_tile_index = newval; }
bool opt_get_map_tile_order(void)        { return opt_map_use_sequential_tile_index; }
