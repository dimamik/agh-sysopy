#include "common.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int number_of_clients = 0;
pthread_mutex_t number_of_clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t list_of_clients[MAXIMUM_NUMBER_OF_CLIENTS] = {0};
char buf[MAX_MESSAGE_LENGTH];
game_t list_of_games[MAXIMUM_NUMBER_OF_CLIENTS / 2];

void pair_if_possible(int index);

#define SERVER_NAME "server"

void delete_client_at_index(int index) {
    printf("Deleting client number %d\n", index);
    list_of_clients[index].is_client_present = 0;
    list_of_clients[index].ping_pong = 0;
    number_of_clients--;
}

void ping_loop(void *args) {


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
            my_send(list_of_clients[i].client_fd, buf);
            list_of_clients[i].ping_pong = 0;
        }
    }
    pthread_mutex_unlock(&number_of_clients_mutex);

    sleep(5);
    ping_loop(args);
}

/**
 * Waits for local or network fd to be ready to perform I/O
 * And when ready - accepts it
 * @return
 */
int initialize_polling_requests(int local_socket, int network_socket) {
    struct pollfd *polling_requests = calloc(number_of_clients + 2,
                                             sizeof(struct pollfd));
    polling_requests[0].fd = local_socket;
    polling_requests[1].fd = network_socket;
    polling_requests[0].events = POLLIN;
    polling_requests[1].events = POLLIN;

    pthread_mutex_lock(&number_of_clients_mutex);
//    Critical section
    int current_poll = 0;
    for (int i = 0; i < MAXIMUM_NUMBER_OF_CLIENTS; ++i) {
        if (list_of_clients[i].is_client_present) {
            polling_requests[current_poll + 2].fd = list_of_clients[i].client_fd;
            polling_requests[current_poll++ + 2].events = POLLIN;
        }
    }
//    EOF Critical section
    pthread_mutex_unlock(&number_of_clients_mutex);

//    Infinitely wait for event in events
    poll(polling_requests, number_of_clients + 2, -1);


    int event_descriptor = -1;
//    Critical section
    for (int i = 0; i < number_of_clients + 2; ++i) {
        if (polling_requests[i].revents & POLLIN) {
            event_descriptor = polling_requests[i].fd;
            break;
        }
    }
//    EOF Critical section

// In case of first connection, we need to accept it
    if (event_descriptor == local_socket || event_descriptor == network_socket) {
        if ((event_descriptor = accept(event_descriptor, NULL, NULL)) == -1) {
            perror("Error in accepting connection");
            exit(-1);
        }
    }
    free(polling_requests);
//    In case of others, we just pass the descriptor
    return event_descriptor;
}

int initialize_local_socket(char *local_socket_path) {
    int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);
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

    listen(local_socket, BACKLOG_MAX);


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
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &address_of_service_provider)) {
        perror("Error in getaddrinfo: ");
        exit(-1);
    }

    int network_socket = socket(address_of_service_provider->ai_family, address_of_service_provider->ai_socktype,
                                address_of_service_provider->ai_protocol);
    if (bind(network_socket, address_of_service_provider->ai_addr, address_of_service_provider->ai_addrlen)) {
        perror("Error in binding address\n");
        exit(-1);
    }
    listen(network_socket, BACKLOG_MAX);

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

void add_new_client(char *nick, int client_socket_descriptor, int index) {
    strcpy(list_of_clients[index].nick, nick);
    list_of_clients[index].client_fd = client_socket_descriptor;
    list_of_clients[index].is_client_present = 1;
    list_of_clients[index].ping_pong = 1;

    sprintf(buf, "ok");
    send(client_socket_descriptor, buf, MAX_MESSAGE_LENGTH, 0);
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
Arr3 *convert_board_to_array(char board[BOARD_SIZE]) {
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
        if (array[i/3][i%3] == -1){
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
    list_of_games[player_id / 2].board[move] = symbol;
    int who_won = check_if_won(list_of_games[player_id / 2].board);
    if (who_won != -1) {
//        Meaning that somebody won the game
        int crosses_fg, dot_fd;
        if (list_of_clients[player_id].am_i_cross) {
            crosses_fg = list_of_clients[player_id].client_fd;
            dot_fd = list_of_clients[get_connected_opponent(player_id)].client_fd;
        } else {
            dot_fd = list_of_clients[player_id].client_fd;
            crosses_fg = list_of_clients[get_connected_opponent(player_id)].client_fd;

        }

        if (who_won == 1) {
            sprintf(buf, "%s|won", SERVER_NAME);

            my_send(crosses_fg, buf);

            sprintf(buf, "%s|loose", SERVER_NAME);

            my_send(dot_fd, buf);
        } else {
            sprintf(buf, "%s|won", SERVER_NAME);

            my_send(dot_fd, buf);

            sprintf(buf, "%s|loose", SERVER_NAME);

            my_send(crosses_fg, buf);
        }

        return 1;
    }
return 0;
}

void process_command(char *string_command, int socket_descriptor) {
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
            add_new_client(nick, socket_descriptor, index);
        } else if (!is_nick_available(nick)) {
            sprintf(buf, "occupied");
            send(socket_descriptor, buf, MAX_MESSAGE_LENGTH, 0);
        } else {
            sprintf(buf, "full");
            send(socket_descriptor, buf, MAX_MESSAGE_LENGTH, 0);
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
            send(list_of_clients[opponent_id].client_fd,
                 buf, MAX_MESSAGE_LENGTH, 0);
            delete_client_at_index(opponent_id);

        }
        delete_client_at_index(client_id);
        printf("Successfully deleted client!\n");
        pthread_mutex_unlock(&number_of_clients_mutex);

    } else if (compare(command, "pong")) {
        list_of_clients[client_id].ping_pong = 1;
    } else if (compare(command, "move_made")) {
//        TODO Add check for wining
        int opponent_fd = list_of_clients[get_connected_opponent(client_id)].client_fd;
        int move = atoi(args);
        if (make_move(client_id, move - 1)){
            return;
        }
        char board[BOARD_SIZE];
        strcpy(board, list_of_games[client_id / 2].board);
        sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);
        my_send(list_of_clients[client_id].client_fd, buf);
        if (list_of_clients[client_id].am_i_cross) {
            sprintf(buf, "%s|move_dot|%s", SERVER_NAME, board);
            my_send(opponent_fd, buf);
        } else {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(opponent_fd, buf);
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

        strcpy(list_of_games[(int) index / 2].board, board);

        if (who_goes_first) {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(list_of_clients[index].client_fd, buf);
            sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);
            my_send(list_of_clients[opponent_index].client_fd, buf);
        } else {
            sprintf(buf, "%s|move_cross|%s", SERVER_NAME, board);
            my_send(list_of_clients[opponent_index].client_fd, buf);
            sprintf(buf, "%s|wait_for_move|%s", SERVER_NAME, board);
            my_send(list_of_clients[index].client_fd, buf);
        }
    }

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

    char *local_path = argv[1];
    char *port = argv[2];

    int local_socket = initialize_local_socket(local_path);
    int network_socket = initialize_network_socket(port);


    //    Thread to ping clients
    pthread_t ping_thread;
//    pthread_create(&ping_thread, NULL, (void *(*)(void *)) ping_loop, NULL);
    srand(time(NULL));
    while (1) {
        int new_socket_descriptor = initialize_polling_requests(local_socket, network_socket);
        my_receive(new_socket_descriptor, buf);
        process_command(buf, new_socket_descriptor);
    }

}

#pragma clang diagnostic pop