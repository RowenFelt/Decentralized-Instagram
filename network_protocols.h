/*
 * network_protocols.h
 * text-based protocols for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include "user_definitions.h"
#include "dispatch_definitions.h"

#define INSTA_PROTOCOL_SIZE 14
#define INSTA_PROTOCOL_NUM 11
#define LARGE_ID_SIZE 20

struct insta_protocol {
	char protocol[INSTA_PROTOCOL_SIZE];
	int (*func)(int, int);
};

int parse_server_command(int in, int out);

int pull_all(int in, int out);
int pull_child(int in, int out);
int pull_dispatch(int in, int out);
int pull_user(int in, int out);
int pull_user_tags(int in, int out);
int pull_tags(int in, int out);

int push_child(int fd, int out);
int push_user_tag(int fd, int out);
int push_message(int fd, int out);
int push_dispatch(int fd, int out);
int push_user(int fd, int out);
