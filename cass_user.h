/*
 * cass_user.h
 * An interface with the insta.user database
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _CASS_USER
#define _CASS_USER

#include <cassandra.h>

struct cass_connect {
	CassError err_code;
  CassCluster* cluster;
  CassSession* session;
  CassFuture* connect_future;
};

int keyspace_table_init(char* keyspace, char* table);
int add_user(uint64_t user_id, char* username, char* ip);
int get_user_ip_by_username(char* keyspace, char* table, char* username);
char * get_user_ip_by_id(char* keyspace, char* table, uint64_t user_id);


#endif /* _CASS_USER */
