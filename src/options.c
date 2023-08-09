//
// options.c
//
#include <stdbool.h>

#include "options.h"

bool opt_map_use_sequential_tile_index = OPT_MAP_TILE_SEQUENTIAL_ORDER;
bool opt_tile_dedupe                   = true;
bool opt_c_file_output                 = false;
int  opt_bank_num                      = BANK_NUM_UNSET;
