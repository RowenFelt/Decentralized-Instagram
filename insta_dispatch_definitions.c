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
  BSON_APPEND_TIME_T(dispatch, "timestamp", dis->timestamp);

  BSON_APPEND_INT32(dispatch, "audience_size", dis->audience_size); //who sees dispatch

  /* Store specific audience for a group message */
  if (dis->audience_size > 0 && dis->audience_size < MAX_GROUP_SIZE){
    BSON_APPEND_ARRAY_BEGIN(dispatch, "audience", &child);
    for (uint32_t i = 0; i < dis->audience_size; i++){
      keylen = bson_uint32_to_string (i, &key, buf, sizeof (str));
      bson_append_int64(&child, key, (int) keylen, dis->audience[i]);
    }
    bson_append_array_end(dispatch, &child);
  }

	printf("num_tags == %d", dis->num_tags);
  BSON_APPEND_INT32(dispatch, "num_tags", dis->num_tags);
	
	/* Insert any hashtags as sub-array */
	if (dis->num_tags > 0 && dis->num_tags < MAX_NUM_TAGS){
	  BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
		for (uint32_t i = 0; i < dis->num_tags; i++){
			keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
			BSON_APPEND_UTF8(&child, key, dis->tags[i]);
		}
		bson_append_array_end(dispatch, &child);
	}

  BSON_APPEND_INT32(dispatch, "num_user_tags", dis->num_user_tags);
  
	/* Insert any tagged users as sub-array */
	if (dis->num_user_tags > 0 && dis->num_user_tags <	MAX_NUM_TAGS){
		BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
		for (uint32_t i = 0; i < dis->num_tags; i++){
			keylen = bson_uint32_to_string(i, &key, buf, sizeof(str));
			bson_append_int64(&child, key, (int) keylen, dis->user_tags[i]);
		}
		bson_append_array_end(dispatch, &child);
	}

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
	//Verify that audience_size, num_tags, the size of each tag (i.e. number of characters),
	//and num_user_tags are all less than the defines constants
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

struct dispatch *
search_dispatch_by_id(uint64_t dispatch_id)
{
	struct dispatch *dis;
  bson_t *target_dispatch;   //Bson doc with target dispatch_id as query key/value
	const bson_t *result_dispatch;	 //Bson doc to write the query responce to
  mongoc_cursor_t *cursor;   //Points to the target dispatch after querying
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

	dis = bson_to_dispatch(result_dispatch);
	
	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	
	return dis; 

}



