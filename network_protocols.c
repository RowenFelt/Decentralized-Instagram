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
#include "insta_user_definitions.h"
#include "insta_dispatch_definitions.h"
#include "insta_mongo_connect.h"
#include "util.h"

static uint64_t read_id(int fd);
  
int
parse_server_command(int in, int out){	
	int result;
	int n = 0;
	char *command = malloc(sizeof(char) * INSTA_PROTOCOL_SIZE);	
	n = read(in, command, INSTA_PROTOCOL_SIZE);	
	if(n < 0){
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

	int (*protocol_func)(int, int) = NULL;
	for(int i = 0; i < INSTA_PROTOCOL_NUM; i++){
		if(memcmp(command, protocol_list[i].protocol, 
								INSTA_PROTOCOL_SIZE) == 0){
			protocol_func = protocol_list[i].func;
			break;
		}
	}

	result = protocol_func(in, out);
	
	free(command);
	return result;
}


static uint64_t 
read_id(int fd)
{
	char str[20];
	uint64_t id;

	memset(str, '\0', 20);
	read(fd, str, 20);
	id = strtoll((str), NULL, 10);
	if(id == LLONG_MIN || id == LLONG_MAX || id == 0 ){
		perror("Invalid arguments for pull user: ");
		return -1;
  }
	return id;
}


int
pull_all(int in, int out){
	char *bson;
	uint64_t user_id;
	int result = 0;
	int n = 0;

	user_id = read_id(in);
	
	bson = search_dispatch_by_user_audience(user_id, NULL, 0, -1, &result);
	if(bson == NULL){
		printf("PULL ALL failed, search function returned NULL\n");
		return -1;
	}
	n =	write(out, bson, strlen(bson)); 
	if(n < 0) {
		perror("write");
		return -1;
	}
	return result;
}

int
pull_child(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t parent_id;
	
	parent_id = read_id(in);

	bson = search_dispatch_by_parent_id(parent_id, -1, &result);
	if(bson == NULL){
		printf("PULL CHILD failed, search function returned NULL\n");
		return -1;
	}
	n =	write(out, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}

int
pull_dispatch(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t dispatch_id;

	dispatch_id = read_id(in);
		
	bson = search_dispatch_by_id(dispatch_id, 1, &result);
	if(bson == NULL){
		printf("PULL ONE failed, search function returned NULL\n");
		return -1;
	}   	
	n = write(out, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}

int
pull_user(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t user_id;

	user_id = read_id(in);
	
	bson = search_user_by_id_mongo(user_id, -1, &result);	
	if(bson == NULL){
		printf("PULL USER failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}



int
pull_user_tags(int in, int out){
	char *bson;
	int result = 0;
	int n = 0;
	uint64_t user_id;

	user_id = read_id(in);
	
	bson = search_dispatch_by_user_tags(user_id, -1, &result);	
	if(bson == NULL){
		printf("PULL USER TAGS failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, strlen(bson));
	if(n < 0) {
		perror("write");
		return -1;
	}
	return result;
}


int
pull_tags(int in, int out){
	char *bson, *query;
	int result = 0;
	int n = 0;
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
	bson = search_dispatch_by_tags(query, -1, &result);	
	if(bson == NULL){
		printf("PULL TAGS failed, search function returned NULL\n");
		return -1;
	}
	n = write(out, bson, strlen(bson));
  if(n < 0) {
    perror("write");
    return -1;
  }
	return result;
}


/* 
 * Receives a pushed json child dispatch and inserts it into the database
 */ 

int
push_child(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	return result;
}


/* 
 * Receives a pushed dispatch with user_tags and inserts it into the database
 */ 
int
push_user_tag(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	return result;
}


/* 
 * Receives a pushed direct message dispatch and inserts it into the database
 */ 
int
push_message(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	return result;
}


/* 
 * Receives a pushed dispatch and inserts it into the database
 */ 
int
push_dispatch(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, DISPATCH_COLLECTION);	
	return result;
}


/* 
 * Receives a pushed user object and inserts it into the database
 */ 
int
push_user(int fd, int out)
{
	int result = 0;
	result = insert_json_from_fd(fd, USER_COLLECTION);	
	return result;
}
