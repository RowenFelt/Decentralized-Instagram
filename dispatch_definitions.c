/*
 * dispatch_definitions.c
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
#include "mongo_connect.h"
#include "dispatch_definitions.h"

#define DISPATCH_ARRAY_INDEX 10
#define NUM_DISPATCH_FIELDS 15

int
insert_dispatch(struct dispatch *dis) {
/* 
 * Takes a dispatch struct and inserts it in the collection 
 * 'dispatch' (part of the 'insta' database).
 * Returns 0 upon successful insertion, -1 otherwise.
 * The BSON_APPEND (uppercase) functions are macros that obfuscate
 * additional fields, such as strlen. The lower-case versions,
 * such as bson_append_document_end(), are direct function calls
 * without macro equivalents.
 */
  bson_t *dispatch; 
  bson_t child;    
  bson_error_t error;
  char buf[DISPATCH_ARRAY_INDEX]; //number of bytes in max index of array followers
  time_t timestamp;
	int n;
  struct mongo_connection cn;
	char *num;

  cn.uri_string = MONGO_URI;

  n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connection failed\n");
		return n;
	}	

  //check if dis is a dupicate already stored in the the database 
  //(in which case duplicate will remain equal to 1)
  int duplicate = 0;
  int req_num = -1; 
	int length = 0;
  num = search_dispatch_by_id(dis->dispatch_id, req_num, &duplicate, &length); 	
  if(duplicate > 0){
		mongo_teardown(&cn); 
		return -1;
  }
	free(num);
  dispatch = bson_new();

  /* Dispatch body comprised of media and text */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "body", &child);
	BSON_APPEND_INT32(&child, "media_size", dis->body->media_size);
	BSON_APPEND_BINARY(&child, "media", 0, dis->body->media, dis->body->media_size);
	BSON_APPEND_UTF8(&child, "text", dis->body->text);  
	bson_append_document_end(dispatch, &child); 

	/* timestamp, user_id, audience size */
	BSON_APPEND_INT64(dispatch, "user_id", dis->user_id);
  timestamp = time(NULL);
  if(timestamp == ((time_t) -1)){
    printf("Faulty current time\n");
		goto insert_error;
  }
  dis->timestamp = timestamp;  
  BSON_APPEND_TIME_T(dispatch, "timestamp", dis->timestamp);
  BSON_APPEND_INT32(dispatch, "audience_size", dis->audience_size); //who sees dispatch

  /* Store specific audience for a group message */
  if (dis->audience_size > MAX_GROUP_SIZE){
    printf("audience is larger than MAX_GROUP_SIZE\n");
		goto insert_error;
  }	
  BSON_APPEND_ARRAY_BEGIN(dispatch, "audience", &child);
  for (int i = 0; i < dis->audience_size; i++){
    memset(buf, '\0', DISPATCH_ARRAY_INDEX);
    sprintf(buf, "%d", i);
    BSON_APPEND_INT64(&child, buf, dis->audience[i]);
  }
	bson_append_array_end(dispatch, &child);
 
  /* Insert any hashtags as sub-array */
	BSON_APPEND_INT32(dispatch, "num_tags", dis->num_tags);
	if(dis->num_tags > MAX_NUM_TAGS){
		goto insert_error;
	}
	BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
	for (int i = 0; i < dis->num_tags; i++){
		memset(buf, '\0', DISPATCH_ARRAY_INDEX);
		sprintf(buf, "%d", i);
		BSON_APPEND_UTF8(&child, buf, dis->tags[i]);
	}
	bson_append_array_end(dispatch, &child);

	/* Insert any tagged users as sub-array */
	BSON_APPEND_INT32(dispatch, "num_user_tags", dis->num_user_tags);
	if (dis->num_user_tags >	MAX_NUM_TAGS){
		goto insert_error;
	}	
	BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
	for (int i = 0; i < dis->num_user_tags; i++){
		memset(buf, '\0', DISPATCH_ARRAY_INDEX);
		sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&child, buf, dis->user_tags[i]);
	}
	bson_append_array_end(dispatch, &child);

	/* Insert dispatch_parent struct w/ parent's id */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "dispatch_parent", &child);
  BSON_APPEND_INT32(&child, "type", dis->parent->type);
	BSON_APPEND_INT64(&child, "id", dis->parent->id); 
  bson_append_document_end(dispatch, &child);

	/* fragmentation and dispatch_id */
	BSON_APPEND_INT32(dispatch, "fragmentation", dis->fragmentation);
  BSON_APPEND_INT64(dispatch, "dispatch_id", dis->dispatch_id);

  /* clean up bson doc and collection */
	if (!mongoc_collection_insert_one (cn.collection, dispatch, NULL, NULL, &error)) {
		printf ("insertion failed with error: %s\n", error.message); 
		goto insert_error;
	} 
  bson_destroy (dispatch);
	return mongo_teardown(&cn);

