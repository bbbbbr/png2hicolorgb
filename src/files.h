#ifndef _FILES_H
#define _FILES_H

#include <stdbool.h>
#include <stddef.h>

bool file_write_from_buffer(char * filename, char * p_buf, size_t data_len);
// Convenience for not requiring pointer casts.
// Casting from any character type (`char`) to any other character type is fine.
#define file_write_from_buffer(filename, p_buf, data_len) \
	file_write_from_buffer(filename, \
	                       _Generic(p_buf, unsigned char *: (char *)p_buf, default: p_buf), \
	                       data_len)

#endif // _FILES_H
