
#include "config.h"

int server_queue_id;
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
//    TODO Send stop to clients to shut them down

    delete_queue(server_queue_id);
}

void stop_client_command(message_t *message) {

}


void disconnect(int client1, int client2) {
    if (client1 < MAX_CLIENTS_NO && client2 < MAX_CLIENTS_NO &&
        clients[client1] != NULL && clients[client2] != NULL
            ) {
        clients[client1]->is_available = 1;
        clients[client2]->is_available = 1;
    }
    message_t request;
    request.message_type = DISCONNECT;

    send_message_to_queue(clients[client2]->client_queue_id, &request);
    kill(clients[client2]->client_pid, INTERRUPT);
}

void disconnect_command(message_t *message) {
    int client1 = message->sender_id;
    if (client1 < MAX_CLIENTS_NO && clients[client1] != NULL) {

        int client2 = clients[client1]->connected_client_id;
        disconnect(client1, client2);
    } else {
        return;
    }
    printf("Disconnected client\n");
}

void list_command(message_t *message) {

}

void init_command(message_t *message) {
    int client_id = get_free_client_id();
    if (client_id == -1) {
        fprintf(stderr, "Can't create more clients than %d\n", MAX_CLIENTS_NO);
        exit(EXIT_FAILURE);
    }

    client_t *client = calloc(1, sizeof(client_t));
    client->client_id = client_id;
    client->client_pid = message->sender_pid;
    client->client_queue_id = (int) strtol(message->message_text, NULL, 10);
    client->connected_client_id = -1;
    client->connected_client_pid = -1;
    client->is_available = 1;

    clients[client_id] = client;
    clients_no++;

    message_t reply;
    reply.message_type = INIT;
    sprintf(reply.message_text, "%d", client_id);
    send_message_to_queue(client->client_queue_id, &reply);

}

int does_client_exist(int client_id) {
    return client_id < MAX_CLIENTS_NO && clients[client_id] != NULL;
}

/**
 *
 * @param message
 */
void connect_command(message_t *message) {
    int client_1 = message->sender_id;
    int client_2 = string_to_int(message->message_text);

    message_t respond;

    if (!does_client_exist(client_1) || !does_client_exist(client_2)
        || !clients[client_2]->is_available) {
        respond.message_type = RESPOND_ERROR;
    } else {
        message_t request;
        request.sender_pid = clients[client_1]->client_pid;
        request.message_type = CONNECT;
        request.sender_id = client_1;
        sprintf(request.message_text, "%d", clients[client_1]->client_queue_id);
        send_message_to_queue(clients[client_2]->client_queue_id, &request);
        kill(clients[client_2]->client_pid, INTERRUPT);
        respond.message_type = CONNECT;
        sprintf(respond.message_text, "%d", clients[client_2]->client_queue_id);

        clients[client_1]->connected_client_pid = clients[client_2]->client_pid;
        clients[client_2]->connected_client_pid = clients[client_1]->client_pid;
        clients[client_1]->connected_client_id = clients[client_2]->client_id;
        clients[client_2]->connected_client_id = clients[client_1]->client_id;
        clients[client_1]->is_available = 0;
        clients[client_2]->is_available = 0;
    }
    respond.sender_id = client_2;
    respond.sender_pid = clients[client_2]->client_pid;
    send_message_to_queue(clients[client_1]->client_queue_id, &respond);
    kill(clients[client_1]->client_pid, INTERRUPT);

}

void handle_queue_message() {
    message_t message;
    get_message_with_wait(server_queue_id, &message);
    switch (message.message_type) {
        case STOP:
            stop_client_command(&message);
            break;
        case DISCONNECT:
            disconnect_command(&message);
            break;
        case LIST:
            list_command(&message);
            break;
        case INIT:
            init_command(&message);
            break;
        case CONNECT:
            connect_command(&message);
            break;
    }

}

int main() {
//   Define STOP Signal
    atexit(stop_command);
    signal(SIGINT, sigint_handler);

//    Generate new queue
    key_t server_key = generate_regular_key();
    server_queue_id = create_queue(server_key);

//    Listen to the commands

    while (1) {
        handle_queue_message();
    }


}