#include <time.h>
#include "config.h"

int client_id;
int am_connected_with_id = -1;
int active = 1;
char *queue_filename;
mqd_t queue_descriptor;
mqd_t am_connected_with_queue_descriptor = -1;
char am_connected_with_queue_name[MAX_MESSAGE_TEXT_LENGTH] = "";
mqd_t server_queue_descriptor;


void sigint_handler() {
    printf("\nStopping the client\n");
    exit(0);
}

void stop_command(const char *message) {
    if (!active) {
    } else {
        active = 0;
        if (message == NULL) {
            //        Send stop to server
            char request[20];
            sprintf(request, "%d", client_id);

            send_message_to_queue(server_queue_descriptor, request, STOP);
        }

        printf("Client has been stopped\n");
        exit(0);
    }
}

/**
 *
 * @param message
 */
void connect_from_server(char *message) {



    sscanf(message, "%d|%s", &am_connected_with_id, am_connected_with_queue_name);

    am_connected_with_queue_descriptor = get_queue(am_connected_with_queue_name);

    printf("I am connected with %d\n", am_connected_with_id);
}

void respond_error() {
    printf("Can't connect to given client\n");
}

void disconnect() {
    am_connected_with_id = -1;
    printf("Disconnected from the client\n");
}

void receive_message(char *message) {
    printf("%s", message);
}

void send_message(char *text) {
    if (am_connected_with_id == -1) {
        fprintf(stderr, "Can't send message, try connecting with somebody!\n");
        return;
    }

    char message[MAX_MESSAGE_TEXT_LENGTH];

    sprintf(message, "%s", text);
    send_message_to_queue(am_connected_with_queue_descriptor, message, MESSAGE);
}

void get_list_from_server(char *respond) {
    printf("%s", respond);
}

void sig_interrupt_handler(int signo) {

    char *message = malloc(MAX_MESSAGE_TEXT_LENGTH * sizeof(char));

    unsigned int type;
    get_message_from_queue(queue_descriptor, message, &type);
    printf("Received a message: %s\n",message);
    switch (type) {
        case STOP:
            stop_command(message);
            break;
        case LIST:
            get_list_from_server(message);
            break;
        case CONNECT:
            connect_from_server(message);
            break;
        case DISCONNECT:
            disconnect();
            break;
        case MESSAGE:
            receive_message(message);
            break;
        case RESPOND_ERROR:
            respond_error();
        default:
            fprintf(stderr, "Unsupported message format %s\n", message);
            break;
    }
}

void disconnect_command() {
    if (am_connected_with_id == -1) {
        fprintf(stderr, "Can't disconnect because is not connected!\n");
        return;
    }

    char message[20];
    sprintf(message, "%d", client_id);
    send_message_to_queue(server_queue_descriptor, message, DISCONNECT);
    disconnect();
}

void get_list_request() {

    char request[50];
    sprintf(request, "%d", client_id);

    send_message_to_queue(server_queue_descriptor, request, LIST);
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
    char request[50];
    sprintf(request, "%d|%d", client_id, client_to_connect_id);

    send_message_to_queue(server_queue_descriptor, request, CONNECT);
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


char *generate_random_string(int len) {
    static int mySeed = 25011984;
    srand(time(NULL) * len + ++mySeed);
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK1234567890";
    char *random_string = malloc(sizeof(char) * (len + 1));
    sprintf(random_string, "/q-");

    for (int i = 3; i < len; i++) {
        int key = rand() % (int) (sizeof(charset) - 1);
        random_string[i] = charset[key];
    }
    random_string[len] = '\0';
    printf("Random string: %s\n", random_string);
    return random_string;
}

void log_in_to_server() {
    queue_filename = generate_random_string(QUEUE_FILENAME_MAX_LEN);
    queue_descriptor = create_queue(queue_filename);
    server_queue_descriptor = get_queue(QUEUE_FILENAME);

    send_message_to_queue(server_queue_descriptor, queue_filename, INIT);

    unsigned int type;
    char msg[16];
    get_message_from_queue(queue_descriptor, msg, &type);

    sscanf(msg, "%d", &client_id);

    printf("Client successfully connected to server with client_id: %d\n", client_id);
}

void notifications() {
    struct sigevent s_sigevent;
    s_sigevent.sigev_signo = SIGRTMIN;
    s_sigevent.sigev_notify = SIGEV_SIGNAL;

    register_notification(queue_descriptor, &s_sigevent);
}

int main() {
    atexit(clear_all);
    signal(SIGINT, sigint_handler);

    struct sigaction sigact;
    sigact.sa_handler = sig_interrupt_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(INTERRUPT, &sigact, NULL);

    log_in_to_server();

    notifications();

    // Take input parameters
    char line[MAX_MESSAGE_TEXT_LENGTH];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }
        input_handler(line);
    }
}
