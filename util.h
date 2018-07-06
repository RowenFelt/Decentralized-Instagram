#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>

uint32_t string_to_ip(char* ipstr);
char* ip_to_string(uint32_t ip);
#endif /* __UTIL_H */
