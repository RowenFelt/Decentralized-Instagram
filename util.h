#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

uint32_t string_to_ip(char* ipstr);
char* ip_to_string(uint32_t ip);
int view_media_from_binary(const char *filename, void *media_buf, int media_size);
#endif /* _UTIL_H */
