/*
 * network_protocols.c
 * text-based protocals for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
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
#include "network_protocols.h"
#include "insta_user_definitions.h"
#include "insta_dispatch_definitions.h"


/*
 * Uses a series of memory comparison to 'parse' the memory location referenced by 'command'.
 * If the memory comparision is sucessful, then one of the server sub functions is called.
 * The subfunctions, defined in this library, will utilized the mongo insta libraries
 * (insta_user_definitions.so and insta_dispatch_definitions.so) to query the insta mongo 
 * database and get bson documents which contian the relevant information that the client
 * software requested. The return type is currently ambigious, probably will return an int
 * and call a method to send bson over the ineret from within on of the submethods...
 */  
int
parse_client_command(char *command){	
	int result;
printf("command recieved: '%s'\n", command);	
	
	if(memcmp(command, "pull all ", 9) == 0){
puts("pull all");
		uint64_t user_id;
		user_id = strtoll((command + 9), NULL, 10); //Assuming base 10
		//checking for possible over or underflow, or no valid integer as a string was found,
		//in which case strtoll returns 0, and 0 is not a valid user id... 
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull all: ");
			return -1;
		}
printf("user id: %ld\n", user_id);
		result = pull_all(user_id);
	}
	
			
	else if(memcmp(command, "pull child ", 11) == 0){
puts("pull child");
		uint64_t parent_id;
		parent_id = strtoll((command + 11), NULL, 10);
		if(parent_id == LLONG_MIN || parent_id == LLONG_MAX || parent_id == 0 ){
			perror("Invalid arguments for pull child: ");
			return -1;
		}
printf("parent id: %ld\n",parent_id);
		result = pull_child(parent_id);
	}
	

	else if(memcmp(command, "pull one ", 9) == 0){
puts("pull one");
		char* endptr;
		uint64_t user_id, dispatch_id;
		user_id = strtoll((command + 9), &endptr, 10);
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull one: ");
			return -1;
		}
		dispatch_id = strtoll(endptr, NULL, 10); //ignore leading whitespace
		if(dispatch_id == LONG_MIN || dispatch_id == LLONG_MAX || dispatch_id == 0){
			perror("Invalid arguments: ");
			return -1;
		}
printf("user id: %ld, dispatch_id: %ld\n", user_id, dispatch_id);
		result = pull_one(user_id, dispatch_id);
	}


	else if(memcmp(command, "pull user ", 10) == 0){
puts("pull user");
		uint64_t user_id;
		user_id = strtoll((command + 10), NULL, 10);
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull user: ");
			return -1;
		}
printf("user id: %ld\n", user_id);
		result = pull_user(user_id);
	}


	else if(memcmp(command, "pull search ", 12) == 0){
puts("pull search");
		char *field, *query;
		char *saveptr;
		if((field = strtok_r((command + 12), " ", &saveptr))  == NULL){
			printf("Invalid argument for pull search\n");
			return -1;
		}
		//check that the pointer field references a valid field ("user_tags" or "tags")
		if(memcmp(field, "user_tags", 9) != 0 && memcmp(field, "tags", 4) != 0){
			printf("Invalid value for field\n");
			return -1;
		}
		
		//currently not preforming any checking to see if this is somehow a nefarious 
		//chunk of data, and strtok don't really provide any robust support in this regard.
		if((query = strtok_r(NULL, " ", &saveptr))  == NULL){
			printf("Invalid argument for pull search\n");
			return -1;
		}
printf("field: %s, query: %s\n", field, query);			
		result =	pull_search(field, query);
	}

	return result;
}

/*
 * Pulls all the dispatches from the user referenced by user id, where all is 
 * defined as some predefined quantity of dispatches.
 */
int
pull_all(uint64_t user_id){
	return 0;
}


/*
 * Pulls all of the 'children' dispatches for a dispatched referenced by parent_id, 
 * where children dispatches are defined as dispatches with he parent given by 
 * parent_id
 */
int
pull_child(uint64_t parent_id){
	
	return 0;
}


/*
 * Pulls a single dispatch referenced by user_id and dispatch_id -- the combination of these
 * two variable should act as a unique reference to a singule dispatch (assuming no
 * duplication in a database
 */
int
pull_one(uint64_t user_id, uint64_t dispatch_id){
	return 0;
}


/*
 * Queries the mongo user collection for a user with an id matching 'user_id', and pulls that
 * user's information (presumably as a bson).
 */
int
pull_user(uint64_t user_id){
	return 0;
}



/*
 * Queries the mongo dispatch collection for a dispatch (or multiple dispatches), which 
 * have a value that matches query, and which is stored in the 'column'/has the key pair
 * referenced by field. 
 */
int
pull_search(char *field, char* query){
	return 0;
}
