
#include "config.h"

mqd_t server_queue;
int clients_no = 0;
client_t *clients[MAX_CLIENTS_NO] = {NULL};

void sigint_handler() {
    printf("\nServer is shutting down\n");
    exit(0);
}

/**
 * Returns index in client array pointing to available place
 * and -1 in case of no empty places
 * @return
 */
int get_free_client_id() {
    for (int i = 0; i < MAX_CLIENTS_NO; ++i) {
        if (clients[i] == NULL) {
            return i;
        }
    }
    return -1;
}

void stop_command() {

    for (int i = 0; i < MAX_CLIENTS_NO; ++i) {
        if (clients[i] != NULL) {
            printf("Sending stop message to client %d\n", i);
            send_message_to_queue(clients[i]->client_queue_descr, "", STOP);
        }
    }
    close_queue(server_queue);
    delete_queue(QUEUE_FILENAME);
    printf("Server is successfully shut down\n");
}

void disconnect(int client1, int client2) {
    if (client1 < MAX_CLIENTS_NO && client2 < MAX_CLIENTS_NO &&
        clients[client1] != NULL && clients[client2] != NULL) {
        clients[client1]->is_available = 1;
        clients[client2]->is_available = 1;
    }

    send_message_to_queue(clients[client2]->client_queue_descr, "", DISCONNECT);
    printf("Sent disconnect message to %d",client2);
}

void stop_client_command(char *message) {

    int client_id;
    sscanf(message, "%d", &client_id);

    if (clients[client_id] != NULL && clients[client_id]->is_available == 0) {
        disconnect(client_id, clients[client_id]->connected_client_id);
    }
    if (clients[client_id] != NULL) {
        clients[client_id] = NULL;
        clients_no--;
    }
    printf("Client %d has stopped\n", client_id);
}

void disconnect_command(char *message) {
    int client1;
    sscanf(message, "%d", &client1);
    int client2;
    if (client1 < MAX_CLIENTS_NO && clients[client1] != NULL) {

        client2 = clients[client1]->connected_client_id;
        disconnect(client1, client2);
    } else {
        return;
    }
    printf("Disconnected clients %d and %d\n", client1, client2);
}

char *is_available_print(int status) {
    static char availability[25];
    if (status) {
        sprintf(availability, "is available\n");
    } else {
        sprintf(availability, "is not available\n");
    }
    return availability;
}

char *list_command_get_string() {
    static char client_list[MAX_MESSAGE_TEXT_LENGTH];
    client_list[0] = '\0';
    strcat(client_list, "Actual server clients:\n");
    for (int i = 0; i < MAX_CLIENTS_NO; ++i) {
        if (clients[i] != NULL) {
            int status = clients[i]->is_available;
            char temp_string[MAX_MESSAGE_TEXT_LENGTH];
            sprintf(temp_string, "Client with id: %d and %s", clients[i]->client_id,
                    is_available_print(status));
            strcat(client_list, temp_string);
        }
    }
    return client_list;
}

void list_command(char *message) {
    int sender_id;
    sscanf(message, "%d", &sender_id);

    char *to_return = calloc(MAX_MESSAGE_TEXT_LENGTH, sizeof(char));

    sprintf(to_return, "%s", list_command_get_string());
    send_message_to_queue(clients[sender_id]->client_queue_descr, to_return, LIST);
}

void init_command(char *message) {
    int client_id = get_free_client_id();
    if (client_id == -1) {
        fprintf(stderr, "Can't create more clients than %d\n", MAX_CLIENTS_NO);
        exit(EXIT_FAILURE);
    }

    client_t *client = calloc(1, sizeof(client_t));
    client->client_id = client_id;
    client->client_queue_name = malloc(QUEUE_FILENAME_MAX_LEN * sizeof(char));
    sscanf(message, "%s", client->client_queue_name);
    client->client_queue_descr = get_queue(client->client_queue_name);

    client->connected_client_id = -1;
    client->is_available = 1;

    clients[client_id] = client;
    clients_no++;

    char reply_msg[5];
    sprintf(reply_msg, "%d", client_id);
    send_message_to_queue(client->client_queue_descr, reply_msg, INIT);
}

int does_client_exist(int client_id) {
    return client_id < MAX_CLIENTS_NO && clients[client_id] != NULL;
}

/**
 *
 * @param message
 */
void connect_command(char *message) {
    int client_1;
    int client_2;

    sscanf(message, "%d|%d", &client_1, &client_2);

    unsigned int respond;
    char *req_m = calloc(MAX_MESSAGE_TEXT_LENGTH, sizeof(char));
    if (client_2 < 0 || client_2 > MAX_CLIENTS_NO || !does_client_exist(client_1) || !does_client_exist(client_2) ||
        !clients[client_2]->is_available) {
        respond = RESPOND_ERROR;
    } else {
        // Sending client_1|client_1_queue_descr

        sprintf(req_m, "%d|%s", client_1, clients[client_1]->client_queue_name);
        send_message_to_queue(clients[client_2]->client_queue_descr, req_m, CONNECT);
        respond = CONNECT;

        clients[client_1]->connected_client_id = clients[client_2]->client_id;
        clients[client_2]->connected_client_id = clients[client_1]->client_id;
        clients[client_1]->is_available = 0;
        clients[client_2]->is_available = 0;
        printf("Connected clients %d and %d\n", client_1, client_2);
        sprintf(req_m, "%d|%s", client_2, clients[client_2]->client_queue_name);
    }

    send_message_to_queue(clients[client_1]->client_queue_descr, req_m, respond);
    free(req_m);
}

// TODO Check
void handle_queue_message() {
    char *message = calloc(MAX_MESSAGE_TEXT_LENGTH, sizeof(char));
    unsigned int type;
    get_message_from_queue(server_queue, message, &type);
    printf("Received a message: %s\n",message);
    switch (type) {
        case STOP:
            stop_client_command(message);
            break;
        case DISCONNECT:
            disconnect_command(message);
            break;
        case LIST:
            list_command(message);
            break;
        case INIT:
            init_command(message);
            break;
        case CONNECT:
            connect_command(message);
            break;
        default:
            break;
    }
    free(message);
}

int main() {
    //   Define STOP Signal
    atexit(stop_command);
    signal(SIGINT, sigint_handler);

    //    Generate new queue
    server_queue = create_queue(QUEUE_FILENAME);
    printf("Server is running with queue_id: %d\n", server_queue);
    //    Listen to the commands

    while (1) {
        handle_queue_message();
    }
}