insert_error:
	bson_destroy(dispatch);
	mongo_teardown(&cn);
	return -1;
}

int 
delete_dispatch(uint64_t dispatch_id){
/* 
 * Takes a dispatch id, queries the 'dispatch' collection for 
 * a matching dispatch and deletes it.
 * Returns 0 if the dispatch is sucessfully deteled, otherwise 
 * -1 is returned
 */
  bson_t *target_dispatch;  
	bson_t reply;
	bson_error_t error;
	int n;

  struct mongo_connection cn;
	cn.uri_string = MONGO_URI;

	n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo_connect() failed in delete_dispatch\n");
		return n;
	}

  target_dispatch = bson_new();
	BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);

	if(!mongoc_collection_delete_one(cn.collection, target_dispatch, NULL, &reply, &error)){
		printf("Delete failed: %s\n", error.message);
		bson_destroy(target_dispatch);
		return -1;
	}
	bson_destroy(target_dispatch);
	bson_destroy(&reply);
	
	return mongo_teardown(&cn); 
}



char *
search_dispatch_by_id(uint64_t dispatch_id, int req_num, int *result, int *length){ 
/* 
 * Takes a dispatch id, an upper limit of dispatches to 
 * query for, and a pointer to an integer to be updated
 * with the number of dispatches matching the dispatch 
 * id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query.
 */
  bson_t *target_dispatch;  
  mongoc_cursor_t *cursor;
	int n;
  struct mongo_connection cn;

  cn.uri_string = MONGO_URI;

  n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connect failed in search_dispatch_by_id\n");
		return NULL;
	}
  
  target_dispatch = bson_new();
  BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);
  
  cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_dispatch_by_id\n");
		bson_destroy(target_dispatch);
		mongo_teardown(&cn);
		return NULL;
	}
	 
	*result = 0;
	char* buf = build_json(cursor, req_num, result, length);	 
  mongoc_cursor_destroy(cursor);
  bson_destroy(target_dispatch);
  mongo_teardown(&cn);
  return buf;
}

char *
search_dispatch_by_user_audience(uint64_t user_id, uint64_t *audience,
												 int audience_size, int req_num, int *result, int *length){
/* 
 * Takes a user id of the dispatch poster, a list of user
 * ids of the audience, an upper limit of dispatches to 
 * query for, and a pointer to an integer to be updated
 * with the number of dispatches matching the dispatch 
 * id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query.
 */
  bson_t *target_dispatch;  
	bson_t child;
  mongoc_cursor_t *cursor;   
  char query_buffer[DISPATCH_ARRAY_INDEX];
	int n;
	struct mongo_connection cn;

	cn.uri_string = MONGO_URI;

  n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connect failed in search_dispatch_by_user_audience\n");
		return NULL;
	}
  target_dispatch = bson_new();
	
	if(audience_size == 0){
		target_dispatch = BCON_NEW (
			  "$and", "[",
				"{", "user_id", BCON_INT64(user_id), "}",
				"{", "audience_size", BCON_INT32(0), "}",
				"]"
			);
	}
	else{
		BSON_APPEND_INT64(target_dispatch, "user_id", user_id);
		BSON_APPEND_ARRAY_BEGIN (target_dispatch, "audience", &child);
		for (int i = 0; i < audience_size; ++i) {
		  memset(query_buffer, '\0', DISPATCH_ARRAY_INDEX);
		  sprintf(query_buffer, "%d", i);
		  BSON_APPEND_INT64(&child, query_buffer, audience[i]);
		}
		bson_append_array_end (target_dispatch, &child);
	}

	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_dispatch_by_user_audience\n");
		bson_destroy(target_dispatch);
		mongo_teardown(&cn);
		return NULL;
	}

	*result = 0;
	char *buf = build_json(cursor, req_num, result, length);
	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	mongo_teardown(&cn);	
	return buf; 
}

