/*
 * network_protocols.h
 * text-based protocols for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include "insta_user_definitions.h"
#include "insta_dispatch_definitions.h"

int parse_server_command(char *command, int fd);

int pull_all(uint64_t user_id, int fd);
int pull_child(uint64_t parent_id, int fd);
int pull_dispatch(uint64_t user_id, uint64_t dispatch_id, int fd);
int pull_user(uint64_t user_id, int fd);
int pull_user_tags(uint64_t user_id, int fd);
int pull_tags(const char *query, int fd);

int push_child(char *json);
int push_user_tag(char *json);
int push_message(char *json);
int push_dispatch(char *json);
int push_user(char *json);
