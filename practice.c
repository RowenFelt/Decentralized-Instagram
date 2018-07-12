/*
 * practice.c
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
  keyspace_table_init(KEYSPACE, TABLE);
	add_user(12521512, "rowen", "4.5.6.7");
	add_user(713717, "rowen", "1.2.3.4");
	get_user_ip_by_username(KEYSPACE, TABLE, "rowen");
	get_user_ip_by_id(KEYSPACE, TABLE, 7137170);
	return 0;
}
