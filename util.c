/*
 * util.c
 * variouse utility methods
 * Authors: Rowen Felt and Campbell Boswell
 */

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

char* ip_to_string(uint32_t ip){
	static char ipstr[16];

	snprintf(ipstr, 16, "%hhu.%hhu.%hhu.%hhu", ip & 0xff, (ip & 0xff00) >> 8,
			(ip & 0xff0000) >> 16, (ip & 0xff000000) >> 24);
	
	return ipstr;
}

