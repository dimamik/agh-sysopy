//
// Created by dzmitry on 4/25/21.
//

#include "config.h"

char *get_path() {
    char *path = getenv("HOME");
    if (path == NULL) {
        fprintf(stderr, "Error in get_path: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return path;
}

key_t generate_regular_key() {
    key_t key;
    if ((key = ftok(get_path(), 1)) == -1) {
        fprintf(stderr, "Error in generate_regular_key: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return key;
}

key_t generate_unique_key() {
    key_t key;
    if ((key = ftok(get_path(), getpid())) == -1) {
        fprintf(stderr, "Error in generate_unique_key: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return key;
}

int create_queue(int key) {
    int queue_id;
    if ((queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        fprintf(stderr, "Error in create_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return queue_id;
}

void delete_queue(int queue_id) {
    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Error in delete_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int get_queue_id_based_on_key(int key) {
    int queue_id;
    if ((queue_id = msgget(key, 0)) == -1) {
        fprintf(stderr, "Error in get_queue_id_based_on_key: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return queue_id;
}

void send_message_to_queue(int queue_id, message_t *message) {
    if (msgsnd(queue_id, message, MAX_MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "Error in send_message_to_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void get_message_with_wait(int queue_id, message_t *message) {
    if (msgrcv(queue_id, message, MAX_MESSAGE_SIZE, -10, 0) == -1) {
        fprintf(stderr, "Error in get_message: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void get_message_instant(int queue_id, message_t *message) {
    if (msgrcv(queue_id, message, MAX_MESSAGE_SIZE, -10, IPC_NOWAIT) == -1) {
        fprintf(stderr, "Error in get_message_instant: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

}

int string_to_int(char *string) {
    return (int) strtol(string, NULL, 10);
}

