#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#ifndef LAB_7_COMMON_H
#define LAB_7_COMMON_H

#define PATHNAME getenv("HOME")
#define PROJECT_ID 65

#define OVEN_SIZE 5
#define TABLE_SIZE 5
#define PIZZA_MAX_TYPE 10

#define LOCKED (-1)
#define UNLOCKED 1

#define INCREASE 1
#define DECREASE (-1)

#define S_DELIVERYMAN_TABLE_IS_EMPTY 0
#define S_TABLE_IN_USE 1
#define S_COOKER_TABLE_IS_FULL 2
#define S_BAKERY_IN_USE 3
#define S_BAKERY_IS_FULL 4


#define PREPARATION_TIME (rand() % 2 + 1)
//#define PREPARATION_TIME (rand() % 1 + 1)
#define BAKING_TIME (rand() % 3 + 4)
//#define BAKING_TIME (rand() % 2 + 1)
#define DELIVERY_TIME (rand() % 3 + 4)

#define MAX(x,y) ((x>y)?x:y)

typedef struct {
    int type;
    int is_there_pizza;
} pizza_t;

typedef struct {
    pizza_t list_of_pizzas[MAX(OVEN_SIZE,TABLE_SIZE)];
    int last_added;
    int last_taken;
} table_t;




typedef struct {
    table_t oven;
    table_t table;
} shared_memory_t;

void add_pizza_to_table(table_t *place_holder, pizza_t pizza, int size) {

    if (place_holder->list_of_pizzas[place_holder->last_added % size].is_there_pizza != 0) {
        printf("Error in add_pizza_to_table: %d %d %d %d----------------------\n",
               place_holder->last_added,
               size,
               getpid(),
               place_holder->list_of_pizzas[place_holder->last_added % size].is_there_pizza
        );
        printf("\n%d\n", place_holder->list_of_pizzas[5].is_there_pizza);
        exit(EXIT_FAILURE);
    } else {
        place_holder->list_of_pizzas[place_holder->last_added % size] = pizza;
        place_holder->list_of_pizzas[place_holder->last_added % size].is_there_pizza = 1;
        place_holder->last_added++;
    }
//    if (place_holder->last_added == size) {
//        place_holder->last_added = 0;
//    }
}

pizza_t *take_pizza_from_table(table_t *place_holder, int size) {

    if (place_holder->list_of_pizzas[place_holder->last_taken % size].is_there_pizza == 0) {
        printf("Error in take_pizza_from_table: %d %d %d",
               place_holder->last_taken,
               size,
               place_holder->list_of_pizzas[place_holder->last_taken % size].is_there_pizza
        );
        exit(EXIT_FAILURE);
    } else {
//        printf("Oven last taken: %d\n", place_holder->last_taken);
        place_holder->list_of_pizzas[place_holder->last_taken % size].is_there_pizza = 0;
        pizza_t *to_ret = &place_holder->list_of_pizzas[(place_holder->last_taken) % size];
        place_holder->last_taken++;
        return to_ret;
    }
}


shared_memory_t *initialize_shared_memory(int *shared_memory_id) {

    key_t key = ftok(PATHNAME, PROJECT_ID);
    *shared_memory_id = shmget(key, sizeof(shared_memory_t), 0666 | IPC_CREAT);
    if (*shared_memory_id == -1) {
        perror("Error in initialize_shared_memory");
        exit(EXIT_FAILURE);
    }
    return (shared_memory_t *) shmat(*shared_memory_id, NULL, 0666);
}

void initialize_tables(table_t *oven, table_t *table) {

    for (int i = 0; i < OVEN_SIZE; ++i) {
        oven->list_of_pizzas[i].is_there_pizza = 0;
    }
    oven->last_added = 0;
    oven->last_taken = 0;
    for (int i = 0; i < TABLE_SIZE; ++i) {
        table->list_of_pizzas[i].is_there_pizza = 0;
    }
    table->last_added = 0;
    table->last_taken = 0;
}

void set_semaphore_value(int sem_id, int sem_number, int new_value) {
    semctl(sem_id, sem_number, SETVAL, new_value);
}

int semaphore_get_value(int semaphore_id, int sem_number) {
    return semctl(semaphore_id, sem_number, GETVAL);

}

int initialize_semaphores() {
    key_t key = ftok(PATHNAME, PROJECT_ID);
    int semaphores_id = semget(key, 5, 0666 | IPC_CREAT);
    return semaphores_id;
}

void set_up_semaphores(int semaphores_id) {
    set_semaphore_value(semaphores_id, S_DELIVERYMAN_TABLE_IS_EMPTY, 0);
    set_semaphore_value(semaphores_id, S_BAKERY_IN_USE, UNLOCKED);
    set_semaphore_value(semaphores_id, S_BAKERY_IS_FULL, OVEN_SIZE);
    set_semaphore_value(semaphores_id, S_COOKER_TABLE_IS_FULL, TABLE_SIZE);
    set_semaphore_value(semaphores_id, S_TABLE_IN_USE, UNLOCKED);
}

void change_semaphore_value(int semaphore_id, int sem_number, int difference) {
    struct sembuf sem_buff = {.sem_num = sem_number, .sem_op = (short) difference, .sem_flg = 0};
    semop(semaphore_id, &sem_buff, 1);

}

void clear_shared_memory(int id) {
    shmctl(id, IPC_RMID, NULL);
}

void clear_semaphores(int id) {
    semctl(id, 0, IPC_RMID, 0);
}

void get_time() {
    // variables to store the date and time components
    int hours, minutes, seconds;

    // `time_t` is an arithmetic time type
    time_t now;

    // Obtain current time
    // `time()` returns the current time of the system as a `time_t` value
    time(&now);
    struct tm *local = localtime(&now);

    hours = local->tm_hour;         // get hours since midnight (0-23)
    minutes = local->tm_min;        // get minutes passed after the hour (0-59)
    seconds = local->tm_sec;        // get seconds passed after a minute (0-59)

    struct timespec timespec;

    clock_gettime(CLOCK_REALTIME, &timespec);


    printf("%02d:%02d:%02d:%02ld", hours, minutes, seconds, timespec.tv_nsec / 1000000);
}

#endif //LAB_7_COMMON_H
