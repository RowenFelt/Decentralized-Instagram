CFLAGS=-g -Wall -pedantic -pthread
MONGOFLAGS=-I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0 -L/usr/bin/mongoc-stat -L/usr/bin/bsondump -lmongoc-1.0 -lbson-1.0
MONGOCOMP=`pkg-config --cflags libmongoc-1.0`
MONGOLINK=`pkg-config --libs libmongoc-1.0`
CASSFLAGS=-L$(HOME)/instaclone/cpp-driver/build -I$(HOME)/instaclone/cpp-driver/include/ -lcassandra

.PHONY: all
all: practice cass_user.so get_user_mongo insta_user_definitions.o insta_mongo_connect.o insta_user_tests

insta_user_tests: insta_user_tests.c insta_user_definitions.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOFLAGS)

insta_user_definitions.o: insta_user_definitions.c 
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(MONGOLINK) $(CASSFLAGS)

insta_user_definitions.so: insta_user_definitions.o cass_user.o insta_mongo_connect.o
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS) $(MONGOLINK) $(MONGOCOMP)  

get_user_mongo: get_user_mongo.c
	gcc $(CFLAGS) -o $@ $^ $(MONGOCOMP) $(MONGOLINK) 

insta_mongo_connect.o: insta_mongo_connect.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(MONGOCOMP) $(MONGOLINK)

util.o: util.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^

cass_user.o: cass_user.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(CASSFLAGS)

cass_user.so: cass_user.o util.o
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS)

practice: practice.c cass_user.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOFLAGS)

.PHONY: clean
clean:
	rm -f practice cass_add_user cass_get_user cass_init cass_user.so cass_user.o util.o get_user_mongo insta_mongo_connect.o insta_user_definitions.o insta_user_tests insta_user_definitions.so
