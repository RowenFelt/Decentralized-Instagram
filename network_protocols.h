/*
 * network_protocols.h
 * text-based protocols for server based insta functions
 * Authors: Rowen Felt and Campbell Boswell
 */
#ifndef _NETWORK_PROTOCOLS_H
#define _NETWORK_PROTOCOLS_H


#include <stdlib.h>
#include "user_definitions.h"
#include "dispatch_definitions.h"

#define INSTA_PROTOCOL_SIZE 14

int parse_server_command(int in, int out);

#endif /* _NETWORK_PROTOCOLS_H */
