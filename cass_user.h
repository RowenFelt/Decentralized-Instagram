/*
 * cass_user.h
 * An interface with the insta.user database
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _CASS_USER
#define _CASS_USER

int keyspace_table_init(char* keyspace, char* table);
int add_user(char* username, char* ip);
int get_user_ip(char* keyspace, char* table, char* username);

#endif /* _CASS_USER */
