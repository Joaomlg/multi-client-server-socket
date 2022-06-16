all:
	gcc -Wall -c src/common.c -o bin/common.o
	gcc -Wall -c src/protocol.c -o bin/protocol.o
	gcc -Wall src/client.c bin/common.o bin/protocol.o -lpthread -o client
	gcc -Wall src/server.c bin/common.o bin/protocol.o -lpthread -o server

clean:
	rm -f client server bin/* tests/bin/*

debug:
	gcc -Wall -c src/common.c -o bin/common.o
	gcc -Wall -g -c src/protocol.c -o bin/protocol.o
	gcc -Wall -g src/server.c bin/common.o bin/protocol.o -lpthread -o server
	gcc -Wall -g src/client.c bin/common.o bin/protocol.o -lpthread -o client

test: all
	gcc tests/tests.c tests/unity/src/unity.c bin/protocol.o bin/common.o -o tests/bin/tests
	./tests/bin/tests