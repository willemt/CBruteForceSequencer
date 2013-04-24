CONTRIB_DIR = ..
HASHMAP_DIR = $(CONTRIB_DIR)/CHashMapViaLinkedList
#FIXEDARRAY_DIR = $(CONTRIB_DIR)/CFixedArrayList
ARRAYQUEUE_DIR = $(CONTRIB_DIR)/CArrayQueue

GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
SHELL  = /bin/bash
CC     = gcc
CCFLAGS = -g -O2 -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS) -I$(HASHMAP_DIR) -I$(ARRAYQUEUE_DIR)

all: tests

chashmap:
	mkdir -p $(HASHMAP_DIR)/.git
	git --git-dir=$(HASHMAP_DIR)/.git init 
	pushd $(HASHMAP_DIR); git pull git@github.com:willemt/CHashMapViaLinkedList.git; popd

carrayqueue:
	mkdir -p $(ARRAYQUEUE_DIR)/.git
	git --git-dir=$(ARRAYQUEUE_DIR)/.git init 
	pushd $(ARRAYQUEUE_DIR); git pull git@github.com:willemt/CArrayQueue.git; popd

download-contrib: chashmap carrayqueue

main.c:
	if test -d $(HASHMAP_DIR); \
	then echo have contribs; \
	else make download-contrib; \
	fi
	sh make-tests.sh > main.c

tests: main.c bruteforcesequencer.c test_bruteforcesequencer.c CuTest.c main.c $(HASHMAP_DIR)/linked_list_hashmap.c $(ARRAYQUEUE_DIR)/arrayqueue.c 
	$(CC) $(CCFLAGS) -o $@ $^
	./tests
	gcov main.c test_bruteforcesequencer.c bruteforcesequencer.c

bruteforcesequencer.o: bruteforcesequencer.c 
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f main.c bruteforcesequencer.o tests $(GCOV_OUTPUT)
