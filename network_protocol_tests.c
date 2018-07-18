/* 
 * network_protocol_tests.c
 * A test suite for the network protocol.so library
 */

#include <mongoc.h>
#include <bson.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "network_protocols.h"

int main(int argc, char *argv[])
{
	printf("pull all 1234\n");
	parse_client_command("pull all 1234");
	printf("pull child 6666\n");
	parse_client_command("pull child 6666");
	printf("pull one 3 1\n");
	parse_client_command("pull one 3 1"); 
	printf("pull user 159178\n");
	parse_client_command("pull user 159178");
	printf("pull search user_tags 1\n");
	parse_client_command("pull search user_tags 1");
	printf("pull search tags lolcats\n");
	parse_client_command("pull search tags lolcats");
	
	return 0;
}
