/* 
 * user_definitions.h
 * Definitions for user objects
 * Authors: Rowen Felt and Campbell Boswell
 */

#ifndef _USER_DEFINITIONS_H
#define _USER_DEFINITIONS_H

#include <bson.h>
#include <stdint.h>

#define DIRECTION_STRING_LEN 11
#define RELATION_STRING_LEN 30

struct user {
  uint64_t user_id;
  char *username;
	uint32_t image_length;
  uint8_t *image;
  struct personal_data *bio;
  int fragmentation;
  struct insta_relations *followers;
  struct insta_relations *following;
};

struct insta_relations {
	int direction; // 0 for followed, 1 for following
	int count;
	uint64_t *user_ids;
};

struct personal_data {
  char *name;
	time_t date_created;
	time_t date_modified;
};

/* primitives for manually modifying entries and structs */
int insert_user(struct user *new_user);
int delete_user(uint64_t user_id);
void user_heap_cleanup(struct user *user);
void print_user_struct(struct user *user);

/* user search functions */
char * search_user_by_name_mongo(char *username, int req_num, int *result);
char * search_user_by_id_mongo(uint64_t user_id, int req_num, int *result);

/* higher level definitions used by the server */
int handle_user_bson(bson_t *doc);
int parse_user_bson(struct user *user, const bson_t *doc);

#endif /* _USER_DEFINITIONS_H */
