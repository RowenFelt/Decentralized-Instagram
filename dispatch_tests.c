/*
 * dispatch_tests.c
 * Temporary testing file for dispatch_definitions.c methods
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
#include "dispatch_definitions.h"
#include "mongo_connect.h"

int
main(int argc, char* argv[])
{
	/*
	 * Testing insert_dispatch method in dispatch_definitions.c 
	 */


	/* 1 original dispatch with cat */ 
	struct dispatch *dis = malloc(sizeof(struct dispatch));
	struct dispatch_body *body = malloc(sizeof( struct dispatch_body));
	struct dispatch_parent *parent = malloc(sizeof(struct dispatch_parent));
	
	body->media =(uint8_t*) "/home/cboswell/Desktop/cat.jpg";
	body->media_size = strlen((char *)body->media); 
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

	body->media = (uint8_t *)"/home/cboswell/Desktop/doOOOOOOg.jpg";
	body->media_size = strlen((char *)body->media); 
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

	body->media = (uint8_t *)"comment:";
	body->media_size = strlen((char *)body->media); 
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

	body->media = (uint8_t *)"comment:";
	body->media_size = strlen((char *)body->media); 
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
	
	body->media = (uint8_t *)"comment:";
	body->media_size = strlen((char *)body->media); 
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
	
	body->media = (uint8_t *)"comment:";
	body->media_size = strlen((char *)body->media); 
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


	body->media = (uint8_t *)"/home/cboswell/Desktop/nude.jpg";
	body->media_size = strlen((char *)body->media); 
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


  body->media = (uint8_t *)"/home/cboswell/Desktop/brooding.jpg";
	body->media_size = strlen((char *)body->media); 
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
	if(strlen(buf) != 2000 || result != 3){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_parent_id(6666, -1, &result)\n"
				"\tbuf_size = %ld\n", strlen(buf));
	//Testing insert_json_from_fd funcion from util.c, which uses dispatches 
	//generated in this test file
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
	//	if(write(fd, buf, 2000) != 2000){
	//		perror("write");
	//		return -1;
	//	}
	
	/*
	 * Requires 3 random bytes to have been writen in json_dispatch test.txt 
	 * first to test this part. Comment out if leading bytes not present
	 */
//	int num_leading_bytes = 3;
//	char *leading_bytes = malloc(num_leading_bytes);
//	read(fd, leading_bytes, num_leading_bytes);

	//attempt to read from fd and store data as bson documents in the
	//dispatch collection
	if(insert_json_from_fd(fd, DISPATCH_COLLECTION) <= 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("insert from JSON\n");
	free(buf);

	//---------------------------------------------------------------------

	buf = search_dispatch_by_id(1, -1, &result);
	if(strlen(buf) != 693 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_id(1, -1, &result)\n"
			"\tbuf_size = %ld\n", strlen(buf));
	free(buf);	

	buf = search_dispatch_by_id(20, -1, &result);
	if(buf != NULL || result != 0){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_id(20, -1, &result)\n");
	free(buf);

	buf = search_dispatch_by_user_audience(1234, NULL, 0, -1, &result);
	if(strlen(buf) != 1598 || result != 2){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_user_audience(1234, NULL, O, 4, &result)\n" 
					"\tbuf_size = %ld\n", strlen(buf));
	free(buf);


	uint64_t aud[2] = {4, 19};
	buf = search_dispatch_by_user_audience(1234, aud, 2, -1, &result);
	if(strlen(buf) != 762 || result != 1){
		printf("TEST FAILED: ");
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_user_audience(1234, aud, 2, -1, &result\n"
				"\tbuf_size = %ld\n", strlen(buf));
	free(buf);


	buf = search_dispatch_by_tags("angstyteen", -1, &result);
	if(strlen(buf) != 809 || result != 1){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_tags('angstyteen', -1, &result)\n"
				"\tbuf_size = %ld\n", strlen(buf));
	free(buf);
	

	buf = search_dispatch_by_user_tags(3, -1, &result);
	if(strlen(buf) != 1598 || result != 2){
		printf("TEST FAILED: "); 
		num_failed++;
	}
	else{
		printf("TEST SUCCESS: ");
	}
	printf("search_dispatch_by_user_tags(3, -1, &result)\n" 
				"\tbuf_size = %ld\n", strlen(buf));
	free(buf);

	if(num_failed == 0){
		printf("ALL TESTS SUCCESSFUL\n");
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


