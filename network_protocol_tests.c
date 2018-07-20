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
	/* output logging file */
	int fd = open("test_output.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
	if(fd < 0){
		perror("open");
		return -1;
	}
	
	/* input files with different commands */
	int all = open("network_protocol_test_calls/pull_all", O_RDONLY, 0666);
	if(all < 0){
		perror("open");
		return -1;
	}
	int child = open("network_protocol_test_calls/pull_child", O_RDONLY, 0666);
	if(child < 0){
		perror("open");
		return -1;
	}
	int dispatch = open("network_protocol_test_calls/pull_dispatch", O_RDONLY, 0666);
	if(dispatch < 0){
		perror("open");
		return -1;
	}
	int user = open("network_protocol_test_calls/pull_user", O_RDONLY, 0666);
	if(user < 0){
		perror("open");
		return -1;
	}
	int user_tags = open("network_protocol_test_calls/pull_user_tag", O_RDONLY, 0666);
	if(user_tags < 0){
		perror("open");
		return -1;
	}
	int tags = open("network_protocol_test_calls/pull_tags", O_RDONLY, 0666);
	if(tags < 0){
		perror("open");
		return -1;
	}
	write(fd, "pull all***** 1234\n", 19);
	parse_server_command(all, fd);
	write(fd, "\n", 1);
	write(fd, "pull child*** 6666\n", 19);
	parse_server_command(child, fd);
	write(fd, "\n", 1);
	write(fd, "pull dispatch 1\n", 16);
	parse_server_command(dispatch, fd); 
	write(fd, "\n", 1);
	write(fd, "pull user**** 9999\n", 19);
	parse_server_command(user, fd);
	write(fd, "\n", 1);
	write(fd, "pull user_tag 1\n", 16);
	parse_server_command(user_tags, fd);
	write(fd, "\n", 1);
	write(fd, "pull tags**** lolcats\n", 22);
	parse_server_command(tags, fd);
	write(fd, "\n", 1);
	close(fd);	
	return 0;
}
