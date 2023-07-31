// See LICENSE  file for license details

#ifndef _COMMON_H
#define _COMMON_H

#define TILE_HEIGHT_PX       8
#define TILE_WIDTH_PX        8

#define BANK_NUM_UNSET 0
#define BANK_NUM_MIN   1
#define BANK_NUM_MAX   511

#define MAX_STR_LEN     4096
#define DEFAULT_STR_LEN 100

#define MAX_PATH (MAX_STR_LEN)

enum {
    IMG_TYPE_PNG
};

void set_exit_error(void);
bool get_exit_error(void);

#endif // _COMMON_H
