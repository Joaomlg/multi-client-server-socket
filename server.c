#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define MAX_CLIENTS 15

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
            break;
        }

        if (strcmp(buf, "kill\n") == 0) {
            printf("[log] server killed\n");
            exit(EXIT_SUCCESS);
        }

        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, strtok(buf, "\n"));

        memset(buf, 0, BUFSZ);

        count = send(cdata->csock, strtok(buf, "\0"), strlen(buf), 0);
        if (count != strlen(buf)) {
            logexit("send");
        }
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
    size_t count;

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

        cdata->id = next_client_id++;
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);

        printf("[log] Equipment %i added\n", cdata->id);

        sprintf(buf, "New ID: %i\n", cdata->id);
        count = send(cdata->csock, strtok(buf, "\0"), strlen(buf), 0);
        if (count != strlen(buf)) {
            logexit("send");
        }

        for (int i=0; i<clients_count; i++) {
            sprintf(buf, "Equipment %i added\n", cdata->id);
            count = send(clients[i]->csock, strtok(buf, "\0"), strlen(buf), 0);
            if (count != strlen(buf)) {
                logexit("send");
            }
        }

        clients[clients_count++] = cdata;
    }

    exit(EXIT_SUCCESS);
}
