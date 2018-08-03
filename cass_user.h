/*
 * cass_user.h
 * An interface with the insta.user database
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _CASS_USER_H
#define _CASS_USER_H

#include <cassandra.h>


#define KEYSPACE "insta"
#define TABLE "user"

struct cass_connect {
	CassError err_code;
  CassCluster* cluster;
  CassSession* session;
  CassFuture* connect_future;
};

int keyspace_table_init(char* keyspace, char* table);
int add_user(uint64_t user_id, char* username, char* ip);
uint64_t * get_user_id_by_username(char* keyspace, char* table, char* username, int *result);
char * get_user_ip_by_id(char* keyspace, char* table, uint64_t user_id);


#endif /* _CASS_USER_H */
