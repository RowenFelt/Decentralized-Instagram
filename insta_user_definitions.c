/*
 * insta_user_definitions.c
 * user object and methods for Mongo implementation
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <mongoc.h>
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

int 
init_user(void)
{
	return 0;
}



int 
search_user(char *username)
{
	return 0;
}


int 
pull_user_profile(uint64_t user_id)
{
	return 0;
}
