#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

uint32_t string_to_ip(char *ipstr){
	uint32_t ip;
	uint8_t a, b, c, d;

	sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d);
	ip = d << 24 | c << 16 | b << 8 | a;

	return ip;
}
