//
// Created by dzmitry on 4/25/21.
//

#ifndef LAB_06_CONFIG_H
#define LAB_06_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>

#define MAX_MESSAGE_TEXT_LENGTH 4000
#define MAX_CLIENTS_NO 10
#define MAX_MESSAGE_SIZE sizeof(message_t) - sizeof(long)
#define INTERRUPT SIGUSR1
#define QUEUE_FILENAME "/posix-server"
#define QUEUE_FILENAME_MAX_LEN 20

// Types of commands
# define STOP 1
# define DISCONNECT 2
# define LIST 3
# define CONNECT 4
# define INIT 5
# define MESSAGE 6
# define RESPOND_ERROR 7


typedef struct {
    int client_id;
    char *client_queue_name;
    mqd_t client_queue_descr;
    int is_available;
    pid_t client_pid;
    pid_t connected_client_pid;
    int connected_client_id;

} client_t;


mqd_t create_queue(char *name);

mqd_t get_queue(char *name);


/**
 * Sends message to queue
 * @param queue_id
 * @param message
 */
void send_message_to_queue(mqd_t queue_descr,
                           char *message, unsigned int type);


void delete_queue(char *name);

void close_queue(mqd_t queue_descr);

int string_to_int(char *string);

void register_notification(mqd_t queue_descriptor, struct sigevent *s_sigevent);

void get_message_from_queue(mqd_t queue_descr, char *msg, unsigned int *type);

#endif //LAB_06_CONFIG_H
