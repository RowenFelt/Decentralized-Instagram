SHELL=/bin/bash
CFLAGS=-g -Wall -pedantic -pthread 
MONGOCOMP=`pkg-config --cflags libmongoc-1.0`
MONGOLINK=`pkg-config --libs libmongoc-1.0`
CASSFLAGS=-L$(HOME)/instaclone/cpp-driver/build -I$(HOME)/instaclone/cpp-driver/include/ -lcassandra


RESET          = \033[0m
make_std_color = \033[3$1m      # defined for 1 through 7
make_color     = \033[38;5;$1m  # defined for 1 through 255
WRN_COLOR = $(strip $(call make_color,81))
ERR_COLOR = $(strip $(call make_color,202))
STD_COLOR = $(strip $(call make_color,116))

COLOR_OUTPUT = 2>&1 |                                   \
    while IFS='' read -r line; do                       \
        if  [[ $$line == *:[\ ]error:* ]]; then         \
            echo -e "$(ERR_COLOR)$${line}$(RESET)";     \
        elif [[ $$line == *:[\ ]warning:* ]]; then      \
            echo -e "$(WRN_COLOR)$${line}$(RESET)";     \
        else                                            \
            echo -e "$(STD_COLOR)$${line}$(RESET)";     \
        fi;                                             \
    done; exit $${PIPESTATUS[0]};

.PHONY: all

all: cass_user_tests cass_user.so get_user_mongo user_definitions.o user_definitions.so dispatch_definitions.o dispatch_definitions.so mongo_connect.o user_tests dispatch_tests tcp_server client network_protocols.o network_protocols.so network_protocol_tests util.o

network_protocol_tests: network_protocol_tests.c network_protocols.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOLINK) $(MONGOCOMP) $(COLOR_OUTPUT)

network_protocols.o: network_protocols.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(CASSFLAGS) $(COLOR_OUTPUT)

network_protocols.so: network_protocols.o user_definitions.o dispatch_definitions.o mongo_connect.o cass_user.o 
	gcc -rdynamic -shared -o $@ $^ $(MONGOLINK) $(MONGOCOMP) $(CASSFLAGS) $(COLOR_OUTPUT) 

client: client.c user_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(CASSFLAGS) $(COLOR_OUTPUT)

tcp_server: tcp_server.c
	gcc $(CFLAGS) -o $@ $^ $(COLOR_OUTPUT)

user_tests: user_tests.c user_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

user_definitions.o: user_definitions.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(CASSFLAGS) $(COLOR_OUTPUT)

user_definitions.so: user_definitions.o dispatch_definitions.o cass_user.o mongo_connect.o 
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS) $(MONGOLINK) $(MONGOCOMP)  $(COLOR_OUTPUT)

dispatch_tests: dispatch_tests.c dispatch_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

dispatch_definitions.o: dispatch_definitions.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(COLOR_OUTPUT)

dispatch_definitions.so: dispatch_definitions.o user_definitions.o mongo_connect.o  cass_user.o 
	gcc -rdynamic -shared -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

get_user_mongo: get_user_mongo.c
	gcc $(CFLAGS) -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

mongo_connect.o: mongo_connect.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

util.o: util.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(COLOR_OUTPUT)

cass_user.o: cass_user.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(COLOR_OUTPUT)

cass_user.so: cass_user.o
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS) $(COLOR_OUTPUT)

cass_user_tests: cass_user_tests.c cass_user.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK) $(COLOR_OUTPUT)

.PHONY: clean
clean:
	rm -f cass_user_tests cass_add_user cass_get_user cass_init cass_user.so cass_user.o util.o get_user_mongo mongo_connect.o user_definitions.o user_tests user_definitions.so dispatch_definitions.o dispatch_definitions.so dispatch_tests tcp_server client network_protocols.o network_protocols.so network_protocol_tests util.o
