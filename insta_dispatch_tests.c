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
	dis.audience_size = 3;
	for (int i = 0; i < dis.audience_size; i++){
		dis.audience[i] = (i + 1)*1000;
	}

	dis.num_tags = 3;	

	strcpy(dis.tags[0], "cute");
	strcpy(dis.tags[1], "lolcats");
	strcpy(dis.tags[2], "wow");
	
	dis.num_user_tags = 4;

	dis.user_tags[0] = 1111;	
	dis.user_tags[1] = 2222;
	dis.user_tags[2] = 3333;
	dis.user_tags[3] = 4444;
	
	parent.type = 0;
	parent.id = 1234;
	dis.parent = &parent;

	dis.fragmentation = 5;
	dis.dispatch_id = 9949;

	insert_dispatch(&dis);

	dis.dispatch_id = 8734;
	insert_dispatch(&dis);
	
	/*
	 * Testing delete_dispatch method in insta_dispatch_definions.c 
	 */

	uint64_t dispatch_id = 9949; 
	
//	delete_dispatch(dispatch_id);

	printf("\n\n\nprinting dipatch with id %ld\n", dispatch_id);
	
	search_dispatch_by_id(dispatch_id);	

}

