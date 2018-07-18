/*
 * insta-client.c
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

#include "insta_user_definitions.h" 

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    char *dest_user, *dest_port, *dest_hostname;
    struct addrinfo hints, *res;
    int conn_fd;
    char buf[BUF_SIZE];
    int rc;
    int n;

		/* dest_user is user_id of user */
    dest_user = argv[1];
    dest_port = argv[2];

		int user_id = atoi(dest_user);
		dest_hostname = search_user_by_id_cass(user_id);	

    /* create a socket */
    conn_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(conn_fd == -1){
      perror("socket");
    }

    /* but we do need to find the IP address of the server */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if((rc = getaddrinfo(dest_hostname, dest_port, &hints, &res)) != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(rc));
        exit(1);
    }

    /* connect to the server */
    if(connect(conn_fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        exit(2);
    }

    printf("Connected\n");

    /* infinite loop of reading from terminal, sending the data, and printing
     * what we get back */

    while((n = read(0, buf, BUF_SIZE)) > 0) {
      if(buf[0] == EOF){
        printf("Exiting\n");
        //exit(1);
      }
        send(conn_fd, buf, n, 0);
    }
}