char *
search_dispatch_by_parent_id(uint64_t dispatch_id, int req_num, 
						int *result, int *length){
/* 
 * Takes a dispatch id, an upper limit of dispatches to
 * query for, and a pointer to an integer to be updated
 * with the number of dispatches matching the dispatch
 * id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the query, or NULL on failure. A failure in
 * build_json will return NULL, which will be returned in buf
 * regardless, so error-checking is unnecessary.
 */
  bson_t *target_dispatch;  
	bson_t child;
  mongoc_cursor_t *cursor;
	int n;
	struct mongo_connection cn;

	cn.uri_string = MONGO_URI;

  n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connect failed in search_dispatch_by_parent_id\n");
		return NULL;
	}
  target_dispatch = bson_new();
  
  BSON_APPEND_DOCUMENT_BEGIN(target_dispatch, "dispatch_parent", &child);
	BSON_APPEND_INT32(&child, "type", (int32_t) 1);
	BSON_APPEND_INT64(&child, "id", dispatch_id); 
  bson_append_document_end(target_dispatch, &child);

	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_dispatch_by_parent_id\n");
		bson_destroy(target_dispatch);
		mongo_teardown(&cn);
		return NULL;
	}
	*result = 0;		
	char *buf = build_json(cursor, req_num, result, length);

	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	bson_destroy(&child);
	mongo_teardown(&cn);
	return buf; 
}

char *
search_dispatch_by_tags(const char* query, int req_num, int *result, int *length){
/* 
 * Takes a string query that describes a tag, an upper
 * limit of dispatches to query for, and a pointer to 
 * an integer to be updated with the number of 
 * dispatches matching the dispatch id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query, or NULL on failure.
 */
	bson_t *target_dispatch;
	mongoc_cursor_t *cursor;
	int n;	
	struct mongo_connection cn;

	cn.uri_string = MONGO_URI;	

	n =	mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connect failed in search_dispatch_by_tags\n");
		return NULL;
	}
	target_dispatch = bson_new();

  BSON_APPEND_UTF8(target_dispatch, "tags", query);
	
	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	if(cursor == NULL){
		printf("invalid cursor in search_dispatch_by_parent_id\n");
		bson_destroy(target_dispatch);
		mongo_teardown(&cn);
		return NULL;
	}
	*result = 0;
	char *buf = build_json(cursor, req_num, result, length);

	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	mongo_teardown(&cn);
	return buf;

}

char *
search_dispatch_by_user_tags(uint64_t query, int req_num, int *result, int *length){
/* 
 * Takes a user id of a tagged user, an upper
 * limit of dispatches to query for, and a pointer to 
 * an integer to be updated with the number of 
 * dispatches matching the dispatch id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query, or NULL on failure.
 */
  bson_t *target_dispatch;
  mongoc_cursor_t *cursor;
	int n;
  struct mongo_connection cn;

  cn.uri_string = MONGO_URI;

	n = mongo_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	if(n < 0){
		printf("mongo connect failed in search_dispatch_by_user_tags\n");
		return NULL;
	}
  target_dispatch = bson_new();

  BSON_APPEND_INT64(target_dispatch, "user_tags", query);
  
  cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
  if(cursor == NULL){
		printf("invalid cursor in search_dispatch_by_user_tags\n");
		bson_destroy(target_dispatch);
		mongo_teardown(&cn);
		return NULL;
	}
	*result = 0;
  char *buf = build_json(cursor, req_num, result, length);

  mongoc_cursor_destroy(cursor);
  bson_destroy(target_dispatch);
  mongo_teardown(&cn);
  return buf;

}


