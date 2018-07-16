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
 
	struct dispatch dis;
	struct dispatch_body body;
	struct dispatch_parent parent;
	
	body.media_path = "/home/cboswell/Desktop/cat.jpg";
	body.text = "Cat";
	dis.body = &body;

	dis.user_id = 1234;
	dis.timestamp = time(NULL);
	dis.audience_size = 0;

	dis.num_tags = 3;	

	strcpy(dis.tags[0], "cute");
	strcpy(dis.tags[1], "lolcats");
	strcpy(dis.tags[2], "wow");
	
	dis.num_user_tags = 4;

	dis.user_tags[0] = 1;	
	dis.user_tags[1] = 2;
	dis.user_tags[2] = 3;
	dis.user_tags[3] = 4;
	
	parent.type = 0;
	parent.id = 1234;
	dis.parent = &parent;

	dis.fragmentation = 5;
	dis.dispatch_id = 6666;

	insert_dispatch(&dis);

	/* 1 - dispach with shared parent id */

	body.media_path = "comment:";
	body.text = "Cool cat";
	dis.body = &body;

	dis.user_id = 3;
	dis.timestamp = time(NULL);
	dis.audience_size = 0;

	dis.num_tags = 0;	

	dis.num_user_tags = 2;

	dis.user_tags[0] = 1;	
	dis.user_tags[1] = 4;
	
	parent.type = 1;
	parent.id = 6666;
	dis.parent = &parent;

	dis.fragmentation = 5;
	dis.dispatch_id = 1;

	insert_dispatch(&dis);

	/* 2 - dispatch with shared parent id */

	body.media_path = "comment:";
	body.text = "Nice smile!";
	dis.body = &body;

	dis.user_id = 2;
	dis.timestamp = time(NULL);
	dis.audience_size = 0;

	dis.num_tags = 0;	
	
	dis.num_user_tags = 0;
	
	parent.type = 1;
	parent.id = 6666;
	dis.parent = &parent;

	dis.fragmentation = 5;
	dis.dispatch_id = 2;

	insert_dispatch(&dis);
	
	/* 3 - dispatch with shared parent id */

	body.media_path = "comment:";
	body.text = "wow, great picture";
	dis.body = &body;

	dis.user_id = 4;
	dis.timestamp = time(NULL);
	dis.audience_size = 0;

	dis.num_tags = 0;	
	
	dis.num_user_tags = 0;
	
	parent.type = 1;
	parent.id = 6666;
	dis.parent = &parent;

	dis.fragmentation = 5;
	dis.dispatch_id = 3;

	insert_dispatch(&dis);
	

	/* search by parent's dispatch id */
	search_dispatch_by_parent_id((uint64_t) 6666, 3);	

}

