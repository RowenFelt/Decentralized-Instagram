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

#define INSTA_CLIENT "insta_client"

int
mongo_user_connect(struct mongo_user_connection *cn, char *db_name, char *coll_name)
{
	/*  Required to initialize libmongoc's internals */
  mongoc_init ();

  /* Safely create a MongoDB URI object from the given string */
  cn->uri = mongoc_uri_new_with_error (cn->uri_string, &cn->error);
  if (!cn->uri) {
    fprintf (stderr,
      "failed to parse URI: %s\n"
      "error message:       %s\n",
      cn->uri_string,
      cn->error.message);
    return -1;
  }

  /* Create a new client instance */
  cn->client = mongoc_client_new_from_uri (cn->uri);
  if (!cn->client) {
    return -1;
  }

  /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples). */
  mongoc_client_set_appname (cn->client, INSTA_CLIENT);

  /* Get a handle on the database "db_name" and collection "coll_name" */
  cn->database = mongoc_client_get_database (cn->client, db_name);
  cn->collection = mongoc_client_get_collection (cn->client, db_name, coll_name);
  return 0;
}

int
mongo_user_teardown(struct mongo_user_connection *cn)
{
	/* Release our handles and clean up libmongoc */
  mongoc_collection_destroy (cn->collection);
  mongoc_database_destroy (cn->database);
  mongoc_uri_destroy (cn->uri);
  mongoc_client_destroy (cn->client);
  mongoc_cleanup ();
	return 0;
}

char*
build_json(mongoc_cursor_t *cursor, int req_num, int *result){
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

	while(mongoc_cursor_next(cursor, &result_dispatch) && *result < req_num){
    char *json_str;
    json_str = bson_as_canonical_extended_json(result_dispatch, &json_length);
    buf_size += json_length;
    buf = realloc(buf, buf_size);
    strncpy(buf + buf_size - json_length, json_str, json_length);
    *result += 1;
		bson_free(json_str);
  }

	if(mongoc_cursor_error(cursor, &error)){
		fprintf(stderr, "Failed to itterate through all documents: %s\n", error.message);
	}
	
	if( buf != NULL){
		buf = realloc(buf, buf_size +1);
		char *end;
		end = buf+ buf_size;
		*end = '\0';

	}	
	return buf;
}


//TODO: It might be useful to add a variable to describe the size of the json so that
//we can try to determine if the entire json is parses and sucessfully inserted into 
//the collection.
int
insert_json_from_fd(int fd, char *collection_name){
	bson_json_reader_t *json_reader;
	bson_t document =	BSON_INITIALIZER;
	bson_error_t error;
	int a, num_docs_inserted;
	
	if(fcntl(fd, F_GETFD) == -1 || errno == EBADF){
    printf("File closed\n");
	} 

	//Connect to user specified collection
  struct mongo_user_connection cn;
  cn.uri_string = "mongodb://localhost:27017";

	//TODO: add error checking regarding collection names in mongo_connect
  mongo_user_connect(&cn, INSTA_DB, collection_name);
	
	//create new bson reader object that will read from the provided file descriptor
	//true - specified that the fd will be closed when we destroy json_reader
	json_reader = bson_json_reader_new_from_fd(fd, true);
	
	if(fcntl(fd, F_GETFD) == -1 || errno == EBADF){
    printf("File closed\n");
	} 

	
	while((a = bson_json_reader_read(json_reader, &document, &error))){
		if( a < 0){
			fprintf(stderr, "Read Error:%s\n", error.message);
			return -1;
		}
		
		if(strcmp(collection_name, USER_COLLECTION) == 0){
			if(handle_user_bson(&document) < 0){
				printf("insertion failed\n");
				return -1;
			}	
		}
	
		else if(strcmp(collection_name, DISPATCH_COLLECTION) == 0){
			if(handle_dispatch_bson(&document) < 0){
				printf("insertion failed\n");
				return -1;
			}	
		}
	
		num_docs_inserted += a;
		bson_reinit(&document);	
	} 
	
	
	//Breakdown connection and bson types
	bson_json_reader_destroy(json_reader); //Also closes fd
	if(fcntl(fd, F_GETFD) != -1 || errno != EBADF){
		printf("File NOT closed\n");
	}	
	bson_destroy(&document);
	mongo_user_teardown(&cn);

	return num_docs_inserted;
}
