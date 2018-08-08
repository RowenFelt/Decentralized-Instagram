/*
 * dispatch_definitions.h
 * Definitions for the dispatch objects
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _DISPATCH_DEFINITIONS_H
#define _DISPATCH_DEFINITIONS_H

#include <stdint.h>
#include <bson.h>
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
	int media_size;
	uint8_t *media;
	char *text;
};

struct dispatch_parent {
	int type;		
	uint64_t id; 
};

/* Change state of mongo dispatch collection (insert and delete) */
int insert_dispatch(struct dispatch *dis);
int delete_dispatch(uint64_t dispatch_id);
/* Search Mongo dispatch collection */
char *search_dispatch_by_id(uint64_t dispatch_id, int req_num, int *result, int *length);
char *search_dispatch_by_user_audience(uint64_t user_id, uint64_t *audience, 						      	int audience_size, int req_num, int *result, int *length);
char *search_dispatch_by_parent_id(uint64_t dispatch_id, int req_num, int *result, 
			int *length);
char *search_dispatch_by_tags(const char *query, int req_num, int *result, int *length);
char *search_dispatch_by_user_tags(uint64_t query, int req_num, int *result, 
			int *length);
/* transition between Mongo readable bson and dispatch structs */
int parse_dispatch_bson(struct dispatch *dis, const bson_t *bson_dispatch);
int print_dispatch_struct(struct dispatch *dis);
void dispatch_heap_cleanup(struct dispatch *dis);
int handle_dispatch_bson(bson_t *doc);


#endif /* _DISPATCH_DEFINITIONS_H */
