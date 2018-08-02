/*
 * test_cleanup.c
 * removes all users and dispatches from 
 * user_tests, dispatch_tests, and
 * network_protocol_tests
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include <stdio.h>
#include "user_definitions.h"
#include "dispatch_definitions.h"

#define NUM_USER_IDS 4
#define NUM_DISPATCH_IDS 7

int 
main(int argc, char *argv[])
{
	uint64_t dispatch_ids[] = 
		{1, 2, 3, 14351614, 6969, 6667, 6666};
	uint64_t user_ids[] = 
		{159179, 9999, 159178, 11254155};
	
	for(int i = 0; i < NUM_DISPATCH_IDS; i++){
		delete_dispatch(dispatch_ids[i]);
	}
	for(int i = 0; i < NUM_USER_IDS; i++){
		delete_user(user_ids[i]);
	}
}	
