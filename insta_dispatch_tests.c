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

	/* there should only be a total of 6 dispatches inserted */
	
	/* search by parent's dispatch id */
	int result;

	printf("\n\nSEARCH FOR DISPATCH BY PARENT ID\n");
	char *buf = search_dispatch_by_parent_id( 6666, 4, &result);		
	printf("\nthere were %d results. ", result);
	printf("The contents of buf are:\n%s", buf);	
	free(buf);

	printf("\n\nSEARCH FOR DISPATCH BY ID\n");
	buf = search_dispatch_by_id(1, &result);
	printf("\n\nThe dispatch id is 1\nThe contents of buf are:\n%s", buf);	
	free(buf);	

	buf = search_dispatch_by_id(20, &result);
	printf("\n\nThe dispatch id is 20 The contents of buf are:\n%s", buf);	
	free(buf);

	printf("\n\nSEARCH FOR DISPATCH BY USER AND NO AUDIENCE\n");
	buf = search_dispatch_by_user_audience(1234, NULL, 0, 4, &result);
	printf("\n\nLooking for dispatches from user 1234 with public audience (0). The contents of buf are:\n%s\n", buf);
	free(buf);

	printf("\n\nSEARCH FOR DISPATCH BY USER WITH AUDIENCE\n");
	uint64_t aud[] = {4, 19};
	buf = search_dispatch_by_user_audience(1234, aud, 2, -1, &result);
	printf("\n\n Searching for user 1234 with public audience [4,19]. The contents of buf are: \n%s\n", buf);  		
	
	/* delete dispatches */
	// delete_dispatch(6969);
	delete_dispatch(6666);
	delete_dispatch(3);
	//delete_dispatch(2);
	delete_dispatch(1);
	delete_dispatch(6667);
}


