/* 
 * network_protocol_tests.c
 * A test suite for the network protocol.so library
 */

#include <fcntl.h>
#include <mongoc.h>
#include <bson.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "network_protocols.h"

int main(int argc, char *argv[])
{
	int fd = open("test_output.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
	if(fd < 0){
		perror("open");
		return -1;
	}
	write(fd, "pull all 1234\n", 14);
	parse_server_command("pull all 1234", fd);
	write(fd, "\n", 1);
	write(fd, "pull child 6666\n", 16);
	parse_server_command("pull child 6666", fd);
	write(fd, "\n", 1);
	write(fd, "pull one 3 1\n", 13);
	parse_server_command("pull one 3 1", fd); 
	write(fd, "\n", 1);
	write(fd, "pull user 9999\n", 17);
	parse_server_command("pull user 9999", fd);
	write(fd, "\n", 1);
	write(fd, "pull search user_tags 1\n", 24);
	parse_server_command("pull search user_tags 1", fd);
	write(fd, "\n", 1);
	write(fd, "pull search tags lolcats\n", 25);
	parse_server_command("pull search tags lolcats", fd);
	write(fd, "\n", 1);
	close(fd);	
	return 0;
}
