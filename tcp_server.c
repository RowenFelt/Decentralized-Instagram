/*
 * tcp_server.c
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include "network_protocols.h"

#define BACKLOG 10
#define BUF_SIZE 10000

struct connect_info{
  uint16_t remote_port;
  char *remote_ip;
  int conn_fd;
};


int main(int argc, char *argv[])
{
	char *listen_port;
	int listen_fd, conn_fd;
  struct addrinfo hints, *res;
  int rc;
  socklen_t addrlen;
  uint16_t remote_port;
  struct sockaddr_in remote_sa;
  char *remote_ip;
  struct connect_info * info;
	int n = 0;

  listen_port = argv[1];

	/* create a socket */
  listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(listen_fd == -1){
    perror("socket");
	}

	/* bind it to a port */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if((rc = getaddrinfo(NULL, listen_port, &hints, &res)) != 0) {
      printf("getaddrinfo failed: %s\n", gai_strerror(rc));
      exit(1);
  }

  bind(listen_fd, res->ai_addr, res->ai_addrlen);
  if(listen_fd == -1){
    perror("bind");
  }

  /* start listening */
  listen(listen_fd, BACKLOG);
  if(listen_fd == -1){
    perror("listen");
	}

	/* infinite loop of accepting new connections and handling them */
  while(1) {
    /* accept a new connection (will block until one appears) */
    addrlen = sizeof(remote_sa);
    conn_fd = accept(listen_fd, (struct sockaddr *) &remote_sa, &addrlen);
    remote_ip = inet_ntoa(remote_sa.sin_addr);
    remote_port = ntohs(remote_sa.sin_port);
    printf("new connection from %s:%d\n", remote_ip, remote_port);
    info = malloc(sizeof(struct connect_info));
    info->remote_port = remote_port;
    info->remote_ip = remote_ip;
    info->conn_fd = conn_fd;
		n = parse_server_command(info->conn_fd, info->conn_fd);
		close(info->conn_fd);
		free(info);
	}
	return n;	
}
