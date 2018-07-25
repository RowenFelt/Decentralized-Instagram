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
#include "util.h"


int
insert_dispatch(struct dispatch *dis) {
/* 
 * Takes a dispatch struct and inserts it in the collection 
 * 'dispatch' (part of the 'insta' database).
 * Returns 0 upon successful insertion, -1 otherwise.
 */
  bson_t *dispatch; 
  bson_t child;    
  bson_error_t error;
  char buf[10];
  time_t rawtimestamp;

  struct mongo_user_connection cn;
  cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
	
  //check for duclication - don't insert if duplicate present 
  int duplicate = 0;
  int req_num = -1; 
  search_dispatch_by_id(dis->dispatch_id, req_num, &duplicate); 	
  if(duplicate > 0){
    return -1;
  }
  dispatch = bson_new();

  /* Dispatch body comprised of media and text */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "body", &child);
  BSON_APPEND_UTF8(&child, "media_path", dis->body->media_path);
  BSON_APPEND_UTF8(&child, "text", dis->body->text);
  
	bson_append_document_end(dispatch, &child); 
	BSON_APPEND_INT64(dispatch, "user_id", dis->user_id);
  rawtimestamp = time(NULL);
  if(rawtimestamp == (time(NULL) - 1)){
    (void) fprintf(stderr, "Faulty current time\n");
    exit(EXIT_FAILURE);
  }
  dis->timestamp = rawtimestamp;  
  BSON_APPEND_TIME_T(dispatch, "timestamp", dis->timestamp);
  BSON_APPEND_INT32(dispatch, "audience_size", dis->audience_size); 
  /* Store specific audience for a group message */
  if (dis->audience_size > MAX_GROUP_SIZE){
    return -1;
  }	
  BSON_APPEND_ARRAY_BEGIN(dispatch, "audience", &child);
  for (int i = 0; i < dis->audience_size; i++){
    memset(buf, '\0', 10);
    sprintf(buf, "%d", i);
    BSON_APPEND_INT64(&child, buf, dis->audience[i]);
  }
	bson_append_array_end(dispatch, &child);
  BSON_APPEND_INT32(dispatch, "num_tags", dis->num_tags);
  /* Insert any hashtags as sub-array */
	if(dis->num_tags > MAX_NUM_TAGS){
		return -1; 
	} BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
	for (int i = 0; i < dis->num_tags; i++){
		memset(buf, '\0', 10);
		sprintf(buf, "%d", i);
		BSON_APPEND_UTF8(&child, buf, dis->tags[i]);
	}
	bson_append_array_end(dispatch, &child);
	BSON_APPEND_INT32(dispatch, "num_user_tags", dis->num_user_tags);
	/* Insert any tagged users as sub-array */
	if (dis->num_user_tags >	MAX_NUM_TAGS){
		return -1;
	}	
	BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
	for (int i = 0; i < dis->num_user_tags; i++){
		memset(buf, '\0', 10);
		sprintf(buf, "%d", i);
		BSON_APPEND_INT64(&child, buf, dis->user_tags[i]);
	}
	bson_append_array_end(dispatch, &child);
	/* Insert dispatch_parent struct w/ parent's id */
  BSON_APPEND_DOCUMENT_BEGIN(dispatch, "dispatch_parent", &child);
  BSON_APPEND_INT32(&child, "type", dis->parent->type);
  BSON_APPEND_INT64(&child, "id", dis->parent->id); 
  bson_append_document_end(dispatch, &child);
	BSON_APPEND_INT32(dispatch, "fragmentation", dis->fragmentation);
  BSON_APPEND_INT64(dispatch, "dispatch_id", dis->dispatch_id);
	if (!mongoc_collection_insert_one (cn.collection, dispatch, NULL, NULL, &error)) {
		fprintf (stderr, "%s\n", error.message); 
	} 
  /* clean up bson doc and collection */
  bson_destroy (dispatch);
	return mongo_user_teardown(&cn);
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

  struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

  target_dispatch = bson_new();
	BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);

	if(!mongoc_collection_delete_one(cn.collection, target_dispatch, NULL, &reply, &error)){
		fprintf(stderr, "Delete failed: %s\n", error.message);
		return -1;
	}
	bson_destroy(target_dispatch);
	bson_destroy(&reply);
	
  mongo_user_teardown(&cn);

	return 0;
}



