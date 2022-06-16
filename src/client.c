#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

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

int id() {
	return clients[0];
}

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
	char recv_buf[BUFSZ];
	char buf[BUFSZ];

	while (1) {
		read_sock(cdata->csock, recv_buf);

		char *ptr = strtok(recv_buf, MSG_END);
		while (ptr != NULL) {
			printf("[msg] Received %d bytes from server: %s\n", (int) strlen(ptr), ptr);

			decode_msg(ptr, msg);

			switch (msg->id) {
				case RES_ADD:
					clients[clients_count++] = msg->payload[0];
					
					const char *str = (clients_count == 1) ? "[log] New ID: %02d\n" : "[log] Equipment %02d added\n";
					printf(str, msg->payload[0]);
					
					break;

				case RES_LIST:
					for (int i = 0; i < msg->payload_size; i++) {
						clients[clients_count++] = msg->payload[i];
						printf("[log] Equipment %02d added\n", msg->payload[i]);
					}

					break;
				
				case REQ_REM:
					for (int i=0; i<clients_count; i++) {
						if (clients[i] == msg->src) {
							for (int j=i; j<clients_count-1; j++) {
								clients[j] = clients[j+1];
							}

							printf("Equipment %02d removed\n", msg->src);

							clients_count--;
							
							break;
						}
					}

					break;
				
				case REQ_INF:
					printf("[log] Requested information\n");

					float data = (float) (rand() % 999) / 100;

					build_res_inf_msg(msg, id(), msg->src, data);
					encode_msg(buf, msg);

					server_unicast(cdata->csock, buf);

					break;
				
				case RES_INF:
					printf("[log] Value from %02d: %.2f\n", msg->src, decode_msg_data(msg));
					break;
				
				case OK:
					get_ok_msg_str(buf, msg);
					printf("[log] %s\n", buf);

					if (msg->payload[0] == SUCCESSFUL_REMOVAL) {
						close(cdata->csock);
						exit(EXIT_SUCCESS);
					}

					break;
				
				case ERROR:
					get_error_msg_str(buf, msg);
					printf("[log] %s\n", buf);

					if (msg->payload[0] == EQP_LIMIT_EXCEEDED) {
						close(cdata->csock);
						exit(EXIT_FAILURE);
					}

					break;

				default:
					printf("[log] Unexpected message from server\n");
			}

			ptr = strtok(NULL, MSG_END);
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	srand(time(0));

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
		fgets(buf, BUFSZ-1, stdin);

		const static char *close_connection_cmd = "close connection\n";
		if (strcmp(buf, close_connection_cmd) == 0) {
			build_req_rem_msg(msg, id());
			encode_msg(buf, msg);
			server_unicast(csock, buf);
			continue;
		}
		
		const static char *list_equipment_cmd = "list equipment\n";
		if (strcmp(buf, list_equipment_cmd) == 0) {
			for (int i=1; i<clients_count; i++) {
				printf("%02d", clients[i]);
				printf(i == clients_count - 1 ? "\n" : " ");
			}
			continue;
		}
		
		const static char *request_information_cmd = "request information from ";
		const int cmd_size = strlen(request_information_cmd);
		if (strncmp(buf, request_information_cmd, cmd_size) == 0) {
			char dst_id[ID_BYTE_SIZE + 1];
			strncpy(dst_id, buf + cmd_size, ID_BYTE_SIZE);
			
			build_req_inf_msg(msg, id(), atoi(dst_id));
			encode_msg(buf, msg);
			server_unicast(csock, buf);

			continue;
		}

		printf("[log] Invalid client command\n");
	}

	close(csock);

	exit(EXIT_SUCCESS);
}