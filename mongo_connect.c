/*
 * mongo_connect.c
 * connection functionality for mongo database of insta project
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
#include "mongo_connect.h"
#include "user_definitions.h"
#include "dispatch_definitions.h"


int
mongo_connect(struct mongo_connection *cn, char *db_name, char *coll_name)
{
	/*  Required to initialize libmongoc's internals */
  mongoc_init();

	if(cn == NULL || db_name == NULL || coll_name == NULL){
		printf("invalida parameters to mongo_connect()\n");
		return -1;
	}

  /* Safely create a MongoDB URI object from the given string */
  cn->uri = mongoc_uri_new_with_error (cn->uri_string, &cn->error);
  if(!cn->uri){
    fprintf(stderr,
      "failed to parse URI: %s\n"
      "error message:       %s\n",
      cn->uri_string,
      cn->error.message);
    return -1;
  }

  /* Create a new client instance */
  cn->client = mongoc_client_new_from_uri(cn->uri);
  if(!cn->client){ 
    return -1;
  }

  /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples). */
  mongoc_client_set_appname(cn->client, INSTA_CLIENT);

  /* Get a handle on the database "db_name" and collection "coll_name" */
  cn->database = mongoc_client_get_database(cn->client, db_name);
  cn->collection = mongoc_client_get_collection(cn->client, db_name, coll_name);
  return 0;
}

int
mongo_teardown(struct mongo_connection *cn)
{
	/* Release our handles and clean up libmongoc */
  if(cn == NULL){
		return -1;
	}
	if(cn->collection == NULL){
		return -1;
	}
	if(cn->database == NULL){
		return -1;
	}
	if(cn->uri == NULL){
		return -1;
	}
	if(cn->client == NULL){
		return -1;
	}
	mongoc_collection_destroy(cn->collection);
  mongoc_database_destroy(cn->database);
  mongoc_uri_destroy(cn->uri);
  mongoc_client_destroy(cn->client);
  mongoc_cleanup();
	return 0;
}

char*
build_json(mongoc_cursor_t *cursor, int req_num, int *result){
	/* builds a json object using a cursor from a mongo query. 
   * the returned buffer has been malloced to the correct size
   * and must be freed by the caller. Returns the json as a 
   * buffer on success, or NULL on failure */

	const bson_t *result_dispatch;
	size_t json_length;	
	int buf_size;
	bson_error_t error;
	char* buf = NULL;	
  
	buf_size = 0;
	*result = 0;

	if(req_num == -1){
		req_num = INT_MAX;
	}

	if(cursor == NULL){
		printf("invalid cursor argument in build_json()\n");
		return NULL;
	}

	/* buf is set to NULL, so only necessary to return buf in case of errors */
	while(mongoc_cursor_next(cursor, &result_dispatch) && *result < req_num){
    char *json_str;
    json_str = bson_as_canonical_extended_json(result_dispatch, &json_length);
		if(json_str == NULL){
			break;
		}
    buf_size += json_length;
    buf = realloc(buf, buf_size);
    strncpy(buf + buf_size - json_length, json_str, json_length);
    *result += 1;
		bson_free(json_str);
  }

	if(mongoc_cursor_error(cursor, &error)){
		fprintf(stderr, "Failed to iterate through all documents: %s\n", error.message);
	}
	
	if( buf != NULL){
		buf = realloc(buf, buf_size +1);
		char *end;
		end = buf + buf_size;
		*end = '\0';

	}	
	return buf;
}


int
insert_json_from_fd(int fd, char *collection_name){
	/* reads json from a file descriptor and inserts
   * the json object as either a user or dispatch
   * into the appropriate mongo collection. This
   * function deletes duplicates if found. Returns 
   * 0 on success or -1 on failure. */

	bson_json_reader_t *json_reader;
	bson_t document =	BSON_INITIALIZER;
	bson_error_t error;
	int reader_status, num_docs_inserted;
	
	if(fcntl(fd, F_GETFD) == -1 || errno == EBADF){
    printf("File closed\n");
		return -1;
	} 

	//Connect to user specified collection
  struct mongo_connection cn;
  cn.uri_string = "mongodb://localhost:27017";
  mongo_connect(&cn, INSTA_DB, collection_name);
	
	//create new bson reader object that will read from the provided file descriptor
	//true - specified that the fd will be closed when we destroy json_reader
	json_reader = bson_json_reader_new_from_fd(fd, true);

	if(json_reader == NULL){
		goto insert_json_from_fd_error;
	}
	
	num_docs_inserted = 0;	
	while((reader_status = bson_json_reader_read(json_reader, &document, &error))){
		if(strcmp(collection_name, USER_COLLECTION) == 0){
			if(handle_user_bson(&document) < 0){
				printf("insertion failed\n");
				return -1;
			}	
		}
	
		else if(strcmp(collection_name, DISPATCH_COLLECTION) == 0){
			if(handle_dispatch_bson(&document) < 0){
				printf("insertion failed\n");
				bson_destroy(&document);
				goto insert_json_from_fd_error;
			}	
		}
	
		num_docs_inserted += reader_status;
		bson_reinit(&document);	
	} 
	
	if(reader_status < 0){
		printf("Read Error:%s\n", error.message);
		bson_destroy(&document);
		goto insert_json_from_fd_error;
	}
	
	//Breakdown connection and bson types
	bson_json_reader_destroy(json_reader); //Also closes fd
	if(fcntl(fd, F_GETFD) != -1 || errno != EBADF){
		printf("File NOT closed\n");
	}	
	bson_destroy(&document);
	mongo_teardown(&cn);

	return num_docs_inserted;

insert_json_from_fd_error:
	bson_json_reader_destroy(json_reader);
	mongo_teardown(&cn);
	return -1;

}
