/*
 * cass_user_tests.c
 * tests basic cassandra methods
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <cassandra.h>
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

#include "cass_user.h"

#define KEYSPACE "insta"
#define TABLE "user"

struct user_addition {
	uint64_t id;
	char name[20];
	char ip[20];
};

int main(int argc, char *argv[])
{
	char *id;
	uint64_t *user_ids;
	int result = 0;
	int failed = 0;
	int n = 0;

	keyspace_table_init(KEYSPACE, TABLE);

	struct user_addition tests[] = {
		{12521512, "rowen", "4.5.6.7"},
		{713717, "rowen", "1.2.3.4"},
		{1435, "ricker", "140.233.20.181"},
		{1485, "ellen", "140.233.20.153"}
	};

	for(int i = 0; i < 4; i++){
		n = add_user(tests[i].id, tests[i].name, tests[i].ip);
		if(n != 0){
			printf("TEST FAILED: add_user\n");
			failed += 1;
		}
		else{
			printf("TEST SUCCESSFUL: add_user\n");
		}
	}
		
	user_ids = get_user_id_by_username(KEYSPACE, TABLE, "rowen", &result);	
	if(user_ids[0] != 713717 || user_ids[1] != 12521512){
		printf("TEST FAILED: get_user_id_by_username\n");
		failed += 1;
	}	
	else{
		printf("TEST SUCCESSFUL: get_user_id_by_username\n");
	}

	id = get_user_ip_by_id(KEYSPACE, TABLE, 713717);
	if(memcmp(id, "1.2.3.4", 7) != 0){
		printf("TEST FAILED: get_user_ip_by_id\n");
		failed += 1;
	}
	else{
		printf("TEST SUCCESSFUL: get_user_ip_by_id\n");
	}

	if(failed != 0){
		printf("%d TESTS FAILED\n", failed);
	}
	else{
		printf("ALL TESTS SUCCESSFUL\n");
	}	
	free(id);
	return 0;
}
