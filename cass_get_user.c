#include <cassandra.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "util.h"

#define KEYSPACE "insta"
#define TABLE "user"

void usage(char* argv[]);

int main(int argc, char* argv[]){

	if(argc != 2){
		usage(argv);
		exit(1);
	}

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
	char* query_target = argv[1];//The user's name provided from the command line
	uint32_t ip; //The IP address we will return

	/* construct the query statement */
	char get_user_query[1024];
	sprintf(get_user_query, "SELECT %s FROM %s.%s WHERE %s='%s'",
			return_column, KEYSPACE, TABLE, query_column, query_target);
		
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
       	cass_result_free(user_query_result);	

	//get value from specific column - returns CassValue
	// If we are finding IP addresses of nodes users and/or their mirrors, and
	// we are searching for a unique user id, then here we can check multiple columns
	// to find multiple IP's and then return all of them to the higher layer
	const CassValue* cass_ip = cass_row_get_column_by_name(first_row, return_column); 

	//get a uint32_t from the CassValue - returns cass_int32_t
	if(cass_value_get_uint32(cass_ip, &ip) != CASS_OK){
		printf("Error converting cass value to standard value");
	}

	cass_session_free(session);
	cass_cluster_free(cluster);

	printf("%s's ip address is: %s\n", query_target, string_to_ip(ip));
	return ip;
}



void usage(char* argv[]){
	printf("usage: %s user_name\n", argv[0]);
}	
