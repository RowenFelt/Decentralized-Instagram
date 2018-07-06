/*
 * cass_user.h
 * An interface with the insta.user database
 * Authors: Rowen Felt and Campbell Boswell
*/

#ifndef _CASS_USER
#define _CASS_USER

int keyspace_table_init(char* keyspace, char* table);
int add_user(char* username, char* ip);

#endif /* _CASS_USER */
