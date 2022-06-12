all:
	gcc -Wall -c common.c
	gcc -Wall -c message.c 
	gcc -Wall client.c common.o -lpthread -o client
	gcc -Wall server.c common.o -lpthread -o server

clean:
	rm common.o message.o client server

debug: all
	gcc -Wall -g server.c common.o -lpthread -o server

test: all
	gcc tests/main.test.c tests/unity/src/unity.c message.o common.o -o tests/bin/tests
	./tests/bin/tests