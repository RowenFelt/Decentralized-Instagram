/*
 * user_definitions.c
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
#include "mongo_connect.h"
#include "cass_user.h"
#include "user_definitions.h"

#define ARRAY_INDICES 10

static int parse_insta_relations(bson_iter_t *iter, struct insta_relations *friends, char *type, int *fields);

int
insert_user(struct user *new_user)
{
	/* inserts a user object into the mongoDB */
	struct mongo_connection cn;
	int error;
	bson_t *doc;
	bson_t child;
	bson_t second_child;
	bson_t subchild_followers;
	bson_t subchild_following;
	char buf[ARRAY_INDICES];
	int n;
	time_t creation;
	int length;

	cn.uri_string = MONGO_URI;

	if(new_user == NULL){
		return -1;
	}
	if((error = mongo_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		return -1;
	}
	
	// -1 is interperated as INT_MAX, and we want an exhaustive search for duplicates
	char *num = search_user_by_id_mongo(new_user->user_id, -1, &n, &length); 
	free(num);
	if(n > 0){
		goto insert_error;
	}
		
	/* Obtain timestamp */
  creation = time(NULL);

  if(creation == ((time_t)-1)){
		printf("Failure to obtain the current time.\n");
		goto insert_error;
	}
 
	doc = bson_new();
	BSON_APPEND_INT64(doc, "user_id", new_user->user_id);
	BSON_APPEND_UTF8(doc, "username", new_user->username);
	BSON_APPEND_INT32(doc, "image_length", new_user->image_length);
	BSON_APPEND_BINARY(doc, "image", 0, new_user->image, new_user->image_length);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "bio", &child);
	BSON_APPEND_UTF8(&child, "name", new_user->bio->name);
	BSON_APPEND_TIME_T(&child, "date_created", creation);
	BSON_APPEND_TIME_T(&child, "date_modified", creation);
	bson_append_document_end(doc, &child);	
	BSON_APPEND_INT32(doc, "fragmentation", new_user->fragmentation);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "followers", &child);
	BSON_APPEND_INT32(&child, "direction", new_user->followers->direction);
	BSON_APPEND_INT32(&child, "count", new_user->followers->count);
	BSON_APPEND_ARRAY_BEGIN(&child, "user_ids", &subchild_followers);
	for (int i = 0; i < new_user->followers->count; ++i) {
		memset(buf, '\0', ARRAY_INDICES);
		sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&subchild_followers, buf, new_user->followers->user_ids[i]);
	}
	bson_append_array_end(&child, &subchild_followers);
	bson_append_document_end(doc, &child);
	BSON_APPEND_DOCUMENT_BEGIN(doc, "following", &second_child); 
  BSON_APPEND_INT32(&second_child, "direction", new_user->following->direction);
  BSON_APPEND_INT32(&second_child, "count", new_user->following->count);
  BSON_APPEND_ARRAY_BEGIN(&second_child, "user_ids", &subchild_following);
  for (int i = 0; i < new_user->following->count; ++i) {
    memset(buf, '\0', ARRAY_INDICES);
    sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&subchild_following, buf, new_user->following->user_ids[i]);
  }
  bson_append_array_end(&second_child, &subchild_following);
	bson_append_document_end(doc, &second_child);

	if (!mongoc_collection_insert_one(cn.collection, doc, NULL, NULL, &cn.error)) {
		printf("insertion error: %s\n", cn.error.message);
		bson_destroy(doc);
		goto insert_error;
  }

	bson_destroy (doc);
	bson_destroy (&child);
  bson_destroy (&second_child);
  bson_destroy (&subchild_followers);
  bson_destroy (&subchild_following);
	mongo_teardown(&cn);
	return 0;

insert_error:
	mongo_teardown(&cn);
	return -1;
}

