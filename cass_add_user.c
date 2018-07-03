#include <cassandra.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


void usage(char* argv[]);

/* Things not directly addressed in this that need to be:
 * How to make a key space (insta) if this is the very first user:
 * 	Is this important to automate, or is it safe to assume that it will be done
 * 	manually at start-up
 * How to create table (users): 
 * 	Can we work off the same assumptions as deliniated above?
 */
int main(int argc, char* argv[]){
	// variables filled from the command line
	char* user_name;
	uint32_t ip_addr;
	
	//handle command line args
	if(argc != 3){
		usage(argv);
		exit(1);
	}

	user_name = argv[1];

	//store ip as uint32_t using string to ip method
	ip_addr = string_to_ip(argv[2]);


	/* SETTING UP THE CASSANDRA CONNECTION */
	CassCluster* cluster = cass_cluster_new();
	CassSession* session = cass_session_new();
	
	// add contact points -- are these the same as seeds?
	cass_cluster_set_contact_points(cluster," 140.233.20.181"); //That's Ricker's ip

	// connect to the cluster and wait until the futer variable returns, then 
	// error check it
	CassFuture* connect_future = cass_session_connect(session, cluster);
	CassError err_code = cass_future_error_code(connect_future); //blocks till return

	//prints a string description of the error code
	prinft("Connection status: %s\n", cass_error_desc(err_code));

	/* INSERTING THE NEW USER */
	//cassandra statement variable
	CassStatement* add_user_statement= cass_statement_new("INSERT INTO insta.users 
			(user_name, ip_addr) VALUES (?, ?)", 2);
	
	//binding command line args (user_name and ip) to the INSERT statement from above
	cass_statement_bind_string(add_user, 0, user_name);
	cass_statement_bind_int32(add_user, 1, ip_addr);

	//it looks like we can use the CassFuture return type to do some error checking
	// -- worth reading throught that section of the api for more details and 
	//    finding out how to interperate the status of a variable of type CassFuture 
	CassFuture* insert_future = cass_session_execute(session, add_user_statement);
	err_code = cass_future_error_code(insert_future); //blocks till return
	

	prinft("Insertion status: %s\n", cass_error_desc(err_code));

	/* CLEANUP OF CASSANDRA VARS */
	cass_error_result_free(err_code);
	cass_statement_free(add_user_statement);
	cass_future_free(connect_future);  // Free the future from our connect command
	cass_future_free(insert_future); // Free the future from our insert command
	cass_session_free(session); // Free the session
	cass_cluster_free(cluster); //Free the cluster
	//TODO: look in to the convention of freeing -- do we always want to free everything?
	//What happens if we forget to free?
	
}

void usage(char* argv[]){
	printf("usage: %s user_name ip_addr\n", argv[0]);
}

