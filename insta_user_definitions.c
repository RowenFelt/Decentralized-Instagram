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
#include "util.h"

#define INSTA_DB "insta"
#define USER_COLLECTION "users"
#define CASS_TABLE "user"

static int parse_insta_relations(bson_iter_t *iter, struct insta_relations *friends, char *type);

int 
init_user(void)
{
	return 0;
}

char * 
search_user_by_name_mongo(char *username, int req_num, int *result)
{
	/* 
	 * search for a user by username in the mongo database
   * returns a point to a json string containing all
   * the users found as a result of the query. int
   * result is updated to reflect the number of users in
   * the json. If none are found, returns NULL and sets
   * result to 0. req_num is a cap on the number of results
   * to search for (i.e. a query would terminate after 
   * finding a single result if req_num == 1).
   */
	struct mongo_user_connection cn;
	mongoc_cursor_t *cursor;
	bson_t *query;
	bson_error_t error;
	
	cn.uri_string = "mongodb://localhost:27017";
	mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION);
	query = BCON_NEW (
		"$or", "[",
		"{", "username", BCON_UTF8(username), "}",
		"{", "bio.name", BCON_UTF8(username), "}",
		"]"	
	);
	cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
	
	*result = 0; //Default expectation is no results were found
	char *buf = build_json(cursor, req_num, result);	

	if (mongoc_cursor_error (cursor, &error)) {
    fprintf (stderr, "An error occurred: %s\n", error.message);
	}
	mongoc_cursor_destroy (cursor);
	bson_destroy (query);
	mongo_user_teardown(&cn);
	
	return buf;
}

int 
search_user_by_name_cass(char *username)
{
	/* searches for a user in the Cassandra database by username
   * returns the number of results found */ 	
	int result = 0;
	result = get_user_ip_by_username(INSTA_DB, CASS_TABLE, username);
	return result;
}

char * 
search_user_by_id_mongo(uint64_t user_id, int req_num, int *result)
{
	/*
	 * search for a user in the mongoDB list of 
   * user by user_id. Return a pointer to a 
   * json sring and update result to the number
   * of users in the json string. Returns NULL
   * and sets result to 0 if no users are found.
   * req_num is a cap on the number of results
   * to search for (i.e. a query would terminate after 
   * finding a single result if req_num == 1). 
   */
  struct mongo_user_connection cn;
  mongoc_cursor_t *cursor;
  bson_t *query;
  bson_error_t error;
	
  cn.uri_string = "mongodb://localhost:27017";
  mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION);
  query = BCON_NEW ("user_id", BCON_INT64(user_id));
  cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
	
	*result = 0; //Default expectation is no results were found
	char *buf = build_json(cursor, req_num, result); 

  if (mongoc_cursor_error (cursor, &error)) {
    fprintf (stderr, "An error occurred: %s\n", error.message);
  }
  mongoc_cursor_destroy (cursor);
  bson_destroy (query);
  mongo_user_teardown(&cn);
 
	return buf;
}

char *
search_user_by_id_cass(uint64_t user_id)
{
	/* searches the cassandra user database by user_id,
   * returns char * IP address in dotted quad notation
   * on success, or NULL on failure. The inet must be freed */
	char *inet = NULL;
	inet = get_user_ip_by_id(INSTA_DB, CASS_TABLE, user_id);
	return inet;
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

	cn.uri_string = "mongodb://localhost:27017";

	if(new_user == NULL){
		return -1;
	}
	if((error = mongo_user_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		return error;
	}
	
	// -1 is interperated as INT_MAX, and we want an exhaustive search for duplicates
	search_user_by_id_mongo(new_user->user_id, -1, &n); 
	if(n > 0) {
		return -1;
	}
		
	/* Obtain timestamp */
  creation = time(NULL);

  if (creation == ((time_t)-1)){
		(void) fprintf(stderr, "Failure to obtain the current time.\n");
		exit(EXIT_FAILURE);
	}
 
	doc = bson_new ();
	BSON_APPEND_INT64(doc, "user_id", new_user->user_id);
	BSON_APPEND_UTF8(doc, "username", new_user->username);
	BSON_APPEND_UTF8(doc, "image_path", new_user->image_path);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "bio", &child);
	BSON_APPEND_UTF8(&child, "name", new_user->bio->name);
	BSON_APPEND_TIME_T(&child, "date_created", creation);
	BSON_APPEND_TIME_T(&child, "date_modified", creation);
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
	const char *username;
	const char *image_path;		
	const char *name;
	struct personal_data *bio;
	struct insta_relations *followers;
	struct insta_relations *following;

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
	parse_insta_relations(&iter, followers, "followers");
	user->followers = followers;
	parse_insta_relations(&iter, following, "following");
  user->following = following;
	return 0;
}

static int 
parse_insta_relations(bson_iter_t *iter, struct insta_relations *friends, char *type)
{
	/* parses the insta_relations sub document from a bson user object */
	bson_iter_t iter_relation;
	uint32_t bson_array_len;
  const uint8_t *bson_array;
	char subtype[11];
	char new_type[30];
		
	memset(new_type, '\0', 30);
	/* check insta_relations type */
	if(!((memcmp(type, "followers", 9) == 0)
		|| (memcmp(type, "following", 9) == 0))){
		return -1;
	}
	strcpy(subtype, ".direction");
	strcat(new_type, type);
	strcat(new_type, subtype);
	if(bson_iter_find_descendant(iter, new_type, &iter_relation)){
    friends->direction = bson_iter_int32(&iter_relation);
  }
  if(bson_iter_next(&iter_relation)){
    friends->count = bson_iter_int32(&iter_relation);
  }
  if(bson_iter_next(&iter_relation) && BSON_ITER_HOLDS_ARRAY(&iter_relation)){
    bson_iter_array(&iter_relation, &bson_array_len, &bson_array);
    bson_t *relation_array = bson_new_from_data(bson_array, bson_array_len);
    bson_iter_init(&iter_relation, relation_array);
    int i=0;
    friends->user_ids = malloc(sizeof(uint64_t) * friends->count);
    if(friends->user_ids == NULL){
      perror("malloc");
      return -1;
    }
    while(bson_iter_next(&iter_relation) && i < friends->count){
      friends->user_ids[i] = bson_iter_int64(&iter_relation);
      i+=1;
    }
  }
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
	free(user->following->user_ids);
	free(user->following);	
}


void
print_user_struct(struct user *user)
{
	char *c_time_string;
  
	printf("    user_id: %ld\n", user->user_id);
	printf("    username: %s\n", user->username);
	printf("    image_path: %s\n", user->image_path);
	printf("    bio.name: %s\n", user->bio->name);
	c_time_string = ctime(&user->bio->date_created);
	printf("    bio.date_created: %s", c_time_string);
	c_time_string = ctime(&user->bio->date_modified);
	printf("    bio.date_modified: %s", c_time_string);
	printf("    fragmentation: %d\n", user->fragmentation);
	printf("    followers.direction: %d\n", user->followers->direction);
	printf("    followers.count: %d\n", user->followers->count);
	for(int i=0; i<user->followers->count; i++){
		printf("    follower[%d].user_id: %ld\n", i, user->followers->user_ids[i]);
	}
	printf("    following.direction: %d\n", user->following->direction);
  printf("    following.count: %d\n", user->following->count);
  for(int i=0; i<user->following->count; i++){
    printf("    following[%d].user_id: %ld\n", i, user->following->user_ids[i]);
  }
}







