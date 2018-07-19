#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>
#include <mongoc.h>

uint32_t string_to_ip(char* ipstr);
char* ip_to_string(uint32_t ip);
char* build_json(mongoc_cursor_t *cursor, int req_num, int *result);
#endif /* __UTIL_H */