int
delete_user(uint64_t user_id)
{
	/* deletes a user from mongoDB with a given user_id */
	struct mongo_connection cn;
  bson_t *selector;
  bson_t reply;
	bson_error_t error;
	bson_iter_t iterator;
  int result, cn_error;
  
	cn.uri_string = MONGO_URI;
  
	if((cn_error = mongo_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		printf("mongo connect error in delete_user()\n");
		return -1;
	}

	selector = BCON_NEW ("user_id", BCON_INT64(user_id));
  result = mongoc_collection_delete_one(cn.collection, selector, NULL, &reply, &error);
  if(!result){
		printf("An error occurred: %s\n", error.message);
		bson_destroy(selector);
		mongo_teardown(&cn);
		return error.code;
	}
	bson_iter_init(&iterator, &reply); 
	if(bson_iter_find(&iterator, "deletedCount")){
		result = bson_iter_int32(&iterator);
	}
	bson_destroy(selector);
	bson_destroy(&reply);
	mongo_teardown(&cn);
  return result;
}


void
user_heap_cleanup(struct user *user)
{
	/* cleans up heap variables */
	if(user == NULL){
		printf("NULL user pointer in user_heap_cleanup\n");
		return;
	}
	if(user->bio == NULL){
		printf("NULL user bio in user_heap_cleanup\n");
		return;
	}
	if(user->followers == NULL){
		printf("NULL user followers in user_heap_cleanup\n");
		return;
	}
	if(user->following == NULL){
		printf("NULL user following in user_heap_cleanup\n");
		return;
	}
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

	if(user == NULL){
		printf("NULL user pointer in print_user_struct\n");
		return;
	}	
	printf("-----------------------------------------------------------------------\n"); 
	printf("user_id: %ld\n", user->user_id);
	printf("username: %s\n", user->username);
	printf("image_length: %d\n", user->image_length);
	printf("bio:\n");
	printf("  name: %s\n", user->bio->name);
	c_time_string = ctime(&user->bio->date_created);
	printf("  date_created: %s", c_time_string);
	c_time_string = ctime(&user->bio->date_modified);
	printf("  date_modified: %s", c_time_string);
	printf("fragmentation: %d\n", user->fragmentation);
	printf("followers:\n");
	printf("  direction: %d\n", user->followers->direction);
	printf("  count: %d\n", user->followers->count);
	for(int i=0; i<user->followers->count; i++){
		printf("  follower[%d].user_id: %ld\n", i, user->followers->user_ids[i]);
	}
	printf("following:\n");
	printf("  direction: %d\n", user->following->direction);
  printf("  count: %d\n", user->following->count);
  for(int i=0; i<user->following->count; i++){
    printf("  following[%d].user_id: %ld\n", i, user->following->user_ids[i]);
  }
	printf("-----------------------------------------------------------------------\n");
}


char * 
search_user_by_name_mongo(char *username, int req_num, int *result, int *length)
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
	struct mongo_connection cn;
	mongoc_cursor_t *cursor;
	bson_t *query;
	bson_error_t error;
	int cn_error;
  char *buf;
	
	cn.uri_string = MONGO_URI;
	if((cn_error = mongo_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		return NULL;
	}

	query = BCON_NEW (
		"$or", "[",
		"{", "username", BCON_UTF8(username), "}",
		"{", "bio.name", BCON_UTF8(username), "}",
		"]"	
	);
	cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_user_by_name_mongo\n");
		goto search_user_by_name_mongo_error;
	}
	
	*result = 0; //Default expectation is no results were found
	buf = build_json(cursor, req_num, result, length);	

	if(mongoc_cursor_error (cursor, &error)) {
    printf("Cursor error in build_json: %s\n", error.message);
	}
	mongoc_cursor_destroy (cursor);
	bson_destroy (query);
	mongo_teardown(&cn);
	
	return buf;

search_user_by_name_mongo_error:
	bson_destroy(query);
	mongo_teardown(&cn);
	return NULL;
}


char * 
search_user_by_id_mongo(uint64_t user_id, int req_num, int *result, int *length)
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
  struct mongo_connection cn;
  mongoc_cursor_t *cursor;
  bson_t *query;
  bson_error_t error;
	int cn_error;
	
  cn.uri_string = MONGO_URI;
	if((cn_error = mongo_connect(&cn, INSTA_DB, USER_COLLECTION)) != 0){
		return NULL;
	}
  query = BCON_NEW ("user_id", BCON_INT64(user_id));
  cursor = mongoc_collection_find_with_opts(cn.collection, query, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_user_by_id_mongo\n");
		goto search_user_by_id_mongo_error;
	}
	*result = 0; //Default expectation is no results were found
	char *buf = build_json(cursor, req_num, result, length); 

  if (mongoc_cursor_error (cursor, &error)) {
    printf("cursor error in build_json(): %s\n", error.message);
  }
  mongoc_cursor_destroy (cursor);
  bson_destroy (query);
  mongo_teardown(&cn);
 
	return buf;

search_user_by_id_mongo_error:
	bson_destroy(query);
	mongo_teardown(&cn);
	return NULL;

}


int 
handle_user_bson(bson_t *doc)
{
	//parse to user struct
	struct user new_user;
	int result;
	int length;

	if(doc == NULL){
		printf("NULL doc pointer in handle_user_bson\n");
		return -1;
	}

	if(parse_user_bson(&new_user, doc) < 0){
		printf("error parsing to user struct\n");
		return -1;
	}

	//search for duplicate by user id
	char *buf = search_user_by_id_mongo(new_user.user_id, 1, &result, &length);
	free(buf);
 
	if(result > 0 && delete_user(new_user.user_id) < 0){
			printf("deletion of duplicate failed\n");
			return -1;
	}

	//insert the user from the new_user struct	
	if(insert_user(&new_user) != 0){
		printf("insertion from struct failed\n");
		return -1;
	}

	return 0;

}


