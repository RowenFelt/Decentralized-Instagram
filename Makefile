CFLAGS=-g -Wall -pedantic -pthread
MONGOFLAGS=-L/usr/local/lib -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -lmongoc-1.0 -lbson-1.0

CASSFLAGS=-L/home/rfelt/instaclone/cpp-driver/build -lcassandra 

.PHONY: all
all: practice cass_add_user

cass_add_user: cass_add_user.c util.c
	gcc $(CFLAGS) -o $@ $^ $(CASSFLAGS)

practice: practice.c
	gcc $(CFLAGS) -o $@ $^ $(MONGOFLAGS)

.PHONY: clean
clean:
	rm -f practice cass_add_user
