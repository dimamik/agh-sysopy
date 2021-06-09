#include <signal.h>
#include "common.h"

#define SERVER_NAME "server"
pthread_t ping_thread;
int number_of_clients = 0;
pthread_mutex_t number_of_clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t list_of_clients[MAXIMUM_NUMBER_OF_CLIENTS] = {0};
char buf[MAX_MESSAGE_LENGTH];
game_t list_of_games[MAXIMUM_NUMBER_OF_CLIENTS / 2];
int local_socket;

int network_socket;

void pair_if_possible(int index);

void delete_client_at_index(int index) {
    printf("Deleting client number %d\n", index);
    list_of_clients[index].is_client_present = 0;
    list_of_clients[index].ping_pong = 0;
    number_of_clients--;
}

int is_pinging = 1;

void ping_loop(void *args) {

    while (is_pinging) {

        pthread_mutex_lock(&number_of_clients_mutex);

        for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
            if (list_of_clients[i].is_client_present && !list_of_clients[i].ping_pong) {
                delete_client_at_index(i);
            }
        }

        for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
            if (list_of_clients[i].is_client_present) {
                printf("Ping %d client\n", i);
                sprintf(buf, "%s|ping", SERVER_NAME);
                my_send(list_of_clients[i].client_fd, buf,list_of_clients[i].client_address);
                list_of_clients[i].ping_pong = 0;
            }
        }
        pthread_mutex_unlock(&number_of_clients_mutex);

        sleep(5);
    }
}

/**
 * Waits for local or network fd to be ready to perform I/O
 * And when ready - accepts it
 * @return
 */
int initialize_polling_requests() {
    struct pollfd polling_requests[2];
    polling_requests[0].fd = local_socket;
    polling_requests[1].fd = network_socket;
    polling_requests[0].events = POLLIN;
    polling_requests[1].events = POLLIN;

//    Infinitely wait for event in events
    poll(polling_requests, 2, -1);

    for (int i = 0; i < 2; ++i) {
        if (polling_requests[i].revents & POLLIN) {
            return polling_requests[i].fd;
        }
    }
    return -1;

}

int initialize_local_socket(char *local_socket_path) {
    int local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un local_socket_address;
    memset(&local_socket_address, 0, sizeof(struct sockaddr_un));
    local_socket_address.sun_family = AF_UNIX;
    strcpy(local_socket_address.sun_path, local_socket_path);
    unlink(local_socket_path);

//    Assigning a name to a socket local_socket
    if (bind(local_socket, (struct sockaddr *) &local_socket_address,
             sizeof(struct sockaddr_un))) {
        printf("No such address: %s", local_socket_path);
        perror("Error in binding address");
        exit(-1);
    }
    return local_socket;
}

int initialize_network_socket(char *port) {
    struct addrinfo *address_of_service_provider;
    struct addrinfo hints;
    /**
     * The value AF_UNSPEC indicates that
     * getaddrinfo() should return socket addresses  for  any  ad‐
     * dress family (either IPv4 or IPv6, for example) that can be
     * used with node and service
     */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &address_of_service_provider)) {
        perror("Error in getaddrinfo: ");
        exit(-1);
    }
    network_socket = socket(address_of_service_provider->ai_family, address_of_service_provider->ai_socktype,
                            address_of_service_provider->ai_protocol);
    if (bind(network_socket, address_of_service_provider->ai_addr, address_of_service_provider->ai_addrlen)) {
        perror("Error in binding address\n");
        exit(-1);
    }
    freeaddrinfo(address_of_service_provider);

    return network_socket;
}


int is_client_present(int index) {
    return list_of_clients[index].is_client_present;
}

int is_nick_available(char *nick) {
    for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
        if (is_client_present(i) && compare(nick,
                                            list_of_clients[i].nick)) {
            return 0;
        }
    }
    return 1;
}

int is_free_place() {
    for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
        if (!is_client_present(i)) {
            return i;
        }
    }
    return -1;
}

void add_new_client(char *nick, int client_socket_descriptor, int index, struct sockaddr client_address) {
    strcpy(list_of_clients[index].nick, nick);
    list_of_clients[index].client_fd = client_socket_descriptor;
    list_of_clients[index].is_client_present = 1;
    list_of_clients[index].ping_pong = 1;
    list_of_clients[index].client_address = client_address;

    sprintf(buf, "ok");
    my_send(client_socket_descriptor,buf,client_address);
    printf("Client (%s,%d,%d) has been added\n",
           list_of_clients[index].nick,
           list_of_clients[index].client_fd,
           list_of_clients[index].is_client_present
    );
    number_of_clients++;
}

int get_client_id_by_nick(char *nick) {
    pthread_mutex_lock(&number_of_clients_mutex);
    int to_ret = -1;
    for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
        if (list_of_clients[i].is_client_present && compare(list_of_clients[i].nick, nick)) {
            to_ret = i;
            break;
        }
    }
    pthread_mutex_unlock(&number_of_clients_mutex);

    return to_ret;
}

