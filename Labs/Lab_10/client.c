#include <signal.h>
#include "common.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

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

void set_up_local_socket(char *local_socket_path) {
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un local_socket_address;
    memset(&local_socket_address, 0, sizeof(struct sockaddr_un));
    local_socket_address.sun_family = AF_UNIX;
    strcpy(local_socket_address.sun_path, local_socket_path);

    if (connect(server_socket, (struct sockaddr *) &local_socket_address,
                sizeof(struct sockaddr_un))) {
        perror("Error while connecting\n");
        exit(-1);
    }
}

void set_up_network_socket(char *port) {
    struct addrinfo *info;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", port, &hints, &info)) {
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
    my_send(server_socket, buf);
    printf("Sent message to server\n");

    recv(server_socket, buf, MAX_MESSAGE_LENGTH, 0);
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

void clear_all() {
//    TODO Send to server message that you are quitting
    sprintf(buf, "%s|exit", nick);
    my_send(server_socket, buf);
}


void print_board(char* board_from_server){
    printf("\n");
    for (int i = 0; i < 9; ++i) {
        printf("%c|",board_from_server[i]);
        if ((i+1)%3==0 && i!=0){
            printf("\n");
        }
    }
}

void game_loop(void* args){
    while (1){

        pthread_mutex_lock(&reply_mutex);

        while (current_state != ACTIVE){
            pthread_cond_wait(&input_cond,&reply_mutex);
            printf("GOT SIGNAL!\n");
        }

        printf("Hello there\n");
        printf("Please, select a number 1-9 to make a move\n");
        print_board(board);
        current_state = WAIT;
        pthread_mutex_unlock(&reply_mutex);
        int move;
        scanf("%d",&move);
        sprintf(buf,"%s|move_made|%d",nick,move);
        my_send(server_socket,buf);

    }
}

void parse_commands(char* string_command){
    char *server_name = strtok(string_command, "|");
    char *command = strtok(NULL, "|");
    char *args = strtok(NULL, "|");
    if (compare(command,"ping")){
        sprintf(buf, "%s|pong",nick);
        my_send(server_socket, buf);
    }
    else if(compare(command,"move_cross")){
        am_moving_cross = 1;
        strcpy(board,args);
        pthread_mutex_lock(&reply_mutex);
        current_state = ACTIVE;
        pthread_mutex_unlock(&reply_mutex);
        pthread_cond_signal(&input_cond);
    }else if(compare(command,"wait_for_move")){
        print_board(args);
        printf("I am waiting for opponents move\n");
    }else if(compare(command,"move_dot")){
        am_moving_cross = 0;
        strcpy(board,args);
        pthread_mutex_lock(&reply_mutex);
        current_state = ACTIVE;
        pthread_mutex_unlock(&reply_mutex);
        pthread_cond_signal(&input_cond);
    }else if(compare(command,"exit")){
        exit(0);
    }else if(compare(command,"won")){
        printf("You won! Congratulations!\n");
    }else if(compare(command,"loose")){
        printf("You lost! I am sorry :(\n");
    }
}

void sigint_handler(){
    sprintf(buf, "%s|exit", nick);
    my_send(server_socket, buf);
    exit(0);
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
//    atexit(clear_all);

    signal(SIGINT, sigint_handler);

    char *type = argv[1];
    nick = argv[2];
    char *path_dep = argv[3];

    if (strcmp(type, "local") == 0) {
        set_up_local_socket(path_dep);
    } else {
        set_up_network_socket(path_dep);
    }

//    TODO Registration Process
    register_in_server();

    //    Thread to game loop
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, (void *(*)(void *)) game_loop, NULL);


    printf("Waiting for partner\n");

    while (1) {
//        TODO Game Logic from client side
        my_receive(server_socket, buf);

        parse_commands(buf);
    }
}

#pragma clang diagnostic pop