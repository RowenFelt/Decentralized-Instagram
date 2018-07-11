#include <mongoc.h>
#include <bson.h>
#include <time.h>
#include <string.h>

int 
main()
{
	const char *uri_string = "mongodb://localhost:27017";
  mongoc_uri_t *uri;
  mongoc_client_t *client;
  mongoc_database_t *database;
  mongoc_collection_t *collection;
  bson_t *insert_dispatch;
  bson_error_t error;

  /*
   * Required to initialize libmongoc's internals
   */
  mongoc_init ();

  /*
   * Optionally get MongoDB URI from command line
   */
  if (argc > 1) {
		uri_string = argv[1];
  }

  /*
	 * Safely create a MongoDB URI object from the given string
   */
  uri = mongoc_uri_new_with_error (uri_string, &error);
  if (!uri) {
		fprintf (stderr,
	          "failed to parse URI: %s\n"
            "error message:       %s\n",
            uri_string,
            error.message);
    return EXIT_FAILURE;
  }

  /*
   * Create a new client instance
   */
  client = mongoc_client_new_from_uri (uri);
  if (!client) {
		return EXIT_FAILURE;
  }

  /*
   * Register the application name so we can track it in the profile logs
   * on the server. This can also be done from the URI (see other examples).
   */
  mongoc_client_set_appname (client, "temp_dispatch_app");

  /*
   * Get a handle on the database "insta" and collection "dispatch"
   */
  database = mongoc_client_get_database (client, "insta");
  collection = mongoc_client_get_collection (client, database, "dispatch");
	
	/*
   * Release our handles and clean up libmongoc
   */
  mongoc_database_destroy (database);
  mongoc_uri_destroy (uri);
  mongoc_client_destroy (client);
  mongoc_cleanup ();

  return EXIT_SUCCESS;
}



/****FUNCTIONS THAT ARE SOLELY FOR ADDING A DISPATCH BEGIN HERE*******/
/* TODO: setup and teardown, what args does my function need to keep track of? */
int 
insert_dispatch(struct dispatch *dis) {
	
	bson_t *dispatch;   //The dispatch
	bson_t child;				//A temp bson_t doc that gets unitialized and cleared when used


	char buf[16];				//Buffer to build index 'keys' for an array of user id's
	const char *key;		//For char* representations of numerical indexes refferenced above 
	size_t keylen = 0;  //The length of the converted index keys TODO: see if macro implimentation renders this variable unessesary 	
	char* str;					//All-pourpose string used in builing sub docs and arrays
	

  /*
   * Craft a new BSON document using the fields of a dispatch object, then
   * insert in to the dispatch collection
   */
	dispatch = bson_new();

	/* Dispatch body comprised of media and text */
	BSON_APPEND_DOCUMENT_BEGIN(dispatch, "body", &child);
	BSON_APPEND_UTF8(&child, "media_path", dis->body.media_path); //the path to dispatch media
	BSON_APPEND_UTF8(&child, "text", dis->body.text); //the text/caption for the dispatch
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
			keylen = bson_uint32_to_string (i, &key, str, sizeof (str));
			bson_append_int64(&child, key, (int) keylen, dis->audience[i]);	
		}
		bson_append_array_end(dispatch, &child);	
	}
	

	/* Insert any hashtags as sub-array */
	/*TODO: verify that this poiner arithmetic is valid */		
	BSON_APPEND_ARRAY_BEGIN(dispatch, "tags", &child);
	for (uint32_t i = 0; i < MAX_NUM_TAGS; i++){
		if(*(dis->tags + i) != NULL){
			keylen = bson_uint32_to_string(i, &key, str, sizeof(str));
			BSON_APPEND_UTF8(&child, key, (int) keylen, *(dis->tags + i));	
		}
		else{
			break; //Reached a NULL pointer marking the end of the list of tags
		}	
	}
	bson_append_array_end(dispach, &child);


	/* Insert any tagged users as sub-array */
	BSON_APPEND_ARRAY_BEGIN(dispatch, "user_tags", &child);
	for (uint32_t i = 0; i < MAX_GROUP_SIZE; i++){
		if (dis->user_tags[i] > 0){
			keylen = bson_uint32_to_string(i, &key, str, sizeof(str));
			BSON_APPEND_UTF8(&child, key, (int) keylen, dis->user_tags[i]);	
		}
	}	
	bson_append_array_end(dispach, &child);
	

	/* Insert the id of the parent, which is a user or a dispatch (both have the same type) */
	BSON_APPEND_INT64(dispatch, "parent_id", dis->parent_id);
	BSON_APPEND_INT32(dispatch, "fragmentation", dis->fragmentation);
	BSON_APPEND_INT64(dispatch, "dispatch_id", dis->dispatch_id);

	/* Printing the document as a JSON string for error checking */
	str = bson_as_cononical_extended_json(dispatch, NULL);
	printf("%s\n", str);
	bson_free(str);	


  if (!mongoc_collection_insert_one (collection, dispactch, NULL, NULL, &error)) {
     fprintf (stderr, "%s\n", error.message);
  }

	/* clean up bson doc and collection */
  bson_destroy (dispatch);
  mongoc_collection_destroy (collection);
	return 0;
}
