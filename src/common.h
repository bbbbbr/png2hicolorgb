// See LICENSE  file for license details

#ifndef _COMMON_H
#define _COMMON_H

// #define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define MAX_STR_LEN     4096
#define DEFAULT_STR_LEN 100

#define MAX_PATH (MAX_STR_LEN)

enum {
    IMG_TYPE_PNG
};

void set_exit_error(void);
bool get_exit_error(void);

#endif // _COMMON_H
