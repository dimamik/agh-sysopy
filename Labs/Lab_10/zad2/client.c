#include <signal.h>
#include "common.h"

#define WAIT 0
#define ACTIVE 1
int server_socket;
int is_moving_first;
char buf[MAX_MESSAGE_LENGTH + 1];
char *nick;
pthread_cond_t input_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t reply_mutex = PTHREAD_MUTEX_INITIALIZER;
char board[BOARD_SIZE];
int am_moving_cross = 1;
int current_state = WAIT;
char *server_name;
int is_local = 0;
int client_socket;
struct sockaddr_un local_socket_address;

void set_up_local_socket(char *local_socket_path) {
    server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    memset(&local_socket_address, 0, sizeof(struct sockaddr_un));
    local_socket_address.sun_family = AF_UNIX;
    strcpy(local_socket_address.sun_path, local_socket_path);

    if (connect(server_socket, (struct sockaddr *) &local_socket_address,
                sizeof(struct sockaddr_un))) {
        perror("Error while connecting\n");
        exit(-1);
    }
//    Set up client socket
    client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un client_socket_address;
    memset(&client_socket_address, 0, sizeof(struct sockaddr_un));
    client_socket_address.sun_family = AF_UNIX;
    sprintf(client_socket_address.sun_path, "/tmp/%d", getpid());

    if (bind(client_socket, (struct sockaddr *) &client_socket_address,
             sizeof(struct sockaddr_un))) {
        perror("Error in binding client socket");
        exit(-1);
    }


}

void set_up_network_socket(char *port) {
    struct addrinfo *info;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo("127.0.0.1", port, &hints, &info)) {
        perror("Error in getaddrinfo: ");
        exit(-1);
    }

    server_socket =
            socket(info->ai_family, info->ai_socktype, info->ai_protocol);

    if (connect(server_socket, info->ai_addr, info->ai_addrlen)) {
        perror("Error while connecting\n");
        exit(-1);
    }

    freeaddrinfo(info);
}

void register_in_server() {

    sprintf(buf, "%s|register", nick);

    printf("%d", is_local);
    if (is_local) {
        sendto(client_socket,
               buf, MAX_MESSAGE_LENGTH, 0,
               (struct sockaddr *) &local_socket_address, sizeof(struct sockaddr_un));
    } else {
        my_send_client(server_socket, buf);
    }


    printf("Sent message to server\n");
    if (is_local) {
        my_receive(client_socket, buf);

    } else {
        my_receive(server_socket, buf);
    }

    if (compare(buf, "ok")) {
        printf("Successfully registered on server!\n");
        return;
    } else if (compare(buf, "occupied")) {
        printf("Nick is occupied, please exit and try another\n");
        exit(0);
    } else {
        printf("Sorry, server is full, please wait\n");
        exit(0);
    }
}


void print_board(char *board_from_server) {
    printf("\n");
    for (int i = 0; i < 9; ++i) {
        printf("%c|", board_from_server[i]);
        if ((i + 1) % 3 == 0 && i != 0) {
            printf("\n");
        }
    }
}

void game_loop(void *args) {
    while (1) {
        pthread_mutex_lock(&reply_mutex);
        while (current_state != ACTIVE) {
            pthread_cond_wait(&input_cond, &reply_mutex);
        }
        printf("Please, select a number 1-9 to make a move\n");
        printf("Your figure is: %c", am_moving_cross ? 'X' : 'O');
        print_board(board);
        current_state = WAIT;
        pthread_mutex_unlock(&reply_mutex);
        int move;
        scanf("%d", &move);
        sprintf(buf, "%s|move_made|%d", nick, move);
        my_send_client(server_socket, buf);

    }
}

void sigint_handler() {
    sprintf(buf, "%s|exit", nick);
    my_send_client(server_socket, buf);
    exit(0);
}

void parse_commands(char *string_command) {
    server_name = strtok(string_command, "|");
    char *command = strtok(NULL, "|");
    char *args = strtok(NULL, "|");
    if (compare(command, "ping")) {
        sprintf(buf, "%s|pong", nick);
        my_send_client(server_socket, buf);
    } else if (compare(command, "move_cross")) {
        am_moving_cross = 1;
        strcpy(board, args);
        pthread_mutex_lock(&reply_mutex);
        current_state = ACTIVE;
        pthread_mutex_unlock(&reply_mutex);
        pthread_cond_signal(&input_cond);
    } else if (compare(command, "wait_for_move")) {
        print_board(args);
        printf("Waiting for opponents move\n");
    } else if (compare(command, "move_dot")) {
        am_moving_cross = 0;
        strcpy(board, args);
        pthread_mutex_lock(&reply_mutex);
        current_state = ACTIVE;
        pthread_mutex_unlock(&reply_mutex);
        pthread_cond_signal(&input_cond);
    } else if (compare(command, "exit")) {
        close(server_socket);
        exit(0);
    } else if (compare(command, "won")) {
        printf("You won! Congratulations!\n");
    } else if (compare(command, "loose")) {
        printf("You lost! I am sorry :(\n");
        sigint_handler();
    } else if (compare(command, "remiss")) {
        printf("It's remiss!\n");
        sigint_handler();
    }
}


/**
 * Accepts 3 arguments:
 * type: <local|internet>
 * nick: string without spaces
 * destination: Path for AF_UNIX or Port for Internet depending on type
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    if (argc != 4) {
        printf("Invalid number of arguments\n");
        exit(-1);
    }

    signal(SIGINT, sigint_handler);

    char *type = argv[1];
    nick = argv[2];
    char *path_dep = argv[3];

    if (strcmp(type, "local") == 0) {
        is_local = 1;
        set_up_local_socket(path_dep);
    } else {
        set_up_network_socket(path_dep);
    }

    register_in_server();

    //    Thread to game loop
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, (void *(*)(void *)) game_loop, NULL);


    printf("Waiting for partner\n");

    while (1) {
        if (is_local) {
            my_receive(client_socket, buf);

        } else {
            my_receive(server_socket, buf);
        }
        parse_commands(buf);
    }
}