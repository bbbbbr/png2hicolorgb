// "options.h"

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

// Use sequential tile order number (0->255) for map index instead of VRAM tile index (128->255->0->127)
#define OPT_MAP_TILE_ORDER_BY_VRAM_ID  false
#define OPT_MAP_TILE_SEQUENTIAL_ORDER  true
extern bool opt_map_use_sequential_tile_index;

extern bool opt_tile_dedupe;

extern bool opt_c_file_output;

#define BANK_NUM_UNSET 0
#define BANK_NUM_MIN   1
#define BANK_NUM_MAX   511
extern int  opt_bank_num;


#endif

