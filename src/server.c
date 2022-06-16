#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"
#include "protocol.h"

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data {
    int id;
    int csock;
    struct sockaddr_storage storage;
};

struct client_data *clients[MAX_CLIENTS];
int clients_count = 0;
int next_client_id = 1;

int client_exists(int id) {
    for (int i=0; i<clients_count; i++) {
        if (clients[i]->id == id) {
            return 1;
        }
    }
    return 0;
}

void remove_client(int id) {
    for (int i=0; i<clients_count; i++) {
        if (clients[i]->id == id) {
            for (int j=i; j<clients_count-1; j++) {
                clients[j] = clients[j+1];
            }
            clients_count--;
            break;
        }
    }
}

void unicast(struct client_data *cdata, char *buf) {
    size_t count = send(cdata->csock, strtok(buf, "\0"), strlen(buf), 0);
    if (count != strlen(buf)) {
        logexit("send");
    }
}

void unicast_by_id(int id, char *buf) {
    for (int i=0; i<clients_count; i++) {
        if (clients[i]->id == id) {
            unicast(clients[i], buf);
            return;
        }
    }
}

void broadcast(char *buf) {
    for (int i=0; i<clients_count; i++) {
        unicast(clients[i], buf);
    }
}

void process_client_message(struct client_data *cdata, struct message *recv_msg) {
    char buf[BUFSZ];
    struct message *msg = malloc(sizeof(*msg));

    switch (recv_msg->id) {
        case REQ_ADD:
            if (clients_count == MAX_CLIENTS) {
                build_error_msg(msg, EQP_LIMIT_EXCEEDED, 0);
                encode_msg(buf, msg);
                unicast(cdata, buf);
                break;
            }

            cdata->id = next_client_id++;
            
            build_res_add_msg(msg, cdata->id);
            encode_msg(buf, msg);
            unicast(cdata, buf);
            broadcast(buf);

            if (clients_count > 0) {
                int clients_id[clients_count];
                for (int i=0; i<clients_count; i++) {
                    clients_id[i] = clients[i]->id;
                }

                build_res_list_msg(msg, clients_id, clients_count);
                encode_msg(buf, msg);
                unicast(cdata, buf);
            }

            clients[clients_count++] = cdata;
            printf("[log] Equipment %02d added\n", cdata->id);

            break;
        
        case REQ_REM:
            if (!client_exists(recv_msg->src)) {
                build_error_msg(msg, EQP_NOT_FOUND, recv_msg->src);
                encode_msg(buf, msg);
                unicast(cdata, buf);
                break;
            }

            remove_client(recv_msg->src);

            printf("[log] Equipment %02d removed\n", recv_msg->src);

            encode_msg(buf, recv_msg);
            broadcast(buf);

            build_ok_msg(msg, SUCCESSFUL_REMOVAL, recv_msg->src);
            encode_msg(buf, msg);
            unicast(cdata, buf);

            break;
        
        case REQ_INF:
        case RES_INF:
            if (!client_exists(recv_msg->src)) {
                printf("[log] Equipment %02d not found\n", recv_msg->src);
                build_error_msg(msg, SRC_EQP_NOT_FOUND, recv_msg->src);
                encode_msg(buf, msg);
                unicast(cdata, buf);
                break;
            }

            if (!client_exists(recv_msg->dst)) {
                printf("[log] Equipment %02d not found\n", recv_msg->dst);
                build_error_msg(msg, TGT_EQP_NOT_FOUND, recv_msg->src);
                encode_msg(buf, msg);
                unicast(cdata, buf);
                break;
            }

            encode_msg(buf, recv_msg);
            unicast_by_id(recv_msg->dst, buf);

            break;
        default:
            printf("[log] Invalid server msg\n");
    }
}

void * client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    char buf[BUFSZ];

    while (1) {
        memset(buf, 0, BUFSZ);
        size_t count = recv(cdata->csock, buf, BUFSZ - 1, 0);

        if (count == 0) {
            printf("[log] connection closed from %s\n", caddrstr);

            remove_client(cdata->id);
            
            printf("[log] Equipment %02d removed\n", cdata->id);
            
            struct message *msg = malloc(sizeof(*msg));
            build_req_rem_msg(msg, cdata->id);
            encode_msg(buf, msg);
            broadcast(buf);

            break;
        }

        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, strtok(buf, "\n"));

        struct message *msg = malloc(sizeof(*msg));
        decode_msg(buf, msg);

        process_client_message(cdata, msg);
    }

    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char buf[BUFSZ];

    addrtostr(addr, buf, BUFSZ);
    printf("bound to %s, waiting connections\n", buf);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        struct client_data *cdata = malloc(sizeof(*cdata));

        if (!cdata) {
            logexit("malloc");
        }

        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}
