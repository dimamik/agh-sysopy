#include <time.h>
#include "config.h"

int client_id;
int server_queue = -1;
int queue_id;
int am_connected_with_id = -1;
int am_connected_with_queue = -1;
int am_connected_with_process = -1;
int active = 1;

void init_queue() {
    key_t key = generate_unique_key();
    queue_id = create_queue(key);
    if (queue_id == -1) {
        fprintf(stderr, "Unable to create a client!\n");
        exit(EXIT_FAILURE);
    }
    message_t request;
    sprintf(request.message_text, "%d", queue_id);
    request.message_type = INIT;
    request.sender_pid = getpid();
    request.sender_id = -1;
    send_message_to_queue(server_queue, &request);
    message_t respond;

    get_message_with_wait(queue_id, &respond);
    client_id = string_to_int(respond.message_text);
    printf("Client was successfully connected to server\n");
    printf("Client ID is %d\n", client_id);
}

void sigint_handler() {
    printf("\nStopping the client\n");
    exit(0);
}


void stop_command(message_t *message) {
    if (!active) {
        return;
    } else {
        active = 0;
        if (message == NULL) {

//        Send stop to server
            message_t request;
            request.message_type = STOP;
            request.sender_id = client_id;
            send_message_to_queue(server_queue, &request);

        }
        delete_queue(queue_id);
        printf("Client has been stopped\n");
        exit(0);
    }


}


void connect_from_server(message_t *message) {
    am_connected_with_queue = string_to_int(message->message_text);
    am_connected_with_id = message->sender_id;
    am_connected_with_process = message->sender_pid;
    printf("I am connected with %d\n", am_connected_with_id);

}

void respond_error() {
    printf("Can't connect to given client\n");
}

void disconnect() {
    am_connected_with_id = -1;
    am_connected_with_queue = -1;
    printf("Disconnected from the client\n");
}


void receive_message(message_t *message) {
    printf("%s", message->message_text);
}


void send_message(char *text) {
    if (am_connected_with_id == -1) {
        fprintf(stderr, "Can't send message, try connecting with somebody!\n");
        return;
    }
    message_t message;
    message.message_type = MESSAGE;
    sprintf(message.message_text, "%s", text);
    send_message_to_queue(am_connected_with_queue, &message);
    kill(am_connected_with_process, INTERRUPT);

}


void get_list_from_server(message_t *respond) {
    printf("%s", respond->message_text);
}

void sig_interrupt_handler() {
    message_t message;
    get_message_instant(queue_id, &message);
    switch (message.message_type) {
        case STOP:
            stop_command(&message);
            break;
        case LIST:
            get_list_from_server(&message);
            break;
        case CONNECT:
            connect_from_server(&message);
            break;
        case DISCONNECT:
            disconnect();
            break;
        case MESSAGE:
            receive_message(&message);
            break;
        case RESPOND_ERROR:
            respond_error();
        default:
            fprintf(stderr, "Unsupported message format %ld\n", message.message_type);
            break;
    }

}

void disconnect_command() {
    if (am_connected_with_id == -1) {
        fprintf(stderr, "Can't disconnect because is not connected!\n");
        return;
    }
    message_t request;
    request.message_type = DISCONNECT;
    request.sender_id = client_id;
    send_message_to_queue(server_queue, &request);
    disconnect();
}


void get_list_request() {
    message_t request;

    request.message_type = LIST;
    request.sender_pid = getpid();
    request.sender_id = client_id;
    send_message_to_queue(server_queue, &request);

}

/**
 * Sends to server connect request and waits for
 * respond.
 * In positive, gets client2 queue to write messages at
 * @param client_to_connect_id
 */
void connect_command(int client_to_connect_id) {
    if (am_connected_with_id != -1) {
        fprintf(stderr, "Can't connect because is already connected!\n");
        return;
    }
    if (client_to_connect_id == client_id) {
        fprintf(stderr, "Can't connect with yourself!\n");
        return;
    }
    message_t request;
    sprintf(request.message_text, "%d", client_to_connect_id);
    request.message_type = CONNECT;
    request.sender_id = client_id;
    request.sender_pid = getpid();

    send_message_to_queue(server_queue, &request);


}

void input_handler(char *line) {
    char *first_word;
    char *context;
    first_word = strtok_r(line, " \n", &context);

    if (strcmp("STOP", first_word) == 0) {
        exit(0);
    } else if (strcmp("CONNECT", first_word) == 0) {
        connect_command(string_to_int(context));
    } else if (strcmp("DISCONNECT", first_word) == 0) {
        disconnect_command();
    } else if (strcmp("MESSAGE", first_word) == 0) {
        send_message(context);
    } else if (strcmp("LIST", first_word) == 0) {
        get_list_request();
    }

}

void clear_all() {
    stop_command(NULL);
}

int main() {
    atexit(clear_all);
    signal(SIGINT, sigint_handler);
    server_queue = get_queue_id_based_on_key(generate_regular_key());
    signal(INTERRUPT, sig_interrupt_handler);
    init_queue();

// Take input parameters
    char line[MAX_MESSAGE_TEXT_LENGTH];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }
        input_handler(line);
    }

}
