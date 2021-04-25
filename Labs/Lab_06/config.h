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
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define MAX_MESSAGE_TEXT_LENGTH 150
#define MAX_CLIENTS_NO 10
#define MAX_MESSAGE_SIZE sizeof(message_t) - sizeof(long)
#define INTERRUPT SIGUSR1

// Types of commands
# define STOP 0
# define DISCONNECT 1
# define LIST 2
# define CONNECT 3
# define INIT 4
# define MESSAGE 5
# define RESPOND_ERROR 6


typedef struct {
    long message_type;
    int sender_id;
    pid_t sender_pid;
    char message_text[MAX_MESSAGE_TEXT_LENGTH];

} message_t;

typedef struct {
    int client_id;
    int client_queue_id;
    int is_available;
    pid_t client_pid;
    pid_t connected_client_pid;
    int connected_client_id;

} client_t;

/**
 * @return $HOME variable
 */
char *get_path();

/**
 * Generates the same key for all sessions, making it possible to retrieve
 * the same value in every point of runtime
 * @return
 */
key_t generate_regular_key();

/**
 * Generates pseudo random key, based on pid
 * @return
 */
key_t generate_unique_key();

/**
 * Creates a queue using a key, if not exists
 * @param key
 * @return id of created queue
 */
int create_queue(int key);

/**
 * Deletes queue based on queue id
 * @param queue_id
 */
void delete_queue(int queue_id);

/**
 * Given an existing queue returns its id
 * @param key
 * @return
 */
int get_queue_id_based_on_key(int key);

/**
 * Sends message to queue
 * @param queue_id
 * @param message
 */
void send_message_to_queue(int queue_id, message_t *message);

/**
 * Gets last message without waiting for it to come if doesn't exist
 * @param queue_id
 * @param message
 */
void get_message_instant(int queue_id, message_t *message);

/**
 * Waits for message to come and then copies it to message
 * @param queue_id
 * @param message
 */
void get_message_with_wait(int queue_id, message_t *message);

int string_to_int(char *string);


#endif //LAB_06_CONFIG_H
