/*
  * search_user.c
  * Searches for a username in cassandra, pulls user
  * objects from all returned user_ids
  * Authors: Rowen Felt and Campbell Boswell
  */
 
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/random.h>

#include "user_definitions.h"
#include "cass_user.h"
#include "network_protocols.h"

#define FILE_NAME_SIZE 100
#define RANDOM_INT_SIZE 50
#define BASE_FILE_NAME 19

int 
main(int argc, char *argv[])
{
	char *username;
	uint64_t *user_ids;
	int result, fd, n;
	int pid, status;

	username = argv[1]; 
	int buffer_size = 30;
	char file[FILE_NAME_SIZE] ;
	char *port = "3999";
	char *client = "./client";
	int random[1];
	char random_ints[RANDOM_INT_SIZE];

	memset(file, '\0', FILE_NAME_SIZE);
	memcpy(file, "search_user_command", BASE_FILE_NAME);	
	memset(random_ints, '\0', RANDOM_INT_SIZE);
	getrandom(random, sizeof(int), GRND_RANDOM);
	n = sprintf(random_ints, "%d", random[0]); 
	strncat(file, random_ints, n);
	
	user_ids = get_user_id_by_username(KEYSPACE, TABLE, username, &result);

	for(int i = 0; i < result; i++){
		char id[buffer_size];
		char command[buffer_size];
		memset(command, '\0', buffer_size);
		sprintf(id, "%ld", user_ids[i]);
	
		memcpy(command, "pull user**** ", INSTA_PROTOCOL_SIZE);
		int len = strlen(id);
		strncat(command, id, len); 
		
		fd = open(file, O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
		if(fd < 0){
			perror("open in search_user");
			return -1;
		}
		n = write(fd, command, strlen(command)); 
		if(n < 0){
			perror("write in search_user");
			return -1;
		}
		close(fd);
		if((pid = fork()) < 0){
			perror("search_user - fork error:");
			return -1;
		}	
	  if(pid == 0){
			execlp(client, client, id, port, file, (char *) NULL);
			printf("this is a child\n");
	    exit(1);
	  }
	  else{
	    if(waitpid(pid, &status, 0) < -1){
	      printf("view_media_from_binary - waitpid error;\n");
	      return -1;
	    }
	  }
	}

	if(result > 0){	
		execlp("rm", "rm", file, (char *) NULL);
	}
}

