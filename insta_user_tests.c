/*
 * insta_user_tests.c
 * tests basic insta user methods using mongoDB and cassandra
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "insta_user_definitions.h"

int main(int argc, char *argv[])
{
	printf("search_user('rfelt', INSTA_FOLLOWER);\n");
	search_user("rfelt", INSTA_FOLLOWER);
	printf("search_user('Rowen Felt', INSTA_FOLLOWEE);\n");
	search_user("Rowen Felt", INSTA_FOLLOWEE);
	printf("search_user('rowen', INSTA_UNKNOWN);\n");
	search_user("rowen", INSTA_UNKNOWN);
	return 0;
}

