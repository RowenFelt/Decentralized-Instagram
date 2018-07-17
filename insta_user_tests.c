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
	/* initialize test user structs */
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
	
	struct user rf;
	struct personal_data rf_pd;
	struct insta_relations rf_followers;
	struct insta_relations rf_following;
	rf.user_id = 11254155;
	rf.username = "rfelt";
	rf.image_path = "/images/rfelt_image/fake_path";
	rf_pd.name = "Rowen Felt";
	rf.bio = &rf_pd;
	rf.fragmentation = 0;
	rf_followers.direction = 0;
	rf_followers.count = 3;
	uint64_t rf_follower_ids[3] = {100, 33, 423};
	rf_followers.user_ids = rf_follower_ids;
	rf.followers = &rf_followers;
	rf_following.direction = 1;
	rf_following.count = 4;
	uint64_t rf_following_ids[4] = {234, 151, 2, 34};
	rf_following.user_ids = rf_following_ids;
	rf.following = &rf_following;

	struct user pj;
	struct personal_data pj_pd;
	struct insta_relations pj_followers;
	struct insta_relations pj_following;
	pj.user_id = 159178;
	pj.username = "pjonhson";
	pj.image_path = "/images/pjohnson_image/fake_path";
	pj_pd.name = "Pete Johnson";
	pj.bio = &pj_pd;
	pj.fragmentation = 0;
	pj_followers.direction = 0;
	pj_followers.count = 2;
	uint64_t pj_follower_ids[2] = {16, 88};
	pj_followers.user_ids = pj_follower_ids;
	pj.followers = &pj_followers;
	pj_following.direction = 1;
	pj_following.count = 9;
	uint64_t pj_following_ids[9] = {66666, 64, 346, 15235, 2356, 6, 2, 3, 99};
	pj_following.user_ids = pj_following_ids;
	pj.following = &pj_following;
	
	/* test insert_user */
	printf("inserting user, %s\n", cb.username);
	insert_user(&cb);
	printf("searching for user %s\n", cb.username);
	search_user_by_name_mongo("cboswell");
	printf("search for user with id %d\n", 12345);
	search_user_by_id_mongo(12345);
	printf("delete user with id %d\n", 12345);
	delete_user(12345);
	printf("search for user with id %d, should show nothing there\n", 12345);
  search_user_by_id_cass(12345);
	insert_user(&rf);
	insert_user(&pj);

	/* test search_user */
	printf("search_user('rfelt', INSTA_FOLLOWER);\n");
	search_user_by_name_mongo("rfelt");
	printf("search_user('Rowen Felt', INSTA_FOLLOWEE);\n");
	search_user_by_name_mongo("Rowen Felt");
	printf("search_user('rowen', INSTA_UNKNOWN);\n");
	search_user_by_name_cass("rowen");
	printf("search_user('campbell', INSTA_UNKNOWN)\n");
	search_user_by_name_cass("campbell");
	printf("\n\n\n");		

	return 0;
}

