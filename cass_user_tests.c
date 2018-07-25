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

int main(int argc, char *argv[])
{
	char *id;

  printf("initialize keyspace and table\n");
	keyspace_table_init(KEYSPACE, TABLE);

	printf("add_user(12521512, 'rowen', '4.5.6.7')\n");
	add_user(12521512, "rowen", "4.5.6.7");
	
	printf("add_user(713717, 'rowen', '1.2.3.4')\n");
	add_user(713717, "rowen", "1.2.3.4");
	
	printf("add_user(1435, 'ricker', '140.233.20.181')\n");
	add_user(1435, "ricker", "140.233.20.181");
	add_user(1485, "ellen", "140.233.20.153");
	
	printf("get_user_ip_by_username(KEYSPACE, TABLE, 'rowen')\n");
	get_user_ip_by_username(KEYSPACE, TABLE, "rowen");

	printf("get_user_ip_by_id(KEYSPACE, TABLE, 713717)\n");
	id = get_user_ip_by_id(KEYSPACE, TABLE, 713717);
	printf("id = %s\n", id);
	free(id);
	return 0;
}