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
  char* type_1 = "inet";


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
	CassError err_code;
	CassCluster* cluster;
	CassSession* session;
	CassFuture* connect_future;
	CassStatement* add_user_statement;
	CassFuture* insert_future;
	CassInet ip_inet;
	
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
	cass_inet_from_string(ip, &ip_inet);
	cass_statement_bind_string(add_user_statement, 0, username);
	cass_statement_bind_inet(add_user_statement, 1, ip_inet);

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


int get_user_ip(char* keyspace, char* table, char* username){
	/* SETUP CASSANDRA CONNECTION */
  CassCluster* cluster = cass_cluster_new();
  CassSession* session = cass_session_new();

  cass_cluster_set_contact_points(cluster, "127.0.0.1");

  CassFuture* connection_future = cass_session_connect(session, cluster);

  CassError rc = cass_future_error_code(connection_future);

  /* Check error code rc */
  if (rc != CASS_OK){
    /* Display connection error message */
    printf("Connection result: %s\n", cass_error_desc(rc));
  }

  /* Free connection future */
  cass_future_free(connection_future);


  /* BUILD AND EXECUTE USER QUERY */
  char* return_column = "ip_addr"; //The name of the column we want returned
  char* query_column = "name"; //The name of the column we are querying(primary column) 
  char* query_target = username;//The user's name provided from the command line

  /* construct the query statement */
  char get_user_query[1024];
  sprintf(get_user_query, "SELECT %s FROM %s.%s WHERE %s='%s'",
      return_column, keyspace, table, query_column, query_target);

  //craft query statement - returns CassStatement 
  CassStatement* get_user = cass_statement_new(get_user_query, 0);

  //execute statement - returns CassFuture
  CassFuture* statement_future = cass_session_execute(session, get_user);
  cass_statement_free(get_user); //Free statement
if((rc = cass_future_error_code(statement_future)) !=  CASS_OK){
    printf("Statement result: %s\n", cass_error_desc(rc));
  }

  //get results from future - returns CassResult
  const CassResult* user_query_result = cass_future_get_result(statement_future);
  cass_future_free(statement_future); //Free future

  if(user_query_result == NULL){
    exit(2);
  }

  //get first row from result - returns CassRow
  const CassRow* first_row = cass_result_first_row(user_query_result);
  // cass_result_free(user_query_result); 

  //get value from specific column - returns CassValue
  // If we are finding IP addresses of nodes users and/or their mirrors, and
  // we are searching for a unique user id, then here we can check multiple columns
  // to find multiple IP's and then return all of them to the higher layer
  const CassValue* cass_ip = cass_row_get_column_by_name(first_row, return_column);

  CassInet ip;
  //get a uint32_t from the CassValue - returns cass_int32_t
  if(cass_value_get_inet(cass_ip, &ip) != CASS_OK){
    printf("Error converting cass value to standard value\n");
  }

  char ip_string[16];
  cass_inet_string(ip, ip_string);

  printf("%s's ip address is: %s\n", query_target, ip_string);
  cass_session_free(session);
  cass_cluster_free(cluster);
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

