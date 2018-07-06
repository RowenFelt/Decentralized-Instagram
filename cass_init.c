#include <cassandra.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define KEYSPACE "Insta"
#define TABLE "Users"


int main(){
	//Defining the names of the table and keyspace we are creating
	char* keyspace = KEYSPACE;
	char* table = TABLE;

	// Defining the columns (fields) of the table and their associated types
	char* primary_field = "name"; // The name of the primary field in a row	
	char* primary_type = "text";  // The CQL type (e.g. text, int) of said field
	char* field_1 = "ip_addr";
	char* type_1 = "int";


	/* SETUP AND CONNECT TO CLUSTER*/
	CassCluster* cluster = cass_cluster_new();
	CassSession* session = cass_session_new();

  	/* Add contact point (localhost or ricker) */
  	cass_cluster_set_contact_points(cluster, "127.0.0.1");

  	/* Provide the cluster object as configuration to connect the session */
  	CassFuture* connect_future = cass_session_connect(session, cluster);

  	/* This operation will block until the result is ready */
  	CassError rc = cass_future_error_code(connect_future);

  	if (rc != CASS_OK) {
  		/* Display connection error message */
  	 	const char* message;
  	 	size_t message_length;
    		cass_future_error_message(connect_future, &message, &message_length);
    		fprintf(stderr, "Connect error: '%.*s'\n", (int)message_length, message);
  		exit(1);
	}
 	/* free the connection future now that we've connected */	
 	cass_future_free(connect_future);
 

	/* CREATE KEYSPACE AND TABLE */
	
	// Look in to the best practices of declaring all these cass objects at once,
	// it might not be optimal as they all have to be free'd which may imply that
	// they are resource intensive...
//	CassStatement * create_keyspace; 
//	CassStatement * bind_to_keyspace;
//	CassStatement * create_table;
	CassFuture * statement_future; // will be used to track exicution status   

	/* Create keyspace */
	char create_keyspace_query[1024];
	sprintf(create_keyspace_query, "CREATE KEYSPACE %s WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 2}", KEYSPACE);
	
	CassStatement* create_keyspace = cass_statement_new(create_keyspace_query, 0);
	
	statement_future = cass_session_execute(session, create_keyspace);

	// free keyspace statement
	cass_statement_free(create_keyspace);

	// error check
	if((rc = cass_future_error_code(statement_future)) != CASS_OK){
		printf("Create keyspace result: %s\n", cass_error_desc(rc));
	}

	/* Bind to keyspace */
/*
	bind_to_keyspace = cass_statement_new("USE ?", 1);

	cass_statement_bind_string(bind_to_keyspace, 0, KEYSPACE); 
	
	statement_future = cass_session_execute(session, bind_to_keyspace);
	// free bind statement
	cass_statement_free(bind_to_keyspace);
	// error check
	if((rc = cass_future_error_code(statement_future) != CASS_OK){
		printf("Bind keyspace result: %s\n", cass_error_desc(rc));
*/
	/* Create table */
	char create_statement_query[1024];
	sprintf(create_keyspace_query, "CREATE TABLE %s.%s(%s %s PRIMARY KEY, %s %s);",
			KEYSPACE, TABLE, primary_field, primary_type, field_1, type_1);

	CassStatement *	create_table = cass_statement_new(create_statement_query, 0);
	statement_future = cass_session_execute(session, create_table);
	
	// free table statement
	cass_statement_free(create_table);
	// error check
	if((rc = cass_future_error_code(statement_future)) != CASS_OK){
		printf("Create table result: %s\n", cass_error_desc(rc));
	}

	/* Free the session, cluster, and statement future */
	cass_future_free(statement_future);
	cass_session_free(session);
  	cass_cluster_free(cluster);

  	return 0;
}
