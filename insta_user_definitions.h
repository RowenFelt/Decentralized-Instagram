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
  uint64_t *followers;
  uint64_t *following;
};

struct personal_data {
  char *name;
	char *birthdate;
};

int init_user(void);
int search_user(char *username, int flags);
int insert_user(struct user *new_user);
int delete_user(uint64_t user_id);
int pull_user_profile(uint64_t user_id);

#endif /* _INSTA_USER_DEFINITIONS */
