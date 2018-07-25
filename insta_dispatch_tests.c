/*
 * insta_dispatch_tests.c
 * Temporary testing file for insta_dispatch_definitions.c methods
 */

#include <mongoc.h>
#include <bson.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "insta_dispatch_definitions.h"
#include "util.h"
#include "insta_mongo_connect.h"

int
main(int argc, char* argv[])
{
	/*
	 * Testing insert_dispatch method in insta_dispatch_definitions.c 
	 */


	/* 1 original dispatch with cat */ 
	struct dispatch *dis = malloc(sizeof(struct dispatch));
	struct dispatch_body *body = malloc(sizeof( struct dispatch_body));
	struct dispatch_parent *parent = malloc(sizeof(struct dispatch_parent));
	
	body->media_path = "/home/cboswell/Desktop/cat.jpg";
	body->text = "Cat";
	dis->body = body;

	dis->user_id = 1234;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 3;	

	strcpy(dis->tags[0], "cute");
	strcpy(dis->tags[1], "lolcats");
	strcpy(dis->tags[2], "wow");
	
	dis->num_user_tags = 4;

	dis->user_tags[0] = 1;	
	dis->user_tags[1] = 2;
	dis->user_tags[2] = 3;
	dis->user_tags[3] = 4;
	
	parent->type = 0;
	parent->id = 1234;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 6666;

	insert_dispatch(dis);

	free(dis);
	free(body);
	free(parent);
	

	/* 2nd dispatch from user 1234 */		
	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));

	body->media_path = "/home/cboswell/Desktop/doOOOOOOg.jpg";
	body->text = "Dog";
	dis->body = body;

	dis->user_id = 1234;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 3;	

	strcpy(dis->tags[0], "cute");
	strcpy(dis->tags[1], "loldogs");
	strcpy(dis->tags[2], "wow");
	
	dis->num_user_tags = 4;

	dis->user_tags[0] = 1;	
	dis->user_tags[1] = 2;
	dis->user_tags[2] = 3;
	dis->user_tags[3] = 4;
	
	parent->type = 0;
	parent->id = 1234;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 6667;

	insert_dispatch(dis);

	free(dis);
	free(body);
	free(parent);
	
	/* 3 - dispach with shared parent id */
	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));

	body->media_path = "comment:";
	body->text = "Cool cat";
	dis->body = body;

	dis->user_id = 3;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 0;	

	dis->num_user_tags = 2;

	dis->user_tags[0] = 1;	
	dis->user_tags[1] = 4;
	
	parent->type = 1;
	parent->id = 6666;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 1;

	insert_dispatch(dis);
	free(dis);
	free(body);
	free(parent);

	/* 4 - dispatch with shared parent id */
	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));

	body->media_path = "comment:";
	body->text = "Nice smile!";
	dis->body = body;

	dis->user_id = 2;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 0;	
	
	dis->num_user_tags = 0;
	
	parent->type = 1;
	parent->id = 6666;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 2;

	insert_dispatch(dis);
	free(dis);
	free(body);
	free(parent);
	
	/* 5 - dispatch with shared parent id */

	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));
	
	body->media_path = "comment:";
	body->text = "wow, great picture";
	dis->body = body;

	dis->user_id = 4;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 0;	
	
	dis->num_user_tags = 0;
	
	parent->type = 1;
	parent->id = 6666;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 3;

	insert_dispatch(dis);
	free(dis);
	free(body);
	free(parent);
	
	/* 6 - dispatch is a duplicate of dispatch 3, and should not be inerted*/

	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));
	
	body->media_path = "comment:";
	body->text = "wow, great picture";
	dis->body = body;

	dis->user_id = 4;
	dis->timestamp = time(NULL);
	dis->audience_size = 0;

	dis->num_tags = 0;	
	
	dis->num_user_tags = 0;
	
	parent->type = 1;
	parent->id = 6666;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 3;

	insert_dispatch(dis);
	free(dis);
	free(body);
	free(parent);
	
	/* 7 - dispatch with audience */

	dis = malloc(sizeof(struct dispatch));
	body = malloc(sizeof( struct dispatch_body));
	parent = malloc(sizeof(struct dispatch_parent));


	body->media_path = "/home/cboswell/Desktop/nude.jpg";
	body->text = "my nudes";
	dis->body = body;

	dis->user_id = 1234;
	dis->timestamp = time(NULL);
	dis->audience_size = 2;
	dis->audience[0] = 4;
	dis->audience[1] = 19;

	dis->num_tags = 2;	

	strcpy(dis->tags[0], "wokeuplikethis");
	strcpy(dis->tags[1], "aesthetic");
	
	dis->num_user_tags = 0;

	parent->type = 0;
	parent->id = 1234;
	dis->parent = parent;

	dis->fragmentation = 5;
	dis->dispatch_id = 6969;

	insert_dispatch(dis);

	free(dis);
	free(body);
	free(parent);

	/* 8 - dispatch with identical audience */
 
  dis = malloc(sizeof(struct dispatch));
  body = malloc(sizeof( struct dispatch_body));
  parent = malloc(sizeof(struct dispatch_parent));


  body->media_path = "/home/cboswell/Desktop/brooding.jpg";
  body->text = "dudes who brood aren't rude, just misunderstude";
  dis->body = body;

  dis->user_id = 1000;
  dis->timestamp = time(NULL);
  dis->audience_size = 2;
  dis->audience[0] = 4;
  dis->audience[1] = 19;

  dis->num_tags = 2;

  strcpy(dis->tags[0], "angstyteen");
  strcpy(dis->tags[1], "antiaesthetic");

  dis->num_user_tags = 0;

  parent->type = 0;
  parent->id = 1234;
  dis->parent = parent;

  dis->fragmentation = 5;
  dis->dispatch_id = 14351614;

  insert_dispatch(dis);

  free(dis);
  free(body);
  free(parent);


	/* there should only be a total of 6 dispatches inserted */
	
	/* search by parent's dispatch id */
	int result, num_failed;

	num_failed = 0;

	char *buf = search_dispatch_by_parent_id( 6666, -1, &result);		
	if(strlen(buf) != 1742 || result != 3){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_parent_id(6666, -1, &result)\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	
	//Testing insert_json_from_fd funcion from mongo_connect.c
	//---------------------------------------------------------------------
	
	//Open the test file containing dispatch json's
	int fd;
	if((fd = open("json_dispatch_test.txt", O_CREAT | O_RDWR, 0666)) == -1){
		perror("open");
		return -1;
	}
	
	//NOTE: the insert_json_from_fd method does NOT work if fd has just been 
	//written to...

	/*
	 * Uncomment to write to json_dispatch_test.txt if file is not present or 
	 * needs to be updated
	 */
	//	if(write(fd, buf, 1742) != 1742){
	//		perror("write");
	//		return -1;
	//	}
	
	/*
	 * Requires 3 random bytes to have been writen in json_dispatch test.txt 
	 * first to test this part. Comment out if leading bytes not present
	 */
	int num_leading_bytes = 3;
	char *leading_bytes = malloc(num_leading_bytes);
	read(fd, leading_bytes, num_leading_bytes);

	//attempt to read from fd and store data as bson documents in the
	//dispatch collection
	if(insert_json_from_fd(fd, DISPATCH_COLLECTION) <= 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("insert from JSON\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);

	//---------------------------------------------------------------------

	buf = search_dispatch_by_id(1, -1, &result);
	if(strlen(buf) != 607 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
		printf("search_dispatch_by_id(1, -1, &result)\n");
		printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);	

	buf = search_dispatch_by_id(20, -1, &result);
	if(buf != NULL || result != 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_id(20, -1, &result)\n");
	free(buf);

	buf = search_dispatch_by_user_audience(1234, NULL, 0, -1, &result);
	if(strlen(buf) != 1410 || result != 2){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_user_audience(1234, NULL, O, 4, &result)\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);


	uint64_t aud[2] = {4, 19};
	buf = search_dispatch_by_user_audience(1234, aud, 2, -1, &result);
	if(strlen(buf) != 666 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_user_audience(1234, aud, 2, -1, &result\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);


	buf = search_dispatch_by_tags("angstyteen", -1, &result);
	if(strlen(buf) != 713 || result != 1){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_tags('angstyteen', -1, &result)\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);
	

	buf = search_dispatch_by_user_tags(3, -1, &result);
	if(strlen(buf) != 1410 || result != 2){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCESS: ");
	}
	printf("search_dispatch_by_user_tags(3, -1, &result)\n");
	printf("	buf length = %ld\n	result = %d\n", strlen(buf), result);
	free(buf);

	if(num_failed == 0){
		printf("ALL TESTS SUCESSFUL\n");
	}


	/* delete dispatches */
//	delete_dispatch(6969);
//	delete_dispatch(6666);
//	delete_dispatch(3);
//	delete_dispatch(2);
//	delete_dispatch(1);
//	delete_dispatch(6667);
//	delete_dispatch(14351614);
}


