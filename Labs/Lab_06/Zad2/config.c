//
// Created by dzmitry on 4/25/21.
//

#include "config.h"

mqd_t create_queue(char *name) {
    mqd_t queue_descriptor;

    struct mq_attr mq_attr;
    mq_attr.mq_curmsgs = 0;
    mq_attr.mq_flags = 0;
    mq_attr.mq_msgsize = MAX_MESSAGE_TEXT_LENGTH;
    mq_attr.mq_maxmsg = 10;

    if ((queue_descriptor = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &mq_attr)) == -1) {
        fprintf(stderr, "Error in create_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return queue_descriptor;
}

mqd_t get_queue(char *name) {
    mqd_t queue_descriptor;
    if ((queue_descriptor = mq_open(name, O_WRONLY)) == -1) {
        fprintf(stderr, "Error in get_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return queue_descriptor;
}

void delete_queue(char *name) {
    if (mq_unlink(name) == -1) {
        fprintf(stderr, "Error in delete_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void close_queue(mqd_t queue_descr) {
    if (mq_close(queue_descr) == -1) {
        fprintf(stderr, "Error while closing queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int string_to_int(char *string) {
    return (int) strtol(string, NULL, 10);
}

void send_message_to_queue(mqd_t queue_descr, char *message, unsigned int type) {
    if(mq_send(queue_descr, message, strlen(message), type) == -1) {
        fprintf(stderr, "Error in send_message_to_queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void register_notification(mqd_t queue_descr, struct sigevent *s_sigevent) {
    if (mq_notify(queue_descr, s_sigevent) == -1) {
        fprintf(stderr, "Error in register_notification: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void get_message_from_queue(mqd_t queue_descr, char *msg, unsigned int *type) {
    if(mq_receive(queue_descr, msg, MAX_MESSAGE_TEXT_LENGTH, type) == -1) {
        fprintf(stderr, "Error while receiving message from queue: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}