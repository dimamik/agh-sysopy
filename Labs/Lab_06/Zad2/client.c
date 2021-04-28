#include <time.h>
#include "config.h"

int my_client_id;
char *my_queue_name;
mqd_t my_queue_descriptor;

int is_active = 1;
mqd_t am_connected_with_queue_descriptor = -1;
int am_connected_with_id = -1;
char am_connected_with_queue_name[MAX_MESSAGE_TEXT_LENGTH] = "";

mqd_t server_queue_descriptor;


void sigint_handler() {
    printf("\nStopping the client\n");
    exit(0);
}

void stop_command(const char *message) {
    if (is_active) {
        is_active = 0;
        if (message == NULL) {
            // Send stop to server
            char request[20];
            sprintf(request, "%d", my_client_id);
            send_message_to_queue(server_queue_descriptor, request, STOP);
        }
        close_queue(my_queue_descriptor);
        unlink_queue(my_queue_name);
        printf("Client has been stopped\n");
        exit(0);
    }
}


void connect_from_server(char *message) {
    sscanf(message, "%d|%s", &am_connected_with_id, am_connected_with_queue_name);

    am_connected_with_queue_descriptor = get_queue_by_name(am_connected_with_queue_name);

    printf("I am connected with %d and its_queue_name is %s\n", am_connected_with_id, am_connected_with_queue_name);
}

void respond_error() {
    printf("Can't connect to given client\n");
}

void disconnect() {
    am_connected_with_id = -1;
    printf("Disconnected from the client\n");
}

void set_notifier() {
    struct sigevent s_sigevent;
    s_sigevent.sigev_signo = INTERRUPT;
    s_sigevent.sigev_notify = SIGEV_SIGNAL;
    add_notification(my_queue_descriptor, &s_sigevent);
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

void interrupt_from_outside_handler(int signo) {
    char *message = malloc(MAX_MESSAGE_TEXT_LENGTH * sizeof(char));
    printf("Received a message: %s\n", message);
    unsigned int type;
    get_message_from_queue(my_queue_descriptor, message, &type);
//    printf("Received a message: %s\n",message);
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
    set_notifier();
}

void disconnect_command() {
    if (am_connected_with_id == -1) {
        fprintf(stderr, "Can't disconnect because is not connected!\n");
        return;
    }

    char message[20];
    sprintf(message, "%d", my_client_id);
    send_message_to_queue(server_queue_descriptor, message, DISCONNECT);
    disconnect();
}

void get_list_request() {
    char request[50];
    sprintf(request, "%d", my_client_id);
    send_message_to_queue(server_queue_descriptor, request, LIST);
}


void connect_command(int client_to_connect_id) {
    if (am_connected_with_id != -1) {
        fprintf(stderr, "Can't connect because is already connected!\n");
        return;
    }
    if (client_to_connect_id == my_client_id) {
        fprintf(stderr, "Can't connect with yourself!\n");
        return;
    }
    char request[50];
    sprintf(request, "%d|%d", my_client_id, client_to_connect_id);

    send_message_to_queue(server_queue_descriptor, request, CONNECT);
}

void input_parser(char *line) {
    if (strchr(line, '\n') == NULL) {
        return;
    }
    char *command;
    char *body;
    command = strtok_r(line, " \n", &body);

    if (command == NULL) {
        return;
    }

    if (strcmp("STOP", command) == 0) {
        exit(0);
    } else if (strcmp("CONNECT", command) == 0) {
        connect_command(string_to_int(body));
    } else if (strcmp("DISCONNECT", command) == 0) {
        disconnect_command();
    } else if (strcmp("MESSAGE", command) == 0) {
        send_message(body);
    } else if (strcmp("LIST", command) == 0) {
        get_list_request();
    }
}

void clear_all() {
    stop_command(NULL);
}


char *generate_random_string(int len) {
    static int mySeed = 25011984;
    srand(time(NULL) * len + ++mySeed);
    const char letters[] = "ABCDEFGHIJKabcdefghijklmnopqrstuvwxyz";
    char *to_return = malloc(sizeof(char) * (len + 1));
    sprintf(to_return, "/q-");

    for (int i = 3; i < len; i++) {
        int key = rand() % (int) (sizeof(letters) - 1);
        to_return[i] = letters[key];
    }
    to_return[len] = '\0';
    printf("Random string: %s\n", to_return);
    return to_return;
}

void log_in_to_server() {
    my_queue_name = generate_random_string(QUEUE_FILENAME_MAX_LEN);
    my_queue_descriptor = create_queue(my_queue_name);
    server_queue_descriptor = get_queue_by_name(QUEUE_FILENAME);

    send_message_to_queue(server_queue_descriptor, my_queue_name, INIT);

    unsigned int type;
    char msg[20];
    get_message_from_queue(my_queue_descriptor, msg, &type);

    sscanf(msg, "%d", &my_client_id);

    printf("Client successfully connected to server with my_client_id: %d\n", my_client_id);
}


void set_interrupt_handler() {
    struct sigaction sigact;
    sigact.sa_handler = interrupt_from_outside_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(INTERRUPT, &sigact, NULL);
}

int main() {
    atexit(clear_all);
    signal(SIGINT, sigint_handler);

    set_interrupt_handler();

    log_in_to_server();

    set_notifier();

    // Take input parameters
    char line[MAX_MESSAGE_TEXT_LENGTH];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            continue;
        }
        input_parser(line);
    }
}
