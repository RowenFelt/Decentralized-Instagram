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
#include "insta_mongo_connect.h"
#include "cass_user.h"
#include "insta_user_definitions.h"


#define INSTA_DB "insta"
#define USER_COLLECTION "users"

int 
init_user(void)
{
	return 0;
}

int 
search_user(char *username, int flags)
{
	/* search for a user in the mongoDB list of followers followees or the Cassandra
	 * database of users using the respective flags INSTA_FOLLOWER, INSTA_FOLLOWEE,
	 * INSTA_UNKNOWN */ 
	struct mongo_user_connection cn;
	mongoc_cursor_t *cursor;
	bson_t *query;
	const bson_t *doc;
	bson_error_t error;
	char *str;

	if(flags == INSTA_FOLLOWER || flags == INSTA_FOLLOWEE) {
		cn.uri_string = "mongodb://localhost:27017";
		mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION);
		query = BCON_NEW (
			"$or", "[",
			"{", "username", BCON_UTF8(username), "}",
			"{", "name", BCON_UTF8(username), "}",
			"]"	
		);
		cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
		while (mongoc_cursor_next (cursor, &doc)) {
      str = bson_as_canonical_extended_json (doc, NULL);
      printf ("%s\n", str);
      bson_free (str);
		}
		if (mongoc_cursor_error (cursor, &error)) {
      fprintf (stderr, "An error occurred: %s\n", error.message);
		}

		mongoc_cursor_destroy (cursor);
		bson_destroy (query);
		return mongo_user_teardown(&cn);
	}
	else if(flags == INSTA_UNKNOWN){
		// fill out cassandra code
	}
	return 0;
}

int
insert_user(struct user *new_user)
{
	return 0;
}

int
delete_user(uint64_t user_id)
{
	return 0;
}

int 
pull_user_profile(uint64_t user_id)
{
	return 0;
}
