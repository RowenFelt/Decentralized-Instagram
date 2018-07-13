/*
 * insta_user_tests.c
 * tests basic insta user methods using mongoDB and cassandra
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "insta_user_definitions.h"

int main(int argc, char *argv[])
{
	/* test search_user */
	printf("search_user('rfelt', INSTA_FOLLOWER);\n");
	search_user_by_name("rfelt", INSTA_FOLLOWER);
	printf("search_user('Rowen Felt', INSTA_FOLLOWEE);\n");
	search_user_by_name("Rowen Felt", INSTA_FOLLOWEE);
	printf("search_user('rowen', INSTA_UNKNOWN);\n");
	search_user_by_name("rowen", INSTA_UNKNOWN);
	printf("\n\n\n");

	/* initialize test user struct */
	struct user cb;
	struct personal_data pd;
	struct insta_relations followers;
	struct insta_relations following;
	cb.user_id = 12345;
	cb.username = "cboswell";
	cb.image_path = "/images/cboswell_image/fake_path";
	pd.name = "Campbell Boswell";
	cb.bio = &pd;
	cb.fragmentation = 0;
	followers.direction = 0;
	followers.count = 5;
	uint64_t follower_ids[5] = {12, 3, 4, 16, 5};
	followers.user_ids = follower_ids;
	cb.followers = &followers;
	following.direction = 1;
	following.count = 2;
	uint64_t following_ids[2] = {666, 69};
	following.user_ids = following_ids;
	cb.following = &following;
	
	/* test insert_user */
	printf("inserting user, %s\n", cb.username);
	insert_user(&cb);
	printf("searching for user %s\n", cb.username);
	search_user_by_name("cboswell", INSTA_FOLLOWER);
	printf("search for user with id %d\n", 12345);
	search_user_by_id(12345, INSTA_FOLLOWER);
	printf("delete user with id %d\n", 12345);
	delete_user(12345);
	printf("search for user with id %d, should show nothing there\n", 12345);
  search_user_by_id(12345, INSTA_FOLLOWER);

	return 0;
}