int
parse_dispatch_bson(struct dispatch *dis, const bson_t *bson_dispatch)
{
/*
 * Takes a pointer to a dispatch struct and fills it with the
 * contents of a bson document by parsing the contents of the
 * document.
 * Returns -1 upon an error, otherwise, returns 0.
 */
	struct dispatch_body *body;
	struct	dispatch_parent *parent;
	bson_iter_t iter;
	bson_iter_t sub_iter;
	const uint8_t *media;
	const char *text;
	uint32_t media_size, media_size_actual;
	uint32_t text_len;	
	int fields;

	if(dis == NULL){
		printf("invalid dispatch pointer in parse_dispatch_bson\n");
		goto parse_dispatch_error;
	}
	if((body = malloc(sizeof(struct dispatch_body))) == NULL){
		perror("malloc(body)");
		goto parse_dispatch_error;
	}
	if((parent = malloc(sizeof(struct dispatch_parent))) == NULL){
		perror("malloc(parent)");
		goto parse_bson_body;
	}
		
	fields = 0;

	bson_iter_init(&iter, bson_dispatch);

	if(bson_iter_find_descendant(&iter, "body.media_size", &sub_iter)){
		media_size = bson_iter_int32(&sub_iter);
		body->media_size = media_size;	
		fields++;
	}
	if(bson_iter_next(&sub_iter)){
		bson_iter_binary(&sub_iter, NULL, &media_size_actual, &media);
		//Check the stored size of the media against the size of the media
		//we just pulled from the bson
		if((media_size != media_size_actual)  || 
			((body->media = malloc(media_size_actual)) == NULL)){
			perror("malloc(body->media)");
			printf("media size actual = %d\n", media_size_actual);
			goto parse_bson_parent;
		}
		memcpy(body->media, media, media_size_actual);
		fields++;
	}
	if(bson_iter_next(&sub_iter)){
		text =  bson_iter_utf8(&sub_iter, &text_len);
		if((body->text = malloc(text_len)) == NULL){
			perror("malloc(body->text)");
			goto parse_bson_media;
		}
		body->text = strdup(text);
		fields++;
	}
	dis->body = body;

	/* user_id and timestamp */
	if(bson_iter_find(&iter, "user_id")){
		dis->user_id = bson_iter_int64(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "timestamp")){
		dis->timestamp = bson_iter_time_t(&iter);
		fields++;
	}

	/* Pulling data from the audience array */
	if(bson_iter_find(&iter, "audience_size")){
		dis->audience_size = bson_iter_int32(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "audience")){
		fields++;
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
			/* iterate through the new bson document and pull data (where key == index) */
			while(bson_iter_next(&sub_iter) && i < dis->audience_size){
				dis->audience[i] = bson_iter_int64(&sub_iter);
				i++;
			}
		bson_destroy(audience_array);
		}
	}

	/* Pulling data from the tags array */ 
	if(bson_iter_find(&iter, "num_tags")){
		dis->num_tags = bson_iter_int32(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "tags")){
		fields++;
		if(BSON_ITER_HOLDS_ARRAY(&iter) && dis->num_tags != 0){
			const uint8_t *array = NULL;
			uint32_t array_len = 0;			
			int i = 0;										
			bson_iter_array(&iter, &array_len, &array);
			bson_t *tags_array = bson_new_from_data(array, array_len);
			bson_iter_init(&sub_iter, tags_array);	
			while(bson_iter_next(&sub_iter) && i < dis->num_tags){
				const char *tag_string = bson_iter_utf8(&sub_iter, NULL);
				if(strlen(tag_string) >= MAX_TAG_SIZE){
					printf("tag has length which exceded maximum\n");
					goto parse_bson_media;
				}
				strcpy(dis->tags[i], bson_iter_utf8(&sub_iter, NULL));
				i++;
			}	
		bson_destroy(tags_array);
		}
	}

	/* Pulling data from the user_tags array */
	if(bson_iter_find(&iter, "num_user_tags")){
		dis->num_user_tags = bson_iter_int32(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "user_tags")){
		fields++;
		if(BSON_ITER_HOLDS_ARRAY(&iter) && dis->num_user_tags != 0){
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
		parent->type = bson_iter_int32(&sub_iter);
		fields++;
	}
	if(bson_iter_next(&sub_iter)){
		parent->id = bson_iter_int64(&sub_iter);  
		fields++;
	}
	dis->parent = parent;

	/* fragmentation and dispatch_id */
	if(bson_iter_find(&iter, "fragmentation")){
		dis->fragmentation = bson_iter_int32(&iter);
		fields++;
	}
	if(bson_iter_find(&iter, "dispatch_id")){
		dis->dispatch_id = bson_iter_int64(&iter);
		fields++;
	}
	if(fields == NUM_DISPATCH_FIELDS){
		return 0;
	}
	else{
		return -1;
	}

parse_bson_media:
	free(body->media);
parse_bson_parent:
	free(parent);
parse_bson_body:
	free(body);
parse_dispatch_error:
	return -1;
}