int get_connected_opponent(int index) {
    int to_ret;
    if (index % 2 == 0) {
        to_ret = index + 1;
    } else {
        to_ret = index - 1;
    }
    if (list_of_clients[to_ret].is_client_present) {
        return to_ret;
    } else {
        return -1;
    }
}

typedef int Arr3[3];

/**
 * 0 - dot
 * 1 - cross
 * -1 - empty
 * @param board
 * @return
 */
Arr3 *convert_board_to_array(const char board[BOARD_SIZE]) {
    static int array[3][3];

    for (int i = 0; i < 9; ++i) {
        switch (board[i]) {
            case 'O':
                array[i / 3][i % 3] = 0;
                break;
            case 'X':
                array[i / 3][i % 3] = 1;
                break;
            default:
                array[i / 3][i % 3] = -1;
                break;
        }
    }
    return array;
}

/**
 *
 * @param board
 * @return 0 if dot won, 1 if cross won, 2 in remiss, -1 in other cases
 */
int check_if_won(char board[BOARD_SIZE]) {
    Arr3 *array = convert_board_to_array(board);
//    Check horizontal and vertical
    for (int offset = 0; offset < 3; ++offset) {
        if (array[0][offset] == array[1][offset] &&
            array[1][offset] == array[2][offset] && array[2][offset] != -1
                ) {
            return array[2][offset];
        }
        if (array[offset][0] == array[offset][1] &&
            array[offset][1] == array[offset][2] && array[offset][2] != -1
                ) {
            return array[offset][1];
        }
    }
//    Check diagonal
    if (array[0][0] == array[1][1] &&
        array[1][1] == array[2][2] && array[2][2] != -1
            ) {
        return array[2][2];
    }
    if (array[0][2] == array[1][1] &&
        array[1][1] == array[2][0] && array[2][0] != -1
            ) {
        return array[2][0];
    }

    for (int i = 0; i < 9; ++i) {
        if (array[i / 3][i % 3] == -1) {
            return -1;
        }
    }

    return 2;
}

/**
 * Returns 0 when game should can be continued, and 1 when it should be stopped
 * @param player_id
 * @param move
 * @return
 */
int make_move(int player_id, int move) {
    char symbol = 'O';
    if (list_of_clients[player_id].am_i_cross) {
        symbol = 'X';
    }

//    Check if move is valid
    if (list_of_games[player_id / 2].board[move] == 'O' ||
        list_of_games[player_id / 2].board[move] == 'X'
            ) {
//        Send message to repeat the move
        if (list_of_clients[player_id].am_i_cross) {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, list_of_games[player_id / 2].board);
        } else {
            sprintf(buf, "%s|move_dot|%s", SERVER_NAME, list_of_games[player_id / 2].board);

        }
        my_send(list_of_clients[player_id].client_fd, buf,list_of_clients[player_id].client_address);
        return 0;
    }

    list_of_games[player_id / 2].board[move] = symbol;
    int who_won = check_if_won(list_of_games[player_id / 2].board);
    if (who_won != -1) {
//        Meaning that somebody won the game
        int crosses_fg, dot_fd;
        struct sockaddr crosses_address,dot_address;
        if (list_of_clients[player_id].am_i_cross) {
            crosses_fg = list_of_clients[player_id].client_fd;
            crosses_address = list_of_clients[player_id].client_address;
            dot_fd = list_of_clients[get_connected_opponent(player_id)].client_fd;
            dot_address = list_of_clients[get_connected_opponent(player_id)].client_address;
        } else {
            dot_fd = list_of_clients[player_id].client_fd;
            dot_address = list_of_clients[player_id].client_address;
            crosses_fg = list_of_clients[get_connected_opponent(player_id)].client_fd;
            crosses_address = list_of_clients[get_connected_opponent(player_id)].client_address;
        }
        if (who_won == 1) {
            sprintf(buf, "%s|won", SERVER_NAME);
            my_send(crosses_fg, buf,crosses_address);
            sprintf(buf, "%s|loose", SERVER_NAME);
            my_send(dot_fd, buf,dot_address);
        } else if (who_won == 0) {
            sprintf(buf, "%s|won", SERVER_NAME);
            my_send(dot_fd, buf,dot_address);
            sprintf(buf, "%s|loose", SERVER_NAME);
            my_send(crosses_fg, buf,dot_address);
        } else {
            sprintf(buf, "%s|remiss", SERVER_NAME);
            my_send(dot_fd, buf,dot_address);
            sprintf(buf, "%s|remiss", SERVER_NAME);
            my_send(crosses_fg, buf,dot_address);
        }
        return 1;
    }
    return 0;
}

void copy_boards(char board_dest[BOARD_SIZE], const char board_src[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board_dest[i] = board_src[i];
    }
    board_dest[BOARD_SIZE] = '\0';
}

