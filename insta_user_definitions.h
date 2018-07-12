/* 
 * insta_user_definitions.h
 * Definitions for user objects
 * Authors: Rowen Felt and Campbell Boswell
 */

#ifndef _INSTA_USER_DEFINITIONS
#define _INSTA_USER_DEFINITIONS

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
	char *birthdate;
};

int init_user(void);
int search_user_by_name(char *username, int flags);
int search_user_by_id(uint64_t user_id, int flags);
int insert_user(struct user *new_user);
int delete_user(uint64_t user_id);
int pull_user_profile(uint64_t user_id);

#endif /* _INSTA_USER_DEFINITIONS */