int
print_dispatch_struct(struct dispatch *dis){
/*
 * Prints a formated version of a dispatch struct
 */
	char *time_as_string;

	if(dis == NULL){
		printf("null dispatch pointer in print_dispatch_struct\n");	
	}
	printf("-----------------------------------------------------------------------\n");
	printf("body: \n");	
	printf("	media_size: %d\n",dis->body->media_size);
	printf("	text: %s\n", dis->body->text);
	printf("user_id: %ld\n",dis->user_id);	
	time_as_string = ctime(&dis->timestamp);		
	printf("timestamp: %s \n",(time_as_string));	
	printf("audience_size: %d\n", dis->audience_size);	
	for(int i = 0; i < dis->audience_size; i++){
		printf("	audience user_id: %ld\n", (dis->audience[i]));
	}
	printf("num_tags: %d\n", dis->num_tags);
	for(int i = 0; i < dis->num_tags; i++){
		if(strcmp(dis->tags[i], "") == 0){
			break;
		}
		else{
			printf("	tags: %s\n", (dis->tags[i]));
		}
	}		
	printf("num_user_tags %d\n", dis->num_user_tags);
	for(int i = 0; i < dis->num_user_tags; i++){
		if(dis->user_tags[i] > 0){
			printf("	tagged user_id: %ld\n", (dis->user_tags[i]));
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

void
dispatch_heap_cleanup(struct dispatch *dis){
/* Free's memory associated with pointers related
 * to dispach struct that are initialized in
 * parse_dispatch_bson
 * This function should be called after calling
 * the parse_dispatch_bson function
 */
	if(dis == NULL){
		printf("NULL dispatch pointer in dispatch_heap_cleanup\n");
		return;
	}
	if(dis->body == NULL){
		printf("NULL dispatch body pointer in dispatch_heap_cleanup\n");
		return;
	}
	if(dis->parent == NULL){
		printf("NULL dispatch parent pointer in dispatch_heap_cleanup\n");
		return;
	}
	free(dis->body->media);
	free(dis->body->text);
	free(dis->body);
	free(dis->parent);
}

int
handle_dispatch_bson(bson_t *doc)
{
/*
 * Takes a bson document w/ data describing a dispatch.
 * Parses data to an instance of a dispatch struct, 
 * uses the dispatch_id and search_dispatch_by_id function 
 * to identify and delete duplicate dispatches, inserting the
 * new dispatch in the process.
 * Returns 0 upon success, -1 otherwise.
 */
 
	//parse to user struct
  struct dispatch new_dis;
  int result;
	int length;

	if(doc == NULL){
		printf("invalid doc pointer in handle_dispatch_bson\n");
		return -1;
	}

  if(parse_dispatch_bson(&new_dis, doc) < 0){
    printf("error parsing to dispatch struct\n");
    return -1;
  }

  //search for duplicate by dispatch id
  char * buf = search_dispatch_by_id(new_dis.dispatch_id, 1, &result, &length);
	free(buf);

  if(result > 0 && delete_dispatch(new_dis.dispatch_id) < 0){
    printf("deletion of duplicate failed\n");
    return -1;
  }

  //insert the dispatch from the new_dis struct  
  if(insert_dispatch(&new_dis) != 0){
    printf("insertion from struct failed\n");
    return -1;
  }
  return 0;
}
