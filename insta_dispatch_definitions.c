/*
 * insta_dispatch_definitions.c
 * dispatch object and methods for Mongo implementation
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
#include "insta_dispatch_definitions.h"


#define INSTA_DB "insta"
#define DISPATCH_COLLECTION "dispatch"


/*
 * Takes an instance of a dispatch struct, dis, which contains all the data associated with
 * an dispatch. Formats the information contained in dis in a bson_t document, then inserts
 * said documet in the dispatch collection of the insta database as specified by the 
 * constants at the head of this file.
 */
int
insert_dispatch(struct dispatch *dis) {

  bson_t *dispatch;   //The dispatch
  bson_t child;       //A temp bson_t doc that gets unitialized and cleared when used
	bson_error_t error;

  char buf[16];       //Buffer to build index 'keys' for an array of user id's
  const char *key;    //For char* representations of numerical indexes refferenced above 
  size_t keylen = 0;  //The length of the converted index keys TODO: see if macro implimentation renders this variable unessesary   
  char* str;          //All-pourpose string used in builing sub docs and arrays 

	/*
   * Initialize a connection to mongo, specify the the database and collection to store
   * the dispatch bson document in
   */
  struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

  /*
   * Craft a new BSON document using the fields of a dispatch object, then
   * insert in to the dispatch collection
   */
  dispatch = bson_new();

  /* Dispatch body comprised of media and text */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "body", &child);
  BSON_APPEND_UTF8(&child, "media_path", dis->body->media_path); //the path to dispatch media
  BSON_APPEND_UTF8(&child, "text", dis->body->text); //the text/caption for the dispatch
  bson_append_document_end(dispatch, &child);

  BSON_APPEND_INT64(dispatch, "user_id", dis->user_id);

 /* Setting a timestamp based on the current time */
 /* TODO: FIX THE TIMESTAMP */
  time_t rawtime = time(NULL); //time since epoch, this seems to be how time is stored in 
                               //mongo, and it can be easily formated from this
  dis->timestamp = rawtime;    //might need to cast or otherwise 'fuzz' to play nice w/ mongo
  BSON_APPEND_DATE_TIME(dispatch, "timestamp", dis->timestamp);

  BSON_APPEND_INT32(dispatch, "audience_size", dis->audience_size); //who sees dispatch

  /* Discern if we need to store a specific audience for a group message */
  if (dis->audience_size > 0 && dis->audience_size < 32){

    BSON_APPEND_ARRAY_BEGIN(dispatch, "audience", &child);

    /* Generate numerical keys and convert to UTF8 */
    for (uint32_t i = 0; i < dis->audience_size; i++){
      keylen = bson_uint32_to_string (i, &key, buf, sizeof (str));
      bson_append_int64(&child, key, (int) keylen, dis->audience[i]);
    }
    bson_append_array_end(dispatch, &child);
  }


  /* Insert any hashtags as sub-array */
  BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
  for (uint32_t i = 0; i < MAX_NUM_TAGS; i++){
    if(strcmp(dis->tags[i],"") != 0){
						keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
      BSON_APPEND_UTF8(&child, key, dis->tags[i]);
    }
    else{
      break; //Reached a NULL pointer marking the end of the list of tags
    }
  }
  bson_append_array_end(dispatch, &child);


  /*
	 * Insert any tagged users as sub-array - if there are fewer than 32 tagged users, the 
	 * final desired entry in the array (num_tagged_users + 1) must have a value that is 
	 * <= 0 to deliniate the end of valid user ids
	 */
  BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
  for (uint32_t i = 0; i < MAX_GROUP_SIZE; i++){
    if (dis->user_tags[i] > 0){
      keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
      bson_append_int64(&child, key, (int) keylen, dis->user_tags[i]);
    }
		else{
			break; //Reached the end of our list of tagged users, exit loop
		}
  }
  bson_append_array_end(dispatch, &child);


  /* Insert dispatch_parent struct w/ parent's id */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "dispatch_parent", &child);
  BSON_APPEND_INT32(&child, "type", dis->parent->type);
  BSON_APPEND_INT64(&child, "id", dis->parent->id); 
  bson_append_document_end(dispatch, &child);

  BSON_APPEND_INT32(dispatch, "fragmentation", dis->fragmentation);
  BSON_APPEND_INT64(dispatch, "dispatch_id", dis->dispatch_id);

  /* Printing the document as a JSON string for error checking */
  str = bson_as_canonical_extended_json(dispatch, NULL);
  printf("%s\n", str);
  bson_free(str);


  if (!mongoc_collection_insert_one (cn.collection, dispatch, NULL, NULL, &error)) {
     fprintf (stderr, "%s\n", error.message);
  }

  /* clean up bson doc and collection */
  bson_destroy (dispatch);
  return mongo_user_teardown(&cn);
}

uint64_t 
create_dispatch(void)
{
	//TODO
	return 0;
}

uint64_t 
delete_dispatch(uint64_t dispatch_id)
{
	
  bson_t *target_dispatch;   //The dispatch
	bson_t reply;
	bson_error_t error;
	char *str;

	/*
   * Initialize a connection to mongo, specify the the database and collection to store
   * the dispatch bson document in
   */
  struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

  /*
   * Craft a new BSON document using the dispatch_id then query the  dispatch collection
   */
  target_dispatch = bson_new();
	BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);

	if(!mongoc_collection_delete_one(cn.collection, target_dispatch, NULL, &reply, &error)){
		fprintf(stderr, "Delete failed: %s\n", error.message);
	}

	/* print reply as json for debugging purposes */
	str = bson_as_canonical_extended_json(&reply, NULL);
	printf("%s\n", str);

	bson_destroy(target_dispatch);
	bson_destroy(&reply);
	
  mongo_user_teardown(&cn);

	return 0;
}

int
search_dispatch_by_id(uint64_t dispatch_id)
{

  bson_t *target_dispatch;   //Bson doc with target dispatch_id as query key/value
	const bson_t *result_dispatch;	 //Bson doc to write the query responce to
  mongoc_cursor_t *cursor;   //Points to the target dispatch after querying
	bson_iter_t iter;
	char *str;

	/*
   * Initialize a connection to mongo, specify the the database and collection that
   * the dispatch bson document is stored in
   */
  struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

printf("established connection\n");

  /*
   * Craft a new BSON document using the dispatch_id the query the dispatch collection
   */
  target_dispatch = bson_new();
	BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);

	/*
	 * Query using no options of read preferences b/c we are looking for a single 
	 * document with a globally unique identifier.
	 */
	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);

printf("querying database\n");	

	if(!(mongoc_cursor_next(cursor, &result_dispatch))){
		//We failed to find the desired dispatch - TODO: give this failure a more robust
		//return value for effective error checking...
		printf("empty doc, result does not exist\n");
		exit(0);
	}

printf("found result\n");
	
	/* print result_dispatch as json for debugging purposes */
	str = bson_as_canonical_extended_json(result_dispatch, NULL);
	printf("%s\n", str);

printf("parsing result dispatch to construct dispatch struct with c data types\n");
	
	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	
	return 0;

}

int
push_dispatch(uint64_t dispatch_id)
{
	//TODO
	return 0; 
}

int
pull_dispatch(uint64_t dispatch_id)
{
	//TODO
	return 0;
}

int
update_feed(void)
{
	//TODO
	return 0;
}



