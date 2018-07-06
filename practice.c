/*
 * practice.c
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

int main(int argc, char *argv[])
{
  keyspace_table_init("insta", "user");
	add_user("rowen", "1.2.3.4");
	return 0;
}
