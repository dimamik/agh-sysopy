#ifndef LAB_10_COMMON_H
#define LAB_10_COMMON_H

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define MAXIMUM_NUMBER_OF_CLIENTS 6
#define BACKLOG_MAX 20
#define MAX_MESSAGE_LENGTH 50
#define MAX_NICK_LENGTH 20
#define BOARD_SIZE 9
typedef struct {
    char nick[MAX_NICK_LENGTH];
    int client_fd;
    int is_client_present;
    int ping_pong;
    int am_i_cross;
} client_t;

typedef struct{
    char board[BOARD_SIZE];
}game_t;

int compare(char *first, char *second) {
    return strcmp(first, second) == 0;
}

void my_send(int socket_id, char *buf) {
    printf("\n>> Sending to %d message: %s\n", socket_id, buf);
    send(socket_id, buf, MAX_MESSAGE_LENGTH, 0);
}

void my_receive(int socket_id, char *buf) {
    recv(socket_id, buf, MAX_MESSAGE_LENGTH, 0);
    printf("\n>> Received from %d message: %s \n", socket_id, buf);
}

#endif //LAB_10_COMMON_H
