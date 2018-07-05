CFLAGS=-g -Wall -pedantic -pthread
MONGOFLAGS=-I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0

CASSFLAGS=-L$(HOME)/instaclone/cpp-driver/build -I$(HOME)/instaclone/cpp-driver/include/ -lcassandra 

.PHONY: all
all: practice cass_add_user

cass_add_user: cass_add_user.c util.c
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS)

practice: practice.c
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS) $(MONGOFLAGS)

.PHONY: clean
clean:
	rm -f practice cass_add_user
