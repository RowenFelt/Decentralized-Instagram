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
	uint64_t user_id;													//user_id of dispatch creator
	time_t timestamp;									
	uint32_t audience_size;										//specifies if the dispatch is for followes (audience_size == 0), if the dispatch is for the public (i.e. anyone can view it >> audience_size > 32), or for a select group of users (a group message that is capped at 32 users >> 0 < audience_size < 32)
	uint64_t audience[MAX_GROUP_SIZE];				//list of user id's who can view dispatch (group message members) - this is capped at 32 users as per instagram spec. The number of users in this list is specified by audience_size if audience size is greater than 0 (it is only greater than 0 when the dispatch is a group message)
	
	uint32_t num_tags;
	char tags[MAX_NUM_TAGS][MAX_TAG_SIZE];												//key word strings that can be used for querying - maximum 30 hashtags per post - this is the same restriction as the one used by intagram, additionally, we must know how many entries we will be making in the hashtag sub document of our dispatch document in mongo. When constructing a dispatch, "empty" entries or unused entries in this array should denoted by an inititial NULL pointer -- the final tag,if it is not the 30th tag, should be imediately followed by a NULL pointer -- creating a NULl terminated list of tags. Additionally, TODO: there should be a maximum word size of 50 characters (see MAX_TAG_SIZE). 
	uint32_t num_user_tags;
	uint64_t user_tags[MAX_GROUP_SIZE];				//user id's of tagged users, limmited to 32 max users, if there are fewer than 32 entries for user tags, then the final entry in this array must alway be less than or equal to zero, an invalid user_id value. When constructing a dispatch, unused entries in this array should be denoted by -1 -- the final user id, if it is not the 32nd tag, should be immediately followed by -1 -- creating a list that is terminated by -1.
	struct dispatch_parent *parent;										//the id the parent of this dispatch, and an indication of the parent's type (dispatch or user)
	int fragmentation;												
	uint64_t dispatch_id; 
};

struct dispatch_body {
	char *media_path;
	char *text;
};

struct dispatch_parent {
	int type;			// specifies if id field is a user_id (type == 0) or dispatch_id (type == 1) 	
	uint64_t id;  // user_id or dispatch_id (both are uint64_t)
};


// search_tag
int insert_dispatch(struct dispatch *dis);
uint64_t create_dispatch(void);
uint64_t delete_dispatch(uint64_t dispatch_id);
struct dispatch * search_dispatch_by_id(uint64_t dispatch_id);
struct dispatch * bson_to_dispatch(const bson_t *bson_dispatch);
int print_dispatch_struct(struct dispatch *dis);
int push_dispatch(uint64_t dispatch_id);
int pull_dispatch(uint64_t dispatch_id);
int update_feed(void);



#endif /* _INSTA_DEFINITIONS */
