#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

struct thread_data {
	int csock;
};

void * sock_recv_thread(void *data) {
	struct thread_data *cdata = (struct thread_data *)data;

	size_t total, count;
	char buf[BUFSZ];

	while (1) {
		memset(buf, 0, BUFSZ);
		total = 0;

		while(1) {
			count = recv(cdata->csock, buf + total, BUFSZ - total, 0);
			total += count;

			if (count == 0 || buf[total-1] == '\n') {
				break;
			}
		}
		
		if (count == 0) {
			printf("connection was closed by server\n");
			exit(EXIT_SUCCESS);
		}

		printf("\n< ");
		puts(strtok(buf, "\n"));
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int csock;
	csock = socket(storage.ss_family, SOCK_STREAM, 0);
	if (csock == -1) {
		logexit("socket");
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(csock, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	struct thread_data *cdata = malloc(sizeof(*cdata));
	if (!cdata) {
		logexit("malloc");
	}
	cdata->csock = csock;

	pthread_t tid;
	pthread_create(&tid, NULL, sock_recv_thread, cdata);

	char buf[BUFSZ];

	while (1) {
		memset(buf, 0, BUFSZ);
		printf("> ");
		fgets(buf, BUFSZ-1, stdin);
		
		size_t count = send(csock, strtok(buf, "\0"), strlen(buf), 0);
		if (count != strlen(buf)) {
			logexit("send");
		}
	}

	close(csock);

	exit(EXIT_SUCCESS);
}