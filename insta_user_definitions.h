/* 
 * insta_user_definitions.h
 * Definitions for user objects
 * Authors: Rowen Felt and Campbell Boswell
 */

#ifndef _INSTA_USER_DEFINITIONS
#define _INSTA_USER_DEFINITIONS

#include <bson.h>
#include <stdint.h>

#define INSTA_FOLLOWER 0x01
#define INSTA_FOLLOWEE 0x02
#define INSTA_UNKNOWN 0x03

struct user {
  uint64_t user_id;
  char *username;
  char *image_path;
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

int init_user(void);
char * search_user_by_name_mongo(char *username, int req_num, int *result);
int search_user_by_name_cass(char *username);
char * search_user_by_id_mongo(uint64_t user_id, int req_num, int *result);
char * search_user_by_id_cass(uint64_t user_id);
int insert_user(struct user *new_user);
int delete_user(uint64_t user_id);
int pull_user_profile(uint64_t user_id);
int parse_user_bson(struct user *user, const bson_t *doc);
void user_heap_cleanup(struct user *user);
void print_user_struct(struct user *user);

#endif /* _INSTA_USER_DEFINITIONS */