struct dispatch *
bson_to_dispatch(const bson_t *bson_dispatch){

	struct dispatch *dis = (struct dispatch *)malloc(sizeof(struct dispatch));
	dis->body = (struct body *)malloc(sizeof(struct dispatch_body));
	dis->parent = (struct parent *)malloc(sizeof(struct dispatch_parent));
	bson_iter_t iter, sub_iter;
	const char *body_media_path;
	const char *body_text;

	/* bind a bson iterator to the bson document that was found from our query */
	bson_iter_init(&iter, bson_dispatch);

	/* Fill dispatch_body struct */
	if(bson_iter_find_descendant(&iter, "body.media_path", &sub_iter)){
		body_media_path = bson_iter_utf8(&sub_iter, NULL);
		dis->body->media_path = strdup(body_media_path);
	}
	if(bson_iter_next(&sub_iter)){
		body_text =  bson_iter_utf8(&sub_iter, NULL);
		dis->body->text = strdup(body_text);  
	}

	
	if(bson_iter_find(&iter, "user_id")){
		dis->user_id = bson_iter_int64(&iter);
	}
	if(bson_iter_find(&iter, "timestamp")){
		dis->timestamp = bson_iter_time_t(&iter);
	}


	/*
	 * Pulling data from the audience array 
	 */
	if(bson_iter_find(&iter, "audience_size")){
		dis->audience_size = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "audience")){

		/* Check that current itter object holds array */
		if(BSON_ITER_HOLDS_ARRAY(&iter) && dis->audience_size != 0){
	
			/* pull bson data in to array audience_array */
			const uint8_t *array = NULL; 
			uint32_t array_len = 0;
			int i = 0;
			bson_iter_array(&iter, &array_len, &array);
			
			/* craft a new bson from our array we just pulled data to */			
			bson_t *audience_array = bson_new_from_data(array, array_len);
			
			/* initialize sub_iter to the new bson doc we just crafted */			
			bson_iter_init(&sub_iter, audience_array);
				
			/* itterate through the new bson document and pull data (where key == index) */
			while(bson_iter_next(&sub_iter) && i < dis->audience_size){
				dis->audience[i] = bson_iter_int64(&sub_iter);
				i++;
			}
		bson_destroy(audience_array);
		}
	}


	/*
	 * Pulling data from the tags array
	 */ 
	if(bson_iter_find(&iter, "num_tags")){

		dis->num_tags = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "tags")){

		if(BSON_ITER_HOLDS_ARRAY(&iter) && dis->num_tags != 0){
	
			const uint8_t *array = NULL;
			uint32_t array_len = 0;			
			int i = 0;										
			bson_iter_array(&iter, &array_len, &array);
			bson_t *tags_array = bson_new_from_data(array, array_len);
			bson_iter_init(&sub_iter, tags_array);
				
			while(bson_iter_next(&sub_iter) && i < dis->num_tags){
				strcpy(dis->tags[i], bson_iter_utf8(&sub_iter, NULL));
				i++;
			}
		bson_destroy(tags_array);
		}
	}


	/*
	 * Pulling data from the user_tags array 
	 */
	if(bson_iter_find(&iter, "num_user_tags")){
		dis->num_user_tags = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "user_tags") && dis->num_user_tags != 0){

		if(BSON_ITER_HOLDS_ARRAY(&iter)){
	
			const uint8_t *array = NULL; 
			uint32_t array_len = 0;
			int i = 0; 
			bson_iter_array(&iter, &array_len, &array);
			bson_t *user_tags_array = bson_new_from_data(array, array_len);
			bson_iter_init(&sub_iter, user_tags_array);
				
			while(bson_iter_next(&sub_iter) && i < dis->num_user_tags){
				dis->user_tags[i] = bson_iter_int64(&sub_iter);
				i++;
			}
		bson_destroy(user_tags_array);
		}
	}


	/* Fill dispatch_parent struct */
	if(bson_iter_find_descendant(&iter, "dispatch_parent.type", &sub_iter)){
		dis->parent->type = bson_iter_int32(&sub_iter);
	}
	if(bson_iter_next(&sub_iter)){
		dis->parent->id =  bson_iter_int64(&sub_iter);  
	}


	if(bson_iter_find(&iter, "fragmentation")){
		dis->fragmentation = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "dispatch_id")){
		dis->dispatch_id = bson_iter_int64(&iter);
	}

	print_dispatch_struct(dis);

	return dis;
}

int
print_dispatch_struct(struct dispatch *dis)
{
	printf("-----------------------------------------------------------------------\n");
	printf("body: \n");	
	printf("	media_path: %s\n",dis->body->media_path);
	printf("	text: %s\n", dis->body->text);
	printf("user_id: %ld\n",dis->user_id);	
		
	printf("timestamp: %ld \n",(long) dis->timestamp);	
	printf("audience_size %d\n", dis->audience_size);	
	for(int i = 0; i < dis->audience_size; i++){
		printf("user id = %ld\n", (dis->audience[i]));
	}
	printf("num_tags: %d\n", dis->num_tags);
	for(int i = 0; i < dis->num_tags; i++){
		if(strcmp(dis->tags[i], "") == 0){
			printf("no more tags\n");
			break;
		}
		else{
			printf("tags: %s\n", (dis->tags[i]));
		}
	}		
	printf("num_user_tags %d\n", dis->num_user_tags);
	for(int i = 0; i < dis->num_user_tags; i++){
		if(dis->user_tags[i] > 0){
			printf("user tags: %ld\n", (dis->user_tags[i]));
		}
		else{
			break;
		}
	}
	printf("parent: \n");	
	printf("	type: %d\n", dis->parent->type);
	printf("	id: %ld\n", dis->parent->id);
	printf("fragmentation: %d \n", dis->fragmentation);	
	printf("dispatch_id: %ld \n",dis->dispatch_id);	
	printf("-----------------------------------------------------------------------\n");
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



