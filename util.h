#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>
#include <mongoc.h>


/* TODO: put all the database and collection constants in insta_mongo_connect.h b/c they
 * are only really user in relation with functions from that header file. Also add error
 * checking to verify that the variables passed to mongo_user_connect match these expected 
 * constants
 */
#define INSTA_DB "insta"
#define DISPATCH_COLLECTION "dispatch"
#define USER_COLLECTION "users"

uint32_t string_to_ip(char* ipstr);
char* ip_to_string(uint32_t ip);
char* build_json(mongoc_cursor_t *cursor, int req_num, int *result);
int insert_json_from_fd(int fd, char *collection_name);//char * buf, size_t buf_size, char *collection_name);
#endif /* __UTIL_H */