int 
parse_user_bson(struct user *user, const bson_t *doc)
{
	/* populates the user struct with the fields from the bson_t doc,
   * all pointers in the user struct are malloc'ed and must be freed.
   * Returns 0 on success or -1 on failure */
	bson_iter_t iter;
	bson_iter_t iter_bio;
	const char *username;
	const uint8_t *image;		
	const char *name;
	struct personal_data *bio = NULL;
	struct insta_relations *followers = NULL;
	struct insta_relations *following = NULL;
	int fields = 0;

	bio = malloc(sizeof(struct personal_data));
	followers = malloc(sizeof(struct insta_relations));
	following = malloc(sizeof(struct insta_relations));
	if(bio == NULL || followers == NULL ||
			following == NULL){
		perror("parse_user_bson: malloc");
		goto parse_user_bson_error;
	}

	followers->user_ids = NULL;
	following->user_ids = NULL;

	bson_iter_init(&iter, doc);
	if(bson_iter_find(&iter, "user_id")){
		user->user_id = bson_iter_int64(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "username")){
		username = bson_iter_utf8(&iter, NULL);
		user->username = strdup(username);
		if(user->username == NULL){
			perror("parse_user_bson: strdup");
			goto parse_user_username;
		}
		fields++;
	}
	if(bson_iter_find(&iter, "image_length")){
		user->image_length = bson_iter_int32(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "image")){
		user->image = malloc(sizeof(uint8_t) * user->image_length);
		if(user->image == NULL){
			perror("image in parse_user_bson: malloc");
			goto parse_user_image;
		}
		bson_iter_binary(&iter, NULL, &user->image_length, &image);
		memcpy(user->image, image, user->image_length);
		fields++;	
	}
	
	if(bson_iter_find_descendant(&iter, "bio.name", &iter_bio)){
		name = bson_iter_utf8(&iter_bio, NULL);
		bio->name = strdup(name);	
		if(bio->name == NULL){
			perror("parse_user_bson: strdup");
			goto parse_user_bio_name;
		}
		fields++;
	}
	if(bson_iter_next(&iter_bio)){
    bio->date_created = bson_iter_time_t(&iter_bio);
		fields++;	
	}
	if(bson_iter_next(&iter_bio)){
    bio->date_modified = bson_iter_time_t(&iter_bio);
		fields++;
	}
	user->bio = bio;
	if(bson_iter_find(&iter, "fragmentation")){
		user->fragmentation = bson_iter_int32(&iter);
		fields++;
	}
	parse_insta_relations(&iter, followers, "followers", &fields);
	user->followers = followers;
	parse_insta_relations(&iter, following, "following", &fields);
  user->following = following;
	if(fields == 14){
		return 0;
	}
	else{
		printf("incorrect number of fields\n");
		goto parse_relations_error;
	}

parse_relations_error:
	free(following->user_ids);
	free(followers->user_ids);
parse_user_bio_name:
	free(bio->name);
parse_user_image:
	free(user->image);
parse_user_username:
	free(user->username);
parse_user_bson_error:
	free(bio);
	free(followers);
	free(following);
	return -1;
}


static int 
parse_insta_relations(bson_iter_t *iter, struct insta_relations *friends, char *type, int *fields)
{
	/* parses the insta_relations sub document from a bson user object,
   * takes a bson_iter_t, insta_relations pointer, type (either "followers"
   * or "following", and int fields pointer to increment for parse_user_bson.
   * Returns 0 on success or -1 on failure. */
	bson_iter_t iter_relation;
	uint32_t bson_array_len;
  const uint8_t *bson_array;
	char subtype[DIRECTION_STRING_LEN];
	char new_type[RELATION_STRING_LEN];
	int type_length = 9;	

	if(iter == NULL){
		printf("NULL iter pointer, invalid argument\n");
		return -1;
	}
	if(friends == NULL){
		printf("NULL friends pointer, invalid argument\n");
		return -1;
	}
	if(type == NULL || strlen(type) > type_length){
		printf("NULL or invalid type argument\n");
		return -1;
	}
	if(fields == NULL){
		printf("NULL fields pointer, invalid argument\n");
		return -1;
	}
	memset(new_type, '\0', RELATION_STRING_LEN);
	/* check insta_relations type */
	if(!((memcmp(type, "followers", type_length) == 0)
		|| (memcmp(type, "following", type_length) == 0))){
		return -1;
	}
	strcpy(subtype, ".direction");
	strcat(new_type, type);
	strcat(new_type, subtype);
	if(bson_iter_find_descendant(iter, new_type, &iter_relation)){
    friends->direction = bson_iter_int32(&iter_relation);
		*fields+=1;
  }
  if(bson_iter_next(&iter_relation)){
    friends->count = bson_iter_int32(&iter_relation);
		*fields+=1;
  }
  if(bson_iter_next(&iter_relation) && BSON_ITER_HOLDS_ARRAY(&iter_relation)){
    bson_iter_array(&iter_relation, &bson_array_len, &bson_array);
    bson_t *relation_array = bson_new_from_data(bson_array, bson_array_len);
    bson_iter_init(&iter_relation, relation_array);
    int i=0;
    friends->user_ids = malloc(sizeof(uint64_t) * friends->count);
    if(friends->user_ids == NULL){
      printf("parse_insta_relations %s ", type);
			perror("malloc"); 
			return -1;
    }
    while(bson_iter_next(&iter_relation) && i < friends->count){
      friends->user_ids[i] = bson_iter_int64(&iter_relation);
      i+=1;
    }
		*fields+=1;
  }
	return 0;
}

