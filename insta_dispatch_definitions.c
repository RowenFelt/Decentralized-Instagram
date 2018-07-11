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
  /*TODO: verify that this poiner arithmetic is valid */
  BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
  for (uint32_t i = 0; i < MAX_NUM_TAGS; i++){
    if(*(dis->tags + i) != NULL){
						keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
      BSON_APPEND_UTF8(&child, key, *(dis->tags + i));
    }
    else{
      break; //Reached a NULL pointer marking the end of the list of tags
    }
  }
  bson_append_array_end(dispatch, &child);


  /* Insert any tagged users as sub-array */
  BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
  for (uint32_t i = 0; i < MAX_GROUP_SIZE; i++){
    if (dis->user_tags[i] > 0){
      keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
      bson_append_int64(&child, key, (int) keylen, dis->user_tags[i]);
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
	//TODO
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



