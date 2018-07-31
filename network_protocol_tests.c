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
	int child_json = open("network_protocol_test_calls/push_child", O_RDONLY, 0666);
	if(child_json < 0){
		perror("open");
		return -1;
  }
	int user_tag_json = open("network_protocol_test_calls/push_user_tag", O_RDONLY, 0666);
	if(user_tag_json < 0){
		perror("open");
		return -1;
	}
	int message = open("network_protocol_test_calls/push_message", O_RDONLY, 0666);
	if(message < 0){
		perror("open");
		return -1;
	}
	int push_disp = open("network_protocol_test_calls/push_dispatch", O_RDONLY, 0666);
	if(push_disp < 0){
		perror("open");
		return -1;
	}
	int push_user = open("network_protocol_test_calls/push_user", O_RDONLY, 0666);
	if(push_user < 0){
		perror("open");
		return -1;
	}

	/* test pull protocols */	
	int result = 0;
	int failed = 0;
	write(fd, "pull all***** 1234\n", 19);
	result = parse_server_command(all, fd);
	if(result != 2){
		printf("TEST FAILED: pull all*****\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull all*****\n");
	}
	write(fd, "\n", 1);
	write(fd, "pull child*** 6666\n", 19);
	result = parse_server_command(child, fd);
	if(result != 3){
		printf("TEST FAILED: pull child***\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull child***\n");
	}

	write(fd, "\n", 1);
	write(fd, "pull dispatch 1\n", 16);
	result = parse_server_command(dispatch, fd); 
	if(result != 1){
		printf("TEST FAILED: pull dispatch\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull dispatch\n");
	}
	write(fd, "\n", 1);
	write(fd, "pull user**** 9999\n", 19);
	result = parse_server_command(user, fd);
	if(result != 1){
		printf("TEST FAILED: pull user****\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull user****\n");
	}
	write(fd, "\n", 1);
	write(fd, "pull user_tag 1\n", 16);
	result = parse_server_command(user_tags, fd);
	if(result != 3){
		printf("TEST FAILED: pull user_tag\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull user_tag\n");
	}
	write(fd, "\n", 1);
	write(fd, "pull tags**** lolcats\n", 22);
	result = parse_server_command(tags, fd);
	if(result != 1){
		printf("TEST FAILED: pull tags****\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull tags****\n");
	}

	
	/* test push protocols */
	result = parse_server_command(child_json, fd);
	search_dispatch_by_id(123, -1, &result);
	if(result != 1){
		printf("TEST FAILED: push child***\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: pull child***\n");
	}
	delete_dispatch(123);	
	result = parse_server_command(user_tag_json, fd);
	search_dispatch_by_id(6689, -1, &result);
	if(result != 1){
		printf("TEST FAILED: push user_tag\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: push user_tag\n");
	}
	delete_dispatch(6689);
	result = parse_server_command(message, fd);
	search_dispatch_by_id(62938, -1, &result);
	if(result != 1){
		printf("TEST FAILED: push message*\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: push message*\n");
	}
	delete_dispatch(62938);
	result = parse_server_command(push_disp, fd);
	search_dispatch_by_id(9965, -1, &result);
	if(result != 1){
		printf("TEST FAILED: push dispatch\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: push dispatch\n");
	}
	delete_dispatch(9965);
	result = parse_server_command(push_user, fd);
	search_user_by_id_mongo(69696969, -1, &result);
	if(result != 1){
		printf("TEST FAILED: push user***\n");
		failed+=1;
	}
	else{
		printf("TEST SUCCESSFUL: push user***\n");
	}
	delete_user(69696969);
	close(fd);	
	close(all);
	close(child);
	close(dispatch);
	close(user);
	close(user_tags);
	close(tags);
	close(child_json);
	close(user_tag_json);
	close(message);
	close(push_disp);
	close(push_user);
	if(failed == 0){
		printf("All tests passed\n");
	}
	else{
		printf("%d tests failed\n", failed);
	}
	return 0;
}
