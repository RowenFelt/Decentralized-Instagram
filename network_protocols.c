/*
 * network_protocols.c
 * text-based protocals for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <mongoc.h>
#include <bson.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "network_protocols.h"
#include "user_definitions.h"
#include "dispatch_definitions.h"
#include "mongo_connect.h"

#define LARGE_ID_SIZE 20

static uint64_t read_id(int fd);

struct insta_protocol {
	char protocol[INSTA_PROTOCOL_SIZE];
	int (*func)(int, int);
};
 
static int pull_all(int in, int out);
static int pull_child(int in, int out);
static int pull_dispatch(int in, int out);
static int pull_user(int in, int out);
static int pull_user_tags(int in, int out);
static int pull_tags(int in, int out);

static int push_child(int fd, int out);
static int push_user_tag(int fd, int out);
static int push_message(int fd, int out);
static int push_dispatch(int fd, int out);
static int push_user(int fd, int out); 

int
parse_server_command(int in, int out)
{	
	int result;
	int n = 0;
	char command[INSTA_PROTOCOL_SIZE];	

	n = read(in, command, INSTA_PROTOCOL_SIZE);	
	if(n < 0) {
		perror("read");
		return -1;
	}

	struct insta_protocol protocol_list[] = {
		{"pull all***** ", pull_all},
		{"pull child*** ", pull_child},
		{"pull dispatch ", pull_dispatch},
		{"pull user**** ", pull_user},
		{"pull user_tag ", pull_user_tags},
		{"pull tags**** ", pull_tags},
		{"push child*** ", push_child},
		{"push user_tag ", push_user_tag},
		{"push message* ", push_message},
		{"push dispatch ", push_dispatch},
		{"push user**** ", push_user},
	};

	int protocol_count = 11;
	int (*protocol_func)(int, int) = NULL;
	for(int i = 0; i < protocol_count; i++){
		if(memcmp(command, protocol_list[i].protocol, 
								INSTA_PROTOCOL_SIZE) == 0){
			protocol_func = protocol_list[i].func;
			break;
		}
	}

	if(protocol_func == NULL) {
		return -1;
	}
	
	result = protocol_func(in, out);
	
	return result;
}


/* LLONG_MIN and LLONG_MAX are invalid user_ids or dispatch_ids
 * this is defined in the user_definitions and 
 * dispatch_definitions. A value of 0 indicates that strtoll
 * was unsuccessful in reading a valid integer from string.
 */
static uint64_t 
read_id(int fd)
{
	char str[LARGE_ID_SIZE];
	uint64_t id;

	memset(str, '\0', LARGE_ID_SIZE);
	read(fd, str, LARGE_ID_SIZE);
	id = strtoll(str, NULL, 10);
	if(id == LLONG_MIN || id == LLONG_MAX || id == 0 ){
		printf("invalid ID in protocol field\n");
		return -1;
  }
	return id;
}


static int
pull_all(int in, int out){
	char *bson;
	uint64_t user_id;
	int result = 0;
	int n = 0;
	int length = 0;

	user_id = read_id(in);
	
	bson = search_dispatch_by_user_audience(user_id, NULL, 0, -1, &result, &length);
	if(bson == NULL){
		printf("PULL ALL failed, search function returned NULL\n");
		return -1;
	}
	n =	write(out, bson, length); 
	if(n < 0) {
		perror("pull_all write");
		return -1;
	}
	printf("pull all***** %ld successful\n", user_id);
	return result;
}

static int
pull_child(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t parent_id;
	int length = 0;
	
	parent_id = read_id(in);

	bson = search_dispatch_by_parent_id(parent_id, -1, &result, &length);
	if(bson == NULL){
		printf("PULL CHILD failed, search function returned NULL\n");
		return -1;
	}
	n =	write(out, bson, length);
	if(n < 0){
		perror("pull_child write");
		return -1;
	}
	printf("pull child*** %ld successful\n", parent_id);
	return result;
}

static int
pull_dispatch(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t dispatch_id;
	int length = 0;

	dispatch_id = read_id(in);
		
	bson = search_dispatch_by_id(dispatch_id, 1, &result, &length);
	if(bson == NULL){
		printf("PULL ONE failed, search function returned NULL\n");
		return -1;
	}   	
	n = write(out, bson, length);
	if(n < 0){
		perror("pull_dispatch write");
		return -1;
	}
	printf("pull dispatch %ld successful\n", dispatch_id);
	return result;
}

static int
pull_user(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t user_id;
	int length = 0;

	user_id = read_id(in);
	
	bson = search_user_by_id_mongo(user_id, -1, &result, &length);	
	if(bson == NULL){
		printf("PULL USER failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, length);
	if(n < 0){
		perror("pull_user write");
		return -1;
	}
	printf("pull user**** %ld successful\n", user_id);
	return result;
}



static int
pull_user_tags(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t user_id;
	int length = 0;

	user_id = read_id(in);
	
	bson = search_dispatch_by_user_tags(user_id, -1, &result, &length);	
	if(bson == NULL){
		printf("PULL USER TAGS failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, length);
	if(n < 0) {
		perror("pull_user_tags write");
		return -1;
	}
	printf("pull user_tags %ld successful\n", user_id);
	return result;
}


static int
pull_tags(int in, int out){
	char *bson, *query;
	int result = 0;
	int n = 0;
	int length = 0;
	char str[100];
	memset(str, '\0', 100);
	read(in, str, 100);

	if((query = strtok(str, " "))  == NULL){
		printf("Invalid argument for pull search\n");
		return -1;
	}
	
	/* delete newline characters, this might be a bad idea */
	char *newline;
	if ((newline=strchr(query, '\n')) != NULL){
    *newline = '\0';
	}
	
	bson = search_dispatch_by_tags(query, -1, &result, &length);	
	if(bson == NULL){
		printf("PULL TAGS failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, length);
  if(n < 0) {
    perror("pull_tags write");
    return -1;
  }
	printf("pull tags***** %s successful\n", query);
	return result;
}


/* TODO these functions all do the same thing,
 * but we chose to keep them because in the
 * future they will have to push updates to the
 * client under different conditions. */

static int
push_child(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	printf("push child successful, inserted %d dispatch objects\n", result);
	return result;
}


static int
push_user_tag(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	printf("push user_tag successful, inserted %d dispatch objects\n", result);
	return result;
}


static int
push_message(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	printf("push message successful, inserted %d dispatch objects\n", result);
	return result;
}


static int
push_dispatch(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	printf("push dispatch successful, inserted %d dispatch objects\n", result);
	return result;
}


static int
push_user(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, USER_COLLECTION);	
	printf("push user successful, inserted %d user objects\n", result);
	return result;
}
