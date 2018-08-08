/*
 * user_tests.c
 * tests basic insta user methods using mongoDB and cassandra
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "user_definitions.h"
#include "util.c"
#include "mongo_connect.h"

int 
main(int argc, char *argv[])
{
	/* initialize test user structs */
	struct user cb;
	struct personal_data pd;
	struct insta_relations followers;
	struct insta_relations following;
	cb.user_id = 12345;
	cb.username = "cboswell";
	cb.image = (uint8_t *) "/images/cboswell_image/fake_path";
	cb.image_length = strlen((char *) cb.image);
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
	
	struct user test;
  struct personal_data test_pd;
  struct insta_relations test_followers;
  struct insta_relations test_following;
  test.user_id = 9999;
  test.username = "grunge";
  test.image = (uint8_t *) "/images/darkness";
	test.image_length = strlen((char *) test.image);
  test_pd.name = "Kthulu";
  test.bio = &test_pd;
  test.fragmentation = 0;
  test_followers.direction = 0;
  test_followers.count = 0;
  test.followers = &test_followers;
  test_following.direction = 1;
  test_following.count = 1;
  uint64_t test_following_ids[1] = {666};
  test_following.user_ids = test_following_ids;
  test.following = &test_following;

	struct user rf;
	struct personal_data rf_pd;
	struct insta_relations rf_followers;
	struct insta_relations rf_following;
	rf.user_id = 11254155;
	rf.username = "rfelt";
	rf.image = (uint8_t *) "/images/rfelt_image/fake_path";
	rf.image_length = strlen((char *) rf.image);
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
	pj.username = "pjohnson";
	pj.image = (uint8_t *) "/images/pjohnson_image/fake_path";
	pj.image_length = strlen((char *) pj.image);
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
	
	struct user pj_imposter;
	struct personal_data pj_imposter_pd;
	struct insta_relations pj_imposter_followers;
	struct insta_relations pj_imposter_following;
	pj_imposter.user_id = 159179;
	pj_imposter.username = "pjohnson_imposter";
	pj_imposter.image = (uint8_t *) "/images/pjohnson_image/fake_path";
	pj_imposter.image_length = strlen((char *) pj_imposter.image);
	pj_imposter_pd.name = "Pete Johnson";
	pj_imposter.bio = &pj_imposter_pd;
	pj_imposter.fragmentation = 0;
	pj_imposter_followers.direction = 0;
	pj_imposter_followers.count = 2;
	uint64_t pj_imposter_follower_ids[2] = {16, 88};
	pj_imposter_followers.user_ids = pj_imposter_follower_ids;
	pj_imposter.followers = &pj_imposter_followers;
	pj_imposter_following.direction = 1;
	pj_imposter_following.count = 9;
	uint64_t pj_imposter_following_ids[9] = {66666, 64, 346, 15235, 2356, 6, 2, 3, 99};
	pj_imposter_following.user_ids = pj_imposter_following_ids;
	pj_imposter.following = &pj_imposter_following;
	
	/* test insert_user */
	int result, req_num, num_failed, length;
	char *buf;

	req_num = -1; //The number of results requested in queries
	num_failed = 0;

	/* test insert_user */
	insert_user(&cb); //User object has expected strlen of 457 as json
	insert_user(&rf); //User object has expected strlen of 453 as json
	insert_user(&pj); //User object has expected strlen of 478 as json
	insert_user(&pj_imposter);		//has expected strlen of 486 as json	
	insert_user(&test);

	/* test search_user */
	buf = search_user_by_name_mongo("cboswell", req_num, &result, &length);
	if(strlen(buf) != 871 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_for_user_by_name_mongo('cboswell', req_num, &result, &length)\n");
	
	
	buf = search_user_by_id_mongo(12345, req_num, &result, &length);
	if(strlen(buf) != 871 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_for_user_by_id_mongo(12345, req_num, &result, &length)\n");
	
	
	if(delete_user(12345) != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("delete_user(12345)\n");
	

	buf = search_user_by_id_mongo(12345, req_num, &result, &length);
	if(buf != NULL || result != 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_id_mongo(12345) where 12345 is not a valid id\n");
	

	buf = search_user_by_name_mongo("rfelt", req_num, &result, &length);
	if(strlen(buf) != 866 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_name_mongo('rfelt', req_num, &result, &length)\n");
	
	
	buf = search_user_by_name_mongo("Rowen Felt", req_num, &result, &length);
	if(strlen(buf) != 866 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_name_mongo('Rowen Felt', req_num, &result, &length)\n");
	

	buf = search_user_by_name_mongo("Pete Johnson", req_num, &result, &length);
	if(strlen(buf) != 1961 || result != 2){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_name_mongo('Pete Johson', req_num, &result, &length)\n");
	

	buf = search_user_by_id_mongo(159179, req_num, &result, &length);
	if(strlen(buf) != 985 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_id_mongo(159179, req_num, &result, &length)\n");


	buf = search_user_by_id_mongo(159178, req_num, &result, &length);

	if(strlen(buf) != 976 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_user_by_id_mongo(159178, req_num, &result, &length)\n");
	
	//Open the test file
  int fd;
  if((fd = open("json_user_test.txt", O_CREAT | O_RDWR, 0666)) == -1){
    perror("open");
    return -1;
  }

  //attempt to read from fd and store bsons in a collection
  if(insert_json_from_fd(fd, USER_COLLECTION) <= 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
  printf("insert from JSON\n");

  free(buf);

 	if(num_failed == 0){
		printf("ALL TESTS SUCCESSFUL\n");
	}
	
	return 0;
}

