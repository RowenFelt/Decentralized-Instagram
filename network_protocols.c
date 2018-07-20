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

  
int
parse_server_command(char *command, int fd){	
	int result;
	
	if(memcmp(command, "pull all ", 9) == 0){
		uint64_t user_id;
		user_id = strtoll((command + 9), NULL, 10); //Assuming base 10
		//checking for possible over or underflow, or no valid integer as a string was found,
		//in which case strtoll returns 0, and 0 is not a valid user id... 
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull all: ");
			return -1;
		}
		result = pull_all(user_id, fd);
	}
	
			
	else if(memcmp(command, "pull child ", 11) == 0){
		uint64_t parent_id;
		parent_id = strtoll((command + 11), NULL, 10);
		if(parent_id == LLONG_MIN || parent_id == LLONG_MAX || parent_id == 0 ){
			perror("Invalid arguments for pull child: ");
			return -1;
		}
		result = pull_child(parent_id, fd);
	}
	

	else if(memcmp(command, "pull one ", 9) == 0){
		char* endptr;
		uint64_t user_id, dispatch_id;
		user_id = strtoll((command + 9), &endptr, 10);
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull one: ");
			return -1;
		}
		dispatch_id = strtoll(endptr, NULL, 10); //ignore leading whitespace
		if(dispatch_id == LONG_MIN || dispatch_id == LLONG_MAX || dispatch_id == 0){
			perror("Invalid arguments: ");
			return -1;
		}
		result = pull_dispatch(user_id, dispatch_id, fd);
	}


	else if(memcmp(command, "pull user ", 10) == 0){
		uint64_t user_id;
		user_id = strtoll((command + 10), NULL, 10);
		printf("user_id = %ld\n", user_id);
		if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
			perror("Invalid arguments for pull user: ");
			return -1;
		}
		result = pull_user(user_id, fd);
	}


	else if(memcmp(command, "pull search tags ", 17) == 0){
		char *query, *str;
		command+= 17;
		str = strdup(command);
		if((query = strtok(str, " "))  == NULL){
			printf("Invalid argument for pull search\n");
			return -1;
		}
	
		result =	pull_tags(query, fd);
	}

	else if(memcmp(command, "pull search user_tags ", 22) == 0){
		uint64_t user_id;
		command+= 22;
		user_id = strtoll((command), NULL, 10);
    if(user_id == LLONG_MIN || user_id == LLONG_MAX || user_id == 0 ){
      perror("Invalid arguments for pull user: ");
      return -1;
    }
		result =	pull_user_tags(user_id, fd);
	}
	return result;
}


int
pull_all(uint64_t user_id, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_dispatch_by_user_audience(user_id, NULL, 0, -1, &result);
	if(bson == NULL){
		printf("PULL ALL failed, search function returned NULL\n");
		return -1;
	}
	n =	write(fd, bson, strlen(bson)); 
	if(n < 0) {
		perror("write");
		return -1;
	}
	return result;
}

int
pull_child(uint64_t parent_id, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_dispatch_by_parent_id(parent_id, -1, &result);
	if(bson == NULL){
		printf("PULL CHILD failed, search function returned NULL\n");
		return -1;
	}
	n =	write(fd, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}

int
pull_dispatch(uint64_t user_id, uint64_t dispatch_id, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_dispatch_by_id(dispatch_id, 1, &result);
	if(bson == NULL){
		printf("PULL ONE failed, search function returned NULL\n");
		return -1;
	}   	
	n = write(fd, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}

int
pull_user(uint64_t user_id, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_user_by_id_mongo(user_id, -1, &result);	
	if(bson == NULL){
		printf("PULL USER failed, search function returned NULL\n");
		return -1;
	}
	n = write(fd, bson, strlen(bson));
	if(n < 0){
		perror("write");
		return -1;
	}
	return result;
}



int
pull_user_tags(uint64_t user_id, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_dispatch_by_user_tags(user_id, -1, &result);	
	if(bson == NULL){
		printf("PULL USER TAGS failed, search function returned NULL\n");
		return -1;
	}
	n = write(fd, bson, strlen(bson));
	if(n < 0) {
		perror("write");
		return -1;
	}
	return 0;
}


int
pull_tags(const char *query, int fd){
	char *bson;
	int result = 0;
	int n = 0;
	bson = search_dispatch_by_tags(query, -1, &result);	
	if(bson == NULL){
		printf("PULL TAGS failed, search function returned NULL\n");
		return -1;
	}
	if(n < 0) {
		perror("write");
		return -1;
	}
	n = write(fd, bson, strlen(bson));
  if(n < 0) {
    perror("write");
    return -1;
  }
	return 0;
}


/* 
 * Receives a pushed json child dispatch and inserts it into the database
 */ 

int
push_child(char *json)
{
	return 0;	
}

