all:
	gcc -Wall -c src/common.c -o bin/common.o
	gcc -Wall -c src/message.c -o bin/message.o
	gcc -Wall src/client.c bin/common.o -lpthread -o client
	gcc -Wall src/server.c bin/common.o -lpthread -o server

clean:
	rm -f client server bin/* tests/bin/*

debug: all
	gcc -Wall -g src/server.c bin/common.o -lpthread -o server

test: all
	gcc tests/tests.c tests/unity/src/unity.c bin/message.o bin/common.o -o tests/bin/tests
	./tests/bin/tests