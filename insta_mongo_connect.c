/*
 * insta_mongo_connect.c
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
#include "insta_mongo_connect.h"

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
    return EXIT_FAILURE;
  }

  /* Create a new client instance */
  cn->client = mongoc_client_new_from_uri (cn->uri);
  if (!cn->client) {
    return EXIT_FAILURE;
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

