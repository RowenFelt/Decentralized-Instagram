/* 
 * insta_user_definitions.h
 * Definitions for user objects
 * Authors: Rowen Felt and Campbell Boswell
 */

#ifndef _INSTA_USER_DEFINITIONS
#define _INSTA_USER_DEFINITIONS

#include <stdint.h>

struct user {
  uint64_t user_id,
  char *username,
  char *image_path,
  struct personal_data bio,
  int fragmentation,
  uint64_t *followers,
  uint64_t *following,
};

struct personal_data {
  char *name,
};

int init_user(void);
int search_user(char *username);
int insert_user(struct user *new_user);
int delete_user(uint64_t user_id);
int pull_user_profile(uint64_t user_id);

#endif /* _INSTA_USER_DEFINITIONS */