void process_command(char *string_command, int socket_descriptor, struct sockaddr sender_address) {
    char *nick = strtok(string_command, "|");
    char *command = strtok(NULL, "|");
    char *args = strtok(NULL, "|");
    int client_id = get_client_id_by_nick(nick);

    if (command == NULL) {
        return;
    }

    if (compare(command, "register")) {
        int index = -1;
        pthread_mutex_lock(&number_of_clients_mutex);
        if (is_nick_available(nick) && (index = is_free_place()) != -1) {
            add_new_client(nick, socket_descriptor, index,sender_address);
        } else if (!is_nick_available(nick)) {
            sprintf(buf, "occupied");
            my_send(socket_descriptor,buf,sender_address);
        } else {
            sprintf(buf, "full");
            my_send(socket_descriptor,buf,sender_address);
        }
        pthread_mutex_unlock(&number_of_clients_mutex);

        pair_if_possible(index);

    } else if (compare(command, "exit")) {
        pthread_mutex_lock(&number_of_clients_mutex);
        if (client_id == -1) {
            printf("No such client!\n");
            exit(-1);
        }
        int opponent_id;
        if ((opponent_id = get_connected_opponent(client_id)) != -1) {
//            Client has opponent
            sprintf(buf, "%s|exit", SERVER_NAME);
            my_send(list_of_clients[opponent_id].client_fd,buf,list_of_clients[opponent_id].client_address);
            delete_client_at_index(opponent_id);

        }
        delete_client_at_index(client_id);
        printf("Successfully deleted client!\n");
        pthread_mutex_unlock(&number_of_clients_mutex);

    } else if (compare(command, "pong")) {
        list_of_clients[client_id].ping_pong = 1;
    } else if (compare(command, "move_made")) {
        int opponent_fd = list_of_clients[get_connected_opponent(client_id)].client_fd;
        struct sockaddr opponent_address = list_of_clients[get_connected_opponent(client_id)].client_address;
        int move = atoi(args);
        if (make_move(client_id, move - 1)) {
            return;
        }
        char board[BOARD_SIZE + 1] = {0};

        copy_boards(board, list_of_games[(int) client_id / 2].board);

        sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);

        printf("Buffer is: %s\nBoard is: %s\nGame is: %d\n", buf, list_of_games[(int) client_id / 2].board,
               (int) client_id / 2);

        my_send(list_of_clients[client_id].client_fd, buf,list_of_clients[client_id].client_address);
        if (list_of_clients[client_id].am_i_cross) {
            sprintf(buf, "%s|move_dot|%s", SERVER_NAME, board);
            my_send(opponent_fd, buf,opponent_address);
        } else {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(opponent_fd, buf,opponent_address);
        }
    }

}


void pair_if_possible(int index) {
    int opponent_index = get_connected_opponent(index);
    if (opponent_index < MAXIMUM_NUMBER_OF_CLIENTS &&
        opponent_index >= 0 && list_of_clients[opponent_index].is_client_present
            ) {
//        Pair them together

        printf("Pairing!\n");

        int who_goes_first = rand() % 2;

        list_of_clients[index].am_i_cross = who_goes_first;
        list_of_clients[opponent_index].am_i_cross = !who_goes_first;

        char board[BOARD_SIZE];
        for (int i = 0; i < BOARD_SIZE; ++i) {
            sprintf(&board[i], "%d", i + 1);
        }

        copy_boards(list_of_games[(int) index / 2].board, board);

        if (who_goes_first) {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(list_of_clients[index].client_fd, buf,list_of_clients[index].client_address);
            sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);
            my_send(list_of_clients[opponent_index].client_fd, buf,list_of_clients[opponent_index].client_address);
        } else {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(list_of_clients[opponent_index].client_fd, buf,list_of_clients[opponent_index].client_address);
            sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);
            my_send(list_of_clients[index].client_fd, buf,list_of_clients[index].client_address);
        }
    }

}

void sigint_handler() {
    for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
        if (list_of_clients[i].is_client_present) {
            sprintf(buf, "%s|exit", SERVER_NAME);
            my_send(list_of_clients[i].client_fd, buf,list_of_clients[i].client_address);
        }
    }
    pthread_cancel(ping_thread);
    shutdown(local_socket, SHUT_RDWR);
    close(network_socket);
    close(local_socket);
    printf("\nClosed all sockets and exiting\n");
    exit(0);
}


/**
 * Accepts 2 arguments:
 * Path for AF_UNIX
 * Port for Internet
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Invalid number of arguments\n");
        exit(-1);
    }

    signal(SIGINT, sigint_handler);

    char *local_path = argv[1];
    char *port = argv[2];

    local_socket = initialize_local_socket(local_path);
    network_socket = initialize_network_socket(port);

    //    Thread to ping clients
    pthread_create(&ping_thread, NULL, (void *(*)(void *)) ping_loop, NULL);
    srand(time(NULL));
    while (1) {
        int new_socket_descriptor = initialize_polling_requests();
        struct sockaddr from;
        socklen_t length = sizeof(struct sockaddr);
        recvfrom(new_socket_descriptor, buf, MAX_MESSAGE_LENGTH, 0, &from,
                 &length);

        process_command(buf, new_socket_descriptor, from);
    }
}

