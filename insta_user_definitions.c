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
      struct user usr;
			parse_user_bson(&usr, doc); //eventually do it this way 
			// printf ("%s\n", str); //comment out results for now
      bson_free (str);
			user_heap_cleanup(&usr); //this should be moved
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
	time_t creation;
	// char *c_time_string;	
	

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
		
	/* Obtain timestamp */
  creation = time(NULL);

  if (creation == ((time_t)-1)){
		(void) fprintf(stderr, "Failure to obtain the current time.\n");
		exit(EXIT_FAILURE);
	}

  /* Convert to local time format. */
  // c_time_string = ctime(&creation);
	// printf("Current time is %s", c_time_string);
	printf("finished time stuff\n");
	doc = bson_new ();
	BSON_APPEND_INT64(doc, "user_id", new_user->user_id);
	BSON_APPEND_UTF8(doc, "username", new_user->username);
	BSON_APPEND_UTF8(doc, "image_path", new_user->image_path); // TODO: change to binary?
	BSON_APPEND_DOCUMENT_BEGIN(doc, "bio", &child);
	BSON_APPEND_UTF8(&child, "name", new_user->bio->name);
	BSON_APPEND_DATE_TIME(&child, "date_created", creation);
	BSON_APPEND_DATE_TIME(&child, "date_modified", creation);
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

	printf("insert successful\n");	
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
	/* deletes a user from mongoDB with a given user_id */
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
	/* dependent on the protocols we define for peer to peer connections */
	return 0;
}


int 
parse_user_bson(struct user *user, const bson_t *doc)
{
	/* populates the user struct with the fields from the bson_t doc */
	bson_iter_t iter;
	bson_iter_t iter_bio;
	bson_iter_t iter_followers;
	bson_iter_t iter_following;
	const char *username;
	const char *image_path;		
	const char *name;
	struct personal_data *bio;
	struct insta_relations *followers;
	struct insta_relations *following;
	uint32_t bson_array_len;
	const uint8_t *bson_array;

	bio = malloc(sizeof(struct personal_data));
	followers = malloc(sizeof(struct insta_relations));
	following = malloc(sizeof(struct insta_relations));
	if(bio == NULL || followers == NULL ||
			following == NULL){
		perror("malloc");
		return -1;
	}

	bson_iter_init(&iter, doc);
	if(bson_iter_find(&iter, "user_id")){
		user->user_id = bson_iter_int64(&iter);
	}
	if(bson_iter_find(&iter, "username")){
		username = bson_iter_utf8(&iter, NULL);
		user->username = strdup(username);
	}
	if(bson_iter_find(&iter, "image_path")){
		image_path = bson_iter_utf8(&iter, NULL);
		user->image_path = strdup(image_path);
	}
	
	if(bson_iter_find_descendant(&iter, "bio.name", &iter_bio)){
		name = bson_iter_utf8(&iter_bio, NULL);
		bio->name = malloc(sizeof(char) * strlen(name)+1);
		if(bio->name == NULL){
			perror("malloc");
			return -1;
		}
		memcpy(bio->name, name, strlen(name)+1);
	}
	if(bson_iter_next(&iter_bio)){
    bio->date_created = bson_iter_time_t(&iter_bio);
		
	}
	if(bson_iter_next(&iter_bio)){
    bio->date_modified = bson_iter_time_t(&iter_bio);
	}
	user->bio = bio;
	if(bson_iter_find(&iter, "fragmentation")){
		user->fragmentation = bson_iter_int32(&iter);
	}
	if(bson_iter_find_descendant(&iter, "followers.direction", &iter_followers)){
    followers->direction = bson_iter_int32(&iter_followers);
  }	
	if(bson_iter_next(&iter_followers)){
	  followers->count = bson_iter_int32(&iter_followers);
  }
	if(bson_iter_next(&iter_followers) && BSON_ITER_HOLDS_ARRAY(&iter_followers)){
		bson_iter_array(&iter_followers, &bson_array_len, &bson_array);
		bson_t *followers_array = bson_new_from_data(bson_array, bson_array_len);
		bson_iter_init(&iter_followers, followers_array);
		int i=0;	
		followers->user_ids = malloc(sizeof(uint64_t) * followers->count);
		if(followers->user_ids == NULL){
		  perror("malloc");
      return -1;
    }
		while(bson_iter_next(&iter_followers) && i < followers->count){
			followers->user_ids[i] = bson_iter_int64(&iter_followers);
			printf("followers are %ld\n", followers->user_ids[i]);
		}
	}
	user->followers = followers;
	
	if(bson_iter_find_descendant(&iter, "following.direction", &iter_following)){
    following->direction = bson_iter_int32(&iter_following);
  }
  if(bson_iter_next(&iter_following)){
    following->count = bson_iter_int32(&iter_following);
  }
  if(bson_iter_next(&iter_following) && BSON_ITER_HOLDS_ARRAY(&iter_following)){
    bson_iter_array(&iter_following, &bson_array_len, &bson_array);
    bson_t *following_array = bson_new_from_data(bson_array, bson_array_len);
    bson_iter_init(&iter_following, following_array);
    int i=0;
    following->user_ids = malloc(sizeof(uint64_t) * following->count);
    if(following->user_ids == NULL){
      perror("malloc");
      return -1;
    }
    while(bson_iter_next(&iter_following) && i < following->count){
      following->user_ids[i] = bson_iter_int64(&iter_following);
      printf("following user are %ld\n", following->user_ids[i]);
    }
  }
  user->following = following;

	return 0;
}


void
user_heap_cleanup(struct user *user)
{
	/* cleans up heap variables */
	free(user->bio->name);
	free(user->bio);	
	free(user->followers->user_ids);
	free(user->followers);
}








