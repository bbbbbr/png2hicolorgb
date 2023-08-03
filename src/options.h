// "options.h"

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>
#include <stdbool.h>

#define OPTION_UNSET 0xFFFF

#define OPT_MAP_TILE_ORDER_BY_VRAM_ID  false
#define OPT_MAP_TILE_SEQUENTIAL_ORDER  true

void opt_set_map_tile_order(bool newval);
bool opt_get_map_tile_order(void);

#endif

