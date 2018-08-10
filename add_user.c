/*
  * add_user.c
  * Adds a new user to cassandra insta.user
  * Authors: Rowen Felt and Campbell Boswell
  */
 
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "user_definitions.h"
#include "cass_user.h"

int
main(int argc, char *argv[])
{
	char *username;
	char *ip;
	uint64_t user_id;
	int n;

	user_id = atoll(argv[1]);
	username = argv[2]; 
	ip = argv[3];

	keyspace_table_init("insta", "user");
	n = add_user(user_id, username, ip);
	if(n < 0){
		printf("Error inserting user into Cassandra\n");
		return n;
	}
	
	return 0;
		
}

