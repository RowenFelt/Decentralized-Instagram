#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mongoc.h>
#include <bson.h>
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

char*
build_json(mongoc_cursor_t *cursor, int req_num, int *result){
	const bson_t *result_dispatch;
	size_t json_length;	
	int buf_size;
	bson_error_t error;
	char* buf = NULL;	
  
	buf_size = 0;
	*result = 0;

	if(req_num == -1){
		req_num = INT_MAX;
	}

	while(mongoc_cursor_next(cursor, &result_dispatch) && *result < req_num){
    char *json_str;
    json_str = bson_as_json(result_dispatch, &json_length);
    buf_size += json_length;
    buf = realloc(buf, buf_size);
    strncpy(buf + buf_size - json_length, json_str, json_length);
    *result += 1;
  }

	if(mongoc_cursor_error(cursor, &error)){
		fprintf(stderr, "Failed to itterate through all documents: %s\n", error.message);
	}

	return buf;

}
