/* 
 * cass_user.c
 * Interface for Cassandra user database
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <cassandra.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

static int init_error(CassFuture* statement_future);

int keyspace_table_init(char* keyspace, char* table){
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
  CassStatement* create_keyspace;
  CassStatement* create_table;
	CassFuture* statement_future; // will be used to track execution status   

  /* Create keyspace */
  char create_keyspace_query[1024];
  sprintf(create_keyspace_query, "CREATE KEYSPACE %s WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 2}", keyspace);

  create_keyspace = cass_statement_new(create_keyspace_query, 0);

  statement_future = cass_session_execute(session, create_keyspace);

  // free keyspace statement
  cass_statement_free(create_keyspace);

  // error check
  if(init_error(statement_future)){
    printf("Create keyspace result: %s\n", cass_error_desc(rc));
  }

  /* Create table */
  char create_table_query[1024];
  sprintf(create_table_query, "CREATE TABLE %s.%s(%s %s PRIMARY KEY, %s %s)",
      keyspace, table, primary_field, primary_type, field_1, type_1);

  create_table = cass_statement_new(create_table_query, 0);
  statement_future = cass_session_execute(session, create_table);

  // free table statement
  cass_statement_free(create_table);
  // error check
  if(init_error(statement_future)){
    printf("Create table result: %s\n", cass_error_desc(rc));
  }

  /* Free the session, cluster, and statement future */
  cass_future_free(statement_future);
  cass_session_free(session);
  cass_cluster_free(cluster);

  return 0;

}


int add_user(char* username, char* ip){
	uint32_t ip_addr;
	CassError err_code;
	CassCluster* cluster;
	CassSession* session;
	CassFuture* connect_future;
	CassStatement* add_user_statement;
	CassFuture* insert_future;
	
	
	//store ip as uint32_t using string to ip method
  ip_addr = string_to_ip(ip);


  /* SETTING UP THE CASSANDRA CONNECTION */
  cluster = cass_cluster_new();
  session = cass_session_new();

  // add contact points -- are these the same as seeds?
  err_code = cass_cluster_set_contact_points(cluster," 127.0.0.1"); //That's Ricker's ip

  if(err_code == CASS_OK){
    printf("Contact poins set successfully.\n");
  }
  else{
    printf("%s\n", cass_error_desc(err_code));
  }

  // connect to the cluster and wait until the futer variable returns, then 
  // error check it
  connect_future = cass_session_connect(session, cluster);
  err_code = cass_future_error_code(connect_future); //blocks till return

  //prints a string description of the error code
  printf("Connection status: %s\n", cass_error_desc(err_code));

  /* INSERTING THE NEW USER */
  //cassandra statement variable
  add_user_statement= cass_statement_new("INSERT INTO insta.user (name, ip_addr) VALUES (?, ?)", 2);

  //binding command line args (user_name and ip) to the INSERT statement from above
  cass_statement_bind_string(add_user_statement, 0, username);
	cass_statement_bind_int32(add_user_statement, 1, ip_addr);

  //it looks like we can use the CassFuture return type to do some error checking
  // -- worth reading throught that section of the api for more details and 
  //    finding out how to interperate the status of a variable of type CassFuture 
  insert_future = cass_session_execute(session, add_user_statement);
  err_code = cass_future_error_code(insert_future); //blocks till return


  printf("Insertion status: %s\n", cass_error_desc(err_code));

  /* CLEANUP OF CASSANDRA VARS */
  cass_statement_free(add_user_statement);
  cass_future_free(connect_future);  // Free the future from our connect command
  cass_future_free(insert_future); // Free the future from our insert command
  cass_session_free(session); // Free the session
  cass_cluster_free(cluster); //Free the cluster

	return 0;
}


static int init_error(CassFuture* statement_future){
	CassError rc = cass_future_error_code(statement_future);
	if(rc == CASS_ERROR_SERVER_ALREADY_EXISTS){
		return 1;
	}
	if(rc == CASS_OK){
		return 1;
	}
	else{
		return 0;
	}  
}

