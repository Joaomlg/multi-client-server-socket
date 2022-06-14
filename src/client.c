#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"
#include "message.h"

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

struct thread_data {
	int csock;
};

int clients[MAX_CLIENTS - 1];
int clients_count = 0;

void server_unicast(int csock, char *buf) {
	size_t count = send(csock, strtok(buf, "\0"), strlen(buf), 0);
	if (count != strlen(buf)) {
		logexit("send");
	}
}

int read_sock(int csock, char *buf) {
	memset(buf, 0, BUFSZ);

	size_t total = 0;
	size_t count = 0;

	while(1) {
		count = recv(csock, buf + total, BUFSZ - total, 0);
		total += count;

		if (count == 0 || buf[total-1] == '\n') {
			break;
		}
	}

	if (count == 0) {
		printf("connection was closed by server\n");
		exit(EXIT_SUCCESS);
	}

	return (int) total;
}

void * sock_recv_thread(void *data) {
	struct thread_data *cdata = (struct thread_data *)data;
	
	struct message *msg = malloc(sizeof(*msg));
	char buf[BUFSZ];

	while (1) {
		int count = read_sock(cdata->csock, buf);
		printf("[msg] Received %d bytes from server: %s\n", count, strtok(buf, "\n"));

		decode_msg(buf, msg);

		switch (msg->id) {
			case RES_ADD:
				clients[clients_count++] = msg->payload[0];
				printf(clients_count == 1 ? "[log] New ID: %02d\n" : "[log] Equipment %02d added\n", msg->payload[0]);
				break;

			case RES_LIST:
				for (int i = 0; i < msg->payload_size; i++) {
					clients[clients_count++] = msg->payload[i];
					printf("[log] Equipment %02d added\n", msg->payload[i]);
				}
				break;
			
			case ERROR:
				get_error_msg_str(buf, msg);
				puts(buf);
				exit(EXIT_FAILURE);
				break;

			default:
				printf("[log] Unexpected message from server");
		}
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

	char buf[BUFSZ];
	struct message *msg = malloc(sizeof(*msg));

	build_req_add_msg(msg);
	encode_msg(buf, msg);
	server_unicast(csock, buf);

	struct thread_data *cdata = malloc(sizeof(*cdata));
	if (!cdata) {
		logexit("malloc");
	}
	cdata->csock = csock;

	pthread_t tid;
	pthread_create(&tid, NULL, sock_recv_thread, cdata);

	while (1) {
		memset(buf, 0, BUFSZ);
		printf("> ");
		fgets(buf, BUFSZ-1, stdin);
		
		server_unicast(csock, buf);
	}

	close(csock);

	exit(EXIT_SUCCESS);
}