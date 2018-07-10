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
#include "cass_user.h"

static int init_error(CassFuture* statement_future);
static int session_connection(struct cass_connect* connection);
static int tear_down_connection(struct cass_connect* connection);

int keyspace_table_init(char *keyspace, char *table){
	// Defining the columns (fields) of the table and their associated types
  char *primary_field = "name"; // The name of the primary field in a row 
  char *primary_type = "text";  // The CQL type (e.g. text, int) of said field
  char *field_1 = "ip_addr";
  char *type_1 = "inet";
	struct cass_connect connection;
  CassStatement *create_keyspace;
  CassStatement *create_table;
  CassFuture *statement_future; // will be used to track execution status 
	char create_keyspace_query[1024];
	char create_table_query[1024];

	/* connect to cluster */
  session_connection(&connection);

  /* CREATE KEYSPACE AND TABLE */

  /* Create keyspace */
	sprintf(create_keyspace_query, "CREATE KEYSPACE %s WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 2}", keyspace);
  create_keyspace = cass_statement_new(create_keyspace_query, 0);
  statement_future = cass_session_execute(connection.session, create_keyspace);

  // free keyspace statement
  cass_statement_free(create_keyspace);

  // error check
  if(init_error(statement_future)){
    printf("Create keyspace result: %s\n", cass_error_desc(connection.err_code));
  }

  /* Create table */
	sprintf(create_table_query, "CREATE TABLE %s.%s(%s %s PRIMARY KEY, %s %s)",
      keyspace, table, primary_field, primary_type, field_1, type_1);
  create_table = cass_statement_new(create_table_query, 0);
  statement_future = cass_session_execute(connection.session, create_table);

  // free table statement
  cass_statement_free(create_table);
  // error check
  if(init_error(statement_future)){
    printf("Create table result: %s\n", cass_error_desc(connection.err_code));
  }

  /* Free the session, cluster, and statement future */
	tear_down_connection(&connection);
  return 0;
}


int add_user(char *username, char *ip){
	CassStatement *add_user_statement;
	CassFuture *insert_future;
	CassInet ip_inet;
	struct cass_connect connection;

	session_connection(&connection);

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
  insert_future = cass_session_execute(connection.session, add_user_statement);
  connection.err_code = cass_future_error_code(insert_future); //blocks till return


  printf("Insertion status: %s\n", cass_error_desc(connection.err_code));

  /* CLEANUP OF CASSANDRA VARS */
  cass_statement_free(add_user_statement);
  cass_future_free(insert_future); // Free the future from our insert command
	tear_down_connection(&connection);
	return 0;
}


int get_user_ip(char *keyspace, char *table, char *username){
	/* SETUP CASSANDRA CONNECTION */
  struct cass_connect connection;
	char *return_column;
	char *query_column;
	char *query_target;
	char get_user_query[1024];
	CassStatement *get_user;
	CassFuture *statement_future;
	const CassResult *user_query_result;
	const CassRow *first_row;
	const CassValue *cass_ip;
	CassInet ip;
	char ip_string[16];
  
	session_connection(&connection);

  /* BUILD AND EXECUTE USER QUERY */
  return_column = "ip_addr"; //The name of the column we want returned
	query_column = "name"; //The name of the column we are querying(primary column) 
  query_target = username;//The user's name provided from the command line

  /* construct the query statement */
  sprintf(get_user_query, "SELECT %s FROM %s.%s WHERE %s='%s'",
      return_column, keyspace, table, query_column, query_target);

  //craft query statement - returns CassStatement 
  get_user = cass_statement_new(get_user_query, 0);

  //execute statement - returns CassFuture
  statement_future = cass_session_execute(connection.session, get_user);
  cass_statement_free(get_user); //Free statement
	if((connection.err_code = cass_future_error_code(statement_future)) !=  CASS_OK){
    printf("Statement result: %s\n", cass_error_desc(connection.err_code));
  }

  //get results from future - returns CassResult
  user_query_result = cass_future_get_result(statement_future);
  cass_future_free(statement_future); //Free future

  if(user_query_result == NULL){
    exit(2);
  }

  //get first row from result - returns CassRow
  first_row = cass_result_first_row(user_query_result);

  //get value from specific column - returns CassValue
  // If we are finding IP addresses of nodes users and/or their mirrors, and
  // we are searching for a unique user id, then here we can check multiple columns
  // to find multiple IP's and then return all of them to the higher layer
  cass_ip = cass_row_get_column_by_name(first_row, return_column);

  //get a uint32_t from the CassValue - returns cass_int32_t
  if(cass_value_get_inet(cass_ip, &ip) != CASS_OK){
    printf("Error converting cass value to standard value\n");
  }
 
  cass_inet_string(ip, ip_string);

  printf("%s's ip address is: %s\n", query_target, ip_string);
	tear_down_connection(&connection); 
	return 0;
}


static int init_error(CassFuture *statement_future){
	/* interprets init error code */
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


static int session_connection(struct cass_connect *connection){
	/* Initiates the Cassandra connection. Returns 0 on success, error code on failure */
  connection->cluster = cass_cluster_new();
  connection->session = cass_session_new();

  connection->err_code = cass_cluster_set_contact_points(connection->cluster," 127.0.0.1");

  if(connection->err_code == CASS_OK){
    printf("Contact poins set successfully.\n");
  }
  else{
    printf("%s\n", cass_error_desc(connection->err_code));
  }

  // connect to the cluster and wait until the futer variable returns, then 
  // error check it
  connection->connect_future = cass_session_connect(connection->session, connection->cluster);
  connection->err_code = cass_future_error_code(connection->connect_future); //blocks

  //prints a string description of the error code
  if(!init_error(connection->connect_future)){
		printf("Connection status: %s\n", cass_error_desc(connection->err_code));
		return connection->err_code;
	}
	return 0;
}


static int tear_down_connection(struct cass_connect *connection){
	/* frees all connection pointers */
	cass_future_free(connection->connect_future);  // Free the future from our connect command
	cass_session_free(connection->session); // Free the session
  cass_cluster_free(connection->cluster); //Free the cluster
	return 0;
}
