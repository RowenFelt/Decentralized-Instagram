/*
 * network_protocols.h
 * text-based protocols for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */

#include <stdlib.h>
#include "insta_user_definitions.h"
#include "insta_dispatch_definitions.h"

int parse_client_command(char *command);
int pull_all(uint64_t user_id);
int pull_child(uint64_t parent_id);
int pull_one(uint64_t user_id, uint64_t dispatch_id);
int pull_user(uint64_t user_id);
int pull_user_tags(uint64_t user_id);
int pull_tags(const char *query);

