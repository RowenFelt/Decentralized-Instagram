/*
 * insta_dispatch_definitions.h
 * Definitions for the dispatch objects
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _INSTA_DISPATCH_DEFINITIONS
#define _INSTA_DISPATCH_DEFINITIONS

#include <stdint.h>
#include <time.h>

#define MAX_GROUP_SIZE 32
#define MAX_NUM_TAGS 30
#define MAX_TAG_SIZE 50

struct dispatch {
	struct dispatch_body *body;			
	uint64_t user_id;							
	time_t timestamp;									
	uint32_t audience_size;
	uint64_t audience[MAX_GROUP_SIZE];	
	uint32_t num_tags;
	char tags[MAX_NUM_TAGS][MAX_TAG_SIZE];
	uint32_t num_user_tags;
	uint64_t user_tags[MAX_NUM_TAGS];
	struct dispatch_parent *parent;	
	uint32_t fragmentation;												
	uint64_t dispatch_id; 
};

struct dispatch_body {
	char *media_path;
	char *text;
};

struct dispatch_parent {
	int type;		
	uint64_t id; 
};

int insert_dispatch(struct dispatch *dis);
int create_dispatch(void);
int delete_dispatch(uint64_t dispatch_id);
//Search methods take a buffer pointer and buffer size. The buffer must be malloc'ed and 
//free'd at the server level (insta_server.c), but they are "filled" by these search methods
char *search_dispatch_by_id(uint64_t dispatch_id, int req_num, int *result);
char *search_dispatch_by_user_audience(uint64_t user_id, uint64_t *audience, 
																				int audience_size, int req_num, int *result);
char *search_dispatch_by_parent_id(uint64_t dispatch_id, int req_num, int *result);

int parse_dispatch_bson(struct dispatch *dis, const bson_t *bson_dispatch);
void dispatch_heap_cleanup(struct dispatch *dis);
int print_dispatch_struct(struct dispatch *dis);
int push_dispatch(struct dispatch *dis);
int pull_dispatch(uint64_t dispatch_id);
int update_feed(void);



#endif /* _INSTA_DEFINITIONS */
