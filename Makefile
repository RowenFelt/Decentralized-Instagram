CFLAGS=-g -Wall -pedantic -pthread
MONGOFLAGS=-I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
CASSFLAGS=-L$(HOME)/instaclone/cpp-driver/build -I$(HOME)/instaclone/cpp-driver/include/ -lcassandra

.PHONY: all
all: practice cass_add_user cass_init cass_user.so

util.o: util.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^

cass_user.o: cass_user.c
	gcc $(CFLAGS) -fPIC -c -o $@ $^ $(CASSFLAGS)

cass_user.so: cass_user.o util.o
	gcc -rdynamic -shared -o $@ $^ $(CASSFLAGS)

cass_init: cass_init.c
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS)

cass_add_user: cass_add_user.c util.c
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS)

practice: practice.c cass_user.so
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOFLAGS)

.PHONY: clean
clean:
	rm -f practice cass_add_user cass_init cass_user.so cass_user.o util.o
