CFLAGS=-g -Wall -pedantic -pthread
MONGOFLAGS=-L/usr/local/lib -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -lmongoc-1.0 -lbson-1.0

.PHONY: all
all: practice cass_add_user

cass_add_user: cass_add_user.c
	gcc $(CFLAGS) -o $@ $^

practice: practice.c
	gcc $(CFLAGS) -o $@ $^ $(MONGOFLAGS)


.PHONY: clean
clean:
	rm -f practice