char *
search_dispatch_by_id(uint64_t dispatch_id, int req_num, int *result){ 
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

  struct mongo_user_connection cn;
  cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);
  
  target_dispatch = bson_new();
  BSON_APPEND_INT64(target_dispatch, "dispatch_id", dispatch_id);
  
  cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
 
	*result = 0;
	char* buf = build_json(cursor, req_num, result);	 
  mongoc_cursor_destroy(cursor);
  bson_destroy(target_dispatch);
  mongo_user_teardown(&cn);
  return buf;
}

char *
search_dispatch_by_user_audience(uint64_t user_id, uint64_t *audience,
																 int audience_size, int req_num, int *result){
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
  char query_buffer[10];

	struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

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
		  memset(query_buffer, '\0', 10);
		  sprintf(query_buffer, "%d", i);
		  BSON_APPEND_INT64(&child, query_buffer, audience[i]);
		}
		bson_append_array_end (target_dispatch, &child);
	}

	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);

	*result = 0;
	char *buf = build_json(cursor, req_num, result);
	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	mongo_user_teardown(&cn);	
	return buf; 
}

char *
search_dispatch_by_parent_id(uint64_t dispatch_id, int req_num, int *result){
/* 
 * Takes a dispatch id, an upper limit of dispatches to
 * query for, and a pointer to an integer to be updated
 * with the number of dispatches matching the dispatch
 * id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query.
 */
  bson_t *target_dispatch;  
	bson_t child;
  mongoc_cursor_t *cursor;
		
	struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

  target_dispatch = bson_new();
  
  BSON_APPEND_DOCUMENT_BEGIN(target_dispatch, "dispatch_parent", &child);
  BSON_APPEND_INT32(&child, "type", (int32_t) 1);
  BSON_APPEND_INT64(&child, "id", dispatch_id); 
  bson_append_document_end(target_dispatch, &child);

	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	
	*result = 0;		
	char *buf = build_json(cursor, req_num, result);

	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	bson_destroy(&child);
	mongo_user_teardown(&cn);
	return buf; 
}

char *
search_dispatch_by_tags(const char* query, int req_num, int *result){
/* 
 * Takes a string query that describes a tag, an upper
 * limit of dispatches to query for, and a pointer to 
 * an integer to be updated with the number of 
 * dispatches matching the dispatch id provided.
 * Returns a pointer to a JSON string containing the 
 * results from the  query.
 */
	bson_t *target_dispatch;
	mongoc_cursor_t *cursor;
	
	struct mongo_user_connection cn;
	cn.uri_string = "mongodb://localhost:27017";	

	mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

	target_dispatch = bson_new();

  BSON_APPEND_UTF8(target_dispatch, "tags", query);
	
	cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
	*result = 0;
	char *buf = build_json(cursor, req_num, result);

	mongoc_cursor_destroy(cursor);
	bson_destroy(target_dispatch);
	mongo_user_teardown(&cn);
	return buf;

}

