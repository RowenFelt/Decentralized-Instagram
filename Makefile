CFLAGS=-g -Wall -pedantic -pthread
MONGOCOMP=`pkg-config --cflags libmongoc-1.0`
MONGOLINK=`pkg-config --libs libmongoc-1.0`
CASSFLAGS=-L$(HOME)/instaclone/cpp-driver/build -I$(HOME)/instaclone/cpp-driver/include/ -lcassandra

.PHONY: all

all: practice cass_user.so get_user_mongo user_definitions.o user_definitions.so dispatch_definitions.o dispatch_definitions.so mongo_connect.o user_tests dispatch_tests tcp_server client network_protocols.o network_protocols.so network_protocol_tests util.o

network_protocol_tests: network_protocol_tests.c network_protocols.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOLINK) $(MONGOCOMP)

network_protocols.o: network_protocols.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(CASSFLAGS)

network_protocols.so: network_protocols.o user_definitions.o dispatch_definitions.o mongo_connect.o cass_user.o 
	gcc -rdynamic -shared -o $@ $^ $(MONGOLINK) $(MONGOCOMP) $(CASSFLAGS)

client: client.c user_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(CASSFLAGS)

tcp_server: tcp_server.c
	gcc $(CFLAGS) -o $@ $^ 

user_tests: user_tests.c user_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK)

user_definitions.o: user_definitions.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(CASSFLAGS)

user_definitions.so: user_definitions.o dispatch_definitions.o cass_user.o mongo_connect.o 
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS) $(MONGOLINK) $(MONGOCOMP)  

dispatch_tests: dispatch_tests.c dispatch_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK)

dispatch_definitions.o: dispatch_definitions.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) 

dispatch_definitions.so: dispatch_definitions.o user_definitions.o mongo_connect.o  cass_user.o 
	gcc -rdynamic -shared -o $@ $^ $(MONGOCOMP) $(MONGOLINK) 

get_user_mongo: get_user_mongo.c
	gcc $(CFLAGS) -o $@ $^ $(MONGOCOMP) $(MONGOLINK) 

mongo_connect.o: mongo_connect.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(MONGOLINK)

util.o: util.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ 

cass_user.o: cass_user.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(CASSFLAGS) $(MONGOCOMP)

cass_user.so: cass_user.o
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS)

practice: practice.c cass_user.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOCOMP) $(MONGOLINK)

.PHONY: clean
clean:
	rm -f practice cass_add_user cass_get_user cass_init cass_user.so cass_user.o util.o get_user_mongo mongo_connect.o user_definitions.o user_tests user_definitions.so dispatch_definitions.o dispatch_definitions.so dispatch_tests tcp_server client network_protocols.o network_protocols.so network_protocol_tests util.o
