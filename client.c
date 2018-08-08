/*
 * client.c
 * Authors: Rowen Felt and Dylan Mortimer
 */

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

#include "user_definitions.h" 
#include "mongo_connect.h"
#include "cass_user.h"
#include "network_protocols.h"

#define BUF_SIZE 16000
#define PUSH_PROTOCOL "push"
#define PUSH_SIZE 4
#define USER_PROTOCOL "user*"

int push_local(char *command, char *port, int length);

int main(int argc, char *argv[])
{
  char *dest_user, *dest_port, *dest_hostname, *input_file;
	char type[4];
  struct addrinfo hints, *res;
  int conn_fd;
  char buf[BUF_SIZE];
  int rc;
  int n;
	int input_fd;

	/* dest_user is user_id of user */
  dest_user = argv[1];
  dest_port = argv[2];
	input_file = argv[3];

	input_fd = open(input_file, O_RDONLY);
	if(input_fd == -1){
		perror("client open");
		return -1;
	}
	
	int user_id = atoi(dest_user);
	dest_hostname = get_user_ip_by_id(INSTA_DB, USER_COLLECTION, user_id);	

  /* create a socket */
  conn_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(conn_fd == -1){
    perror("create socket failed");
  }

  /* but we do need to find the IP address of the server */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if((rc = getaddrinfo(dest_hostname, dest_port, &hints, &res)) != 0) {
      printf("getaddrinfo failed: %s\n", gai_strerror(rc));
      return -1;
  }

  /* connect to the server */
  if(connect(conn_fd, res->ai_addr, res->ai_addrlen) < 0) {
      perror("connect to server failed");
      return -1;
  }
	
	if((n = read(input_fd, type, PUSH_SIZE)) < 0){
		perror("read command type");
		return -1;
	}
	if((n = lseek(input_fd, 0, SEEK_SET)) < 0){
		perror("lseek input_file");
		return -1;
	}	

	/* read command from input_fd and send to server */
	n = read(input_fd, buf, BUF_SIZE);
	send(conn_fd, buf, n, 0);
	if(memcmp(type, PUSH_PROTOCOL, 4) == 0){
		close(conn_fd);
	}
	else{
		/* read response from server */
		memset(buf, '\0', BUF_SIZE);
		int bytes_read = read(conn_fd, buf, BUF_SIZE);
		
		/* determine to which collection to push */ 
		n = lseek(input_fd, PUSH_SIZE+1, SEEK_SET);	
		char temp[5];
		memset(temp, '\0', 5);
		read(input_fd, temp, 5);
		char command[BUF_SIZE + INSTA_PROTOCOL_SIZE];
		memset(command, '\0', BUF_SIZE + INSTA_PROTOCOL_SIZE);

		if(memcmp(temp, USER_PROTOCOL, 5) == 0){
			strncat(command, "push user**** ", INSTA_PROTOCOL_SIZE);
		}
		else{
			strncat(command, "push dispatch ", INSTA_PROTOCOL_SIZE);	
		} 
		strncat(command, buf, bytes_read);  	
		close(conn_fd);
		push_local(command, dest_port, INSTA_PROTOCOL_SIZE + bytes_read);
	}
}

int push_local(char *command, char *dest_port, int length)
{
	struct addrinfo hints, *res;
  int conn_fd, rc;
	
/* create a socket */
  conn_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(conn_fd == -1){
    perror("create socket failed");
  }

  /* use localhost for IP */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if((rc = getaddrinfo("127.0.0.1", dest_port, &hints, &res)) != 0) {
      printf("getaddrinfo failed: %s\n", gai_strerror(rc));
      return -1;
  }

  /* connect to the server */
  if(connect(conn_fd, res->ai_addr, res->ai_addrlen) < 0) {
      perror("connect to server failed");
      return -1;
  }
		
	send(conn_fd, command, length, 0);
	close(conn_fd);
	return 0;
}