char *
search_dispatch_by_user_tags(uint64_t query, int req_num, int *result){
/* 
 * Takes a user id of a tagged user, an upper
 * limit of dispatches to query for, and a pointer to 
 * an integer to be updated with the number of 
 * dispatches matching the dispatch id provided.
 * Returns a pointer to a JSON string w/ results 
 * from the  query.
 */
  bson_t *target_dispatch;
  mongoc_cursor_t *cursor;

  struct mongo_user_connection cn;
  cn.uri_string = "mongodb://localhost:27017";

  mongo_user_connect(&cn, INSTA_DB, DISPATCH_COLLECTION);

  target_dispatch = bson_new();

  BSON_APPEND_INT64(target_dispatch, "user_tags", query);
  
  cursor = mongoc_collection_find_with_opts(cn.collection, target_dispatch, NULL, NULL);
  *result = 0;
  char *buf = build_json(cursor, req_num, result);

  mongoc_cursor_destroy(cursor);
  bson_destroy(target_dispatch);
  mongo_user_teardown(&cn);
  return buf;

}


int
parse_dispatch_bson(struct dispatch *dis, const bson_t *bson_dispatch){
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
	const char *media_path;
	const char *text;
	uint32_t media_path_len;
	uint32_t text_len;	


	body = (struct dispatch_body *) malloc(sizeof(struct dispatch_body));
	parent = (struct dispatch_parent *) malloc(sizeof(struct dispatch_parent));
	
	if(body == NULL || parent == NULL){
		perror("malloc");
		return -1;
	}	


	bson_iter_init(&iter, bson_dispatch);

	if(bson_iter_find_descendant(&iter, "body.media_path", &sub_iter)){
		media_path = bson_iter_utf8(&sub_iter, &media_path_len);
		if((body->media_path = malloc(media_path_len)) == NULL){
			perror("malloc");
			return -1;
		}
		body->media_path = strdup(media_path);
	}
	if(bson_iter_next(&sub_iter)){
		text =  bson_iter_utf8(&sub_iter, &text_len);
		if((body->text = malloc(text_len)) == NULL){
			perror("malloc");
			return -1;
		}
		body->text = strdup(text);
	}
	dis->body = body;
	if(bson_iter_find(&iter, "user_id")){
		dis->user_id = bson_iter_int64(&iter);
	}
	if(bson_iter_find(&iter, "timestamp")){
		dis->timestamp = bson_iter_time_t(&iter);
	}
	
	//Pulling data from the tags array
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
	
	//Pulling data from the user_tags array 
	if(bson_iter_find(&iter, "num_user_tags")){
		dis->num_user_tags = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "user_tags")){
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

	// Fill dispatch_parent struct 
	if(bson_iter_find_descendant(&iter, "dispatch_parent.type", &sub_iter)){
		parent->type = bson_iter_int32(&sub_iter);
	}
	if(bson_iter_next(&sub_iter)){
		parent->id =  bson_iter_int64(&sub_iter);  
	}
	dis->parent = parent;
	if(bson_iter_find(&iter, "fragmentation")){
		dis->fragmentation = bson_iter_int32(&iter);
	}
	if(bson_iter_find(&iter, "dispatch_id")){
		dis->dispatch_id = bson_iter_int64(&iter);
	}
	return 0;
}




int
print_dispatch_struct(struct dispatch *dis){
/*
 * Prints a formated version of a dispatch struct
 */
	char *time_as_string;
	
	printf("-----------------------------------------------------------------------\n");
	printf("body: \n");	
	printf("	media_path: %s\n",dis->body->media_path);
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
	free(dis->body->media_path);
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

  if(parse_dispatch_bson(&new_dis, doc) < 0){
    printf("error parsing to dispatch struct\n");
    return -1;
  }

  //search for duplicate by dispatch id
  if(search_dispatch_by_id(new_dis.dispatch_id, 1, &result) != NULL){
    if(result > 0){
      if(delete_dispatch(new_dis.dispatch_id) < 0){
        printf("deletion of duplicate failed\n");
        return -1;
      }
    }
  }

  //insert the dispatch from the new_dis struct  
  if(insert_dispatch(&new_dis) != 0){
    printf("insertion from struct failed\n");
    return -1;
  }

	//cleanup the memory associated with the new dispatch we made
	dispatch_heap_cleanup(&new_dis);

  return 0;

}
