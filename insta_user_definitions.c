/*
 * insta_user_definitions.c
 * user object and methods for Mongo implementation
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
#include "insta_mongo_connect.h"
#include "cass_user.h"
#include "insta_user_definitions.h"


#define INSTA_DB "insta"
#define USER_COLLECTION "users"
#define CASS_TABLE "user"

int 
init_user(void)
{
	return 0;
}

int 
search_user_by_name(char *username, int flags)
{
	/* search for a user in the mongoDB list of followers followees or the Cassandra
	 * database of users using the respective flags INSTA_FOLLOWER, INSTA_FOLLOWEE,
	 * INSTA_UNKNOWN. Return number of results found or -1 on failure */ 
	struct mongo_user_connection cn;
	mongoc_cursor_t *cursor;
	bson_t *query;
	const bson_t *doc;
	bson_error_t error;
	char *str;
	int result = 0;
	int n;

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
      result+=1;
			str = bson_as_canonical_extended_json (doc, NULL);
      printf ("%s\n", str);
      bson_free (str);
		}
		if (mongoc_cursor_error (cursor, &error)) {
      fprintf (stderr, "An error occurred: %s\n", error.message);
		}
		mongoc_cursor_destroy (cursor);
		bson_destroy (query);
		n = mongo_user_teardown(&cn);
		if(result == 0){
			return n;
		}
	}
	else if(flags == INSTA_UNKNOWN){
		get_user_ip_by_username(INSTA_DB, CASS_TABLE, username);	
	}
	return result;
}


int 
search_user_by_id(uint64_t user_id, int flags)
{
	  /* search for a user in the mongoDB list of followers followees or the Cassandra
   * database of users using the respective flags INSTA_FOLLOWER, INSTA_FOLLOWEE,
   * INSTA_UNKNOWN. Return number of results found or -1 on failure */
  struct mongo_user_connection cn;
  mongoc_cursor_t *cursor;
  bson_t *query;
  const bson_t *doc;
  bson_error_t error;
  char *str; 
  int result = 0;
  int n;

  if(flags == INSTA_FOLLOWER || flags == INSTA_FOLLOWEE) {
    cn.uri_string = "mongodb://localhost:27017";
    mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION);
    query = BCON_NEW ("user_id", BCON_INT64(user_id));
    cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
    while (mongoc_cursor_next (cursor, &doc)) {
      result+=1;
      str = bson_as_canonical_extended_json (doc, NULL);
      printf ("%s\n", str);
      bson_free (str);
    }
    if (mongoc_cursor_error (cursor, &error)) {
      fprintf (stderr, "An error occurred: %s\n", error.message);
    }
    mongoc_cursor_destroy (cursor);
    bson_destroy (query);
    n = mongo_user_teardown(&cn);
    if(result == 0){
      return n;
    }
  }
  else if(flags == INSTA_UNKNOWN){
    get_user_ip_by_id(INSTA_DB, CASS_TABLE, user_id);
  }
  return result;
}


int
insert_user(struct user *new_user)
{
	/* inserts a user object into the mongoDB */
	struct mongo_user_connection cn;
	int error;
	bson_t *doc;
	bson_t child;
	bson_t second_child;
	bson_t subchild_followers;
	bson_t subchild_following;
	char buf[10];
	int n;
	
	cn.uri_string = "mongodb://localhost:27017";

	if(new_user == NULL){
		return -1;
	}
	if((error = mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		return error;
	}

	/* return -1 if user is already in table, this behavior might be revised */
	if((n = search_user_by_id(new_user->user_id, INSTA_FOLLOWER)) > 0) {
		printf("user with user_id %ld already exists in table\n", new_user->user_id);
		return -1;
	}
	
	
	doc = bson_new ();
	BSON_APPEND_INT64(doc, "user_id", new_user->user_id);
	BSON_APPEND_UTF8(doc, "username", new_user->username);
	BSON_APPEND_UTF8(doc, "image_path", new_user->image_path); // TODO: change to binary?
	BSON_APPEND_DOCUMENT_BEGIN(doc, "bio", &child);
	BSON_APPEND_UTF8(&child, "name", new_user->bio->name);
	BSON_APPEND_UTF8(&child, "birthdate", new_user->bio->birthdate); // TODO: change to time_t
	bson_append_document_end(doc, &child);	
	BSON_APPEND_INT32(doc, "fragmentation", new_user->fragmentation);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "followers", &child);
	BSON_APPEND_INT32(&child, "direction", new_user->followers->direction);
	BSON_APPEND_INT32(&child, "count", new_user->followers->count);
	BSON_APPEND_ARRAY_BEGIN (&child, "user_ids", &subchild_followers);
  for (int i = 0; i < new_user->followers->count; ++i) {
		memset(buf, '\0', 10);
		sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&subchild_followers, buf, new_user->followers->user_ids[i]);
  }
  bson_append_array_end (&child, &subchild_followers);
	bson_append_document_end(doc, &child);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "following", &second_child); 
  BSON_APPEND_INT32(&second_child, "direction", new_user->following->direction);
  BSON_APPEND_INT32(&second_child, "count", new_user->following->count);
  BSON_APPEND_ARRAY_BEGIN (&second_child, "user_ids", &subchild_following);
  for (int i = 0; i < new_user->following->count; ++i) {
    memset(buf, '\0', 10);
    sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&subchild_following, buf, new_user->following->user_ids[i]);
  }
  bson_append_array_end (&second_child, &subchild_following);
	bson_append_document_end(doc, &second_child);
	

	if (!mongoc_collection_insert_one (cn.collection, doc, NULL, NULL, &cn.error)) {
		fprintf (stderr, "%s\n", cn.error.message);
  }
	
	bson_destroy (doc);
	bson_destroy (&child);
  bson_destroy (&second_child);
  bson_destroy (&subchild_followers);
  bson_destroy (&subchild_following);
	mongo_user_teardown(&cn);
	return 0;
}



int
delete_user(uint64_t user_id)
{
	struct mongo_user_connection cn;
  bson_t *selector;
  bson_t reply;
	bson_error_t error;
	bson_iter_t iterator;
  int n;
  
	cn.uri_string = "mongodb://localhost:27017";
  mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION);
  selector = BCON_NEW ("user_id", BCON_INT64(user_id));
  n = mongoc_collection_delete_one(cn.collection, selector, NULL, &reply, &error);
  if(!n){
		fprintf (stderr, "An error occurred: %s\n", error.message);
		return error.code;
	}
	else{
		printf("user_id %ld deleted\n", user_id);
	}
	bson_iter_init(&iterator, &reply); 
	if(bson_iter_find(&iterator, "deletedCount")){
		n = bson_iter_int32(&iterator);
	}
	bson_destroy (selector);
	bson_destroy (&reply);
	mongo_user_teardown(&cn);
  return n;
}

int 
pull_user_profile(uint64_t user_id)
{
	return 0;
}
