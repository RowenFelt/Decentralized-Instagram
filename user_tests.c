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
	
	struct user test;
  struct personal_data test_pd;
  struct insta_relations test_followers;
  struct insta_relations test_following;
  test.user_id = 9999;
  test.username = "grunge";
  test.image_path = "/images/darkness";
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
	pj.username = "pjohnson";
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
	
	struct user pj_imposter;
	struct personal_data pj_imposter_pd;
	struct insta_relations pj_imposter_followers;
	struct insta_relations pj_imposter_following;
	pj_imposter.user_id = 159179;
	pj_imposter.username = "pjohnson_imposter";
	pj_imposter.image_path = "/images/pjohnson_image/fake_path";
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
	int result, req_num, num_failed;
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
	buf = search_user_by_name_mongo("cboswell", req_num, &result);
	if(strlen(buf) != 774 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_for_user_by_name_mongo('cboswell', req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	
	
	buf = search_user_by_id_mongo(12345, req_num, &result);
	if(strlen(buf) != 774 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_for_user_by_id_mongo(12345, req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	
	
	if(delete_user(12345) != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("delete_user(12345)\n");
	

	buf = search_user_by_id_mongo(12345, req_num, &result);
	if(buf != NULL || result != 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_id_mongo(12345) where 12345 is not a valid id\n");
	

	buf = search_user_by_name_mongo("rfelt", req_num, &result);
	if(strlen(buf) != 770 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_name_mongo('rfelt', req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	
	
	buf = search_user_by_name_mongo("Rowen Felt", req_num, &result);
	if(strlen(buf) != 770 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_name_mongo('Rowen Felt', req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	

	buf = search_user_by_name_mongo("Pete Johnson", req_num, &result);
	if(strlen(buf) != 1767 || result != 2){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_name_mongo('Pete Johson', req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	

	buf = search_user_by_id_mongo(159179, req_num, &result);
	if(strlen(buf) != 888 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_id_mongo(159179, req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);


	buf = search_user_by_id_mongo(159178, req_num, &result);
	if(strlen(buf) != 879 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_user_by_id_mongo(159178, req_num, &result)\n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);
	

	//Testing insert_json_from_fd funcion from util.c, which uses users 
  //generated in this test file
  //---------------------------------------------------------------------

  //Open the test file
  int fd;
  if((fd = open("json_user_test.txt", O_CREAT | O_RDWR, 0666)) == -1){
    perror("open");
    return -1;
  }

  //NOTE: the insert_json_from_fd method does NOT work if fd has just been 
  //written to...not sure why

  /* Uncomment to write to json_test.txt  initially */
  //  if(write(fd, buf, 879) != 879){
  //    perror("write");
  //    return -1;
  //  }

  //attempt to read from fd and store bsons in a collection
  if(insert_json_from_fd(fd, USER_COLLECTION) <= 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
  printf("insert from JSON/n");
  printf("  buf length = %ld\n  result = %d\n", strlen(buf), result);

  free(buf);

  //---------------------------------------------------------------------




	//search_user_by_name_cass("rowen");
	//search_user_by_name_cass("campbell");

//	delete_user(11254155);
//	delete_user(159179);

	if(num_failed == 0){
		printf("ALL TESTS SUCESSFUL\n");
	}
	
	return 0;
}

