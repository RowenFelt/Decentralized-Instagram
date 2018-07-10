/*
 * insta_dispatch_definitions.h
 * Definitions for the dispatch objects
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _INSTA_DISPATCH_DEFINITIONS
#define _INSTA_DISPATCH_DEFINITIONS

#include <stdint.h>

struct dispatch {
	struct dispatch_body body,
	struct user user,
	time_t timestamp,
	uint64_t *audience,
	char **tags,
	struct dispatch *parent,
	int fragmentation,
	uint64_t dispatch_id,
};

struct dispatch_body {
	char *media_path,
	char *text,
};

// search_tag
uint64_t create_dispatch(void);
uint64_t delete_dispatch(uint64_t dispatch_id);
int push_dispatch(uint64_t dispatch_id);
int pull_dispatch(uint64_t dispatch_id);
int update_feed(void);



#endif /* _INSTA_DEFINITIONS */
