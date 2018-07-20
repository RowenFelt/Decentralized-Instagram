/*
 * network_protocols.h
 * text-based protocols for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include "insta_user_definitions.h"
#include "insta_dispatch_definitions.h"

int parse_server_command(int in, int out);

int pull_all(int in, int out);
int pull_child(int in, int out);
int pull_dispatch(int in, int out);
int pull_user(int in, int out);
int pull_user_tags(int in, int out);
int pull_tags(int in, int out);

int push_child(char *json);
int push_user_tag(char *json);
int push_message(char *json);
int push_dispatch(char *json);
int push_user(char *json);
