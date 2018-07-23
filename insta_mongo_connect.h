/* 
 * insta_mongo_connect.h
 * provides connection struct and functionality for mongo connection
 * Authors: Rowen Felt and Campbell Boswell
 */

#ifndef _INSTA_MONGO_CONNECT
#define _INSTA_MONGO_CONNECT

#include <mongoc.h>
#include <stdint.h>

#define INSTA_DB "insta"
#define USER_COLLECTION "users"
#define CASS_TABLE "user"
#define INSTA_DB "insta"
#define DISPATCH_COLLECTION "dispatch"


struct mongo_user_connection {
  char *uri_string;
  mongoc_uri_t *uri;
  mongoc_client_t *client;
  mongoc_database_t *database;
  mongoc_collection_t *collection;
  bson_error_t error;
};

int mongo_user_connect(struct mongo_user_connection *cn, char *db_name, char *coll_name);
int mongo_user_teardown(struct mongo_user_connection *cn);

#endif /* _INSTA_MONGO_CONNECT */

