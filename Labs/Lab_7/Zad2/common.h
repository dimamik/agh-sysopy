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
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>


#ifndef LAB_7_COMMON_H
#define LAB_7_COMMON_H

#define SHARED_NAME "/shared_memory"

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


float randomFloat(float min, float max) {
    return ((max - min) * ((float) rand() / RAND_MAX)) + min;
}


#define COOKING_TIME (randomFloat(1,2))
//#define COOKING_TIME (rand() % 1 + 1)
#define BAKING_TIME (randomFloat(4,5))
//#define BAKING_TIME (rand() % 2 + 1)
#define DELIVERY_TIME (randomFloat(3,4))

#define MAX(x, y) ((x>y)?x:y)


typedef struct {
    int type;
    int is_there_pizza;
} pizza_t;

typedef struct {
    pizza_t list_of_pizzas[MAX(OVEN_SIZE, TABLE_SIZE)];
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

char* get_path_of_semaphore(int sem_number) {
    static char path[5];
    sprintf(path, "/%d", sem_number);
    return path;
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

shared_memory_t *initialize_shared_memory() {

    int shared_id = shm_open(SHARED_NAME, O_CREAT | O_RDWR, 0666);
    if (shared_id == -1) {
        perror("Error in shm_open\n");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shared_id, sizeof(shared_memory_t)) == -1) {
        perror("Error in ftruncate\n");
        exit(EXIT_FAILURE);
    }
    shared_memory_t *sharedMemory;
    if ((sharedMemory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, shared_id, 0)) ==
        (void *) 1) {
        perror("Error in mmap\n");
        exit(EXIT_FAILURE);
    }
    return sharedMemory;

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


sem_t **initialize_semaphores() {
    sem_t **semaphores_list = calloc(5, sizeof(sem_t *));
    int default_values[5];
    default_values[0] = 0;
    default_values[1] = UNLOCKED;
    default_values[2] = TABLE_SIZE;
    default_values[3] = UNLOCKED;
    default_values[4] = OVEN_SIZE;


    for (int i = 0; i < 5; ++i) {

        semaphores_list[i] = sem_open(get_path_of_semaphore(i), O_CREAT, 0666, default_values[i]);
        if (semaphores_list[i] == SEM_FAILED) {
            perror("Error in initialize_semaphores\n");
        }

    }
    return semaphores_list;
}

/**
 * Need to be used only after initialize_semaphores()
 * @return
 */
sem_t **get_semaphores() {
    sem_t **semaphores_list = calloc(5, sizeof(sem_t *));
    for (int i = 0; i < 5; ++i) {
        semaphores_list[i] = sem_open(get_path_of_semaphore(i), O_RDWR);
        if (semaphores_list[i] == SEM_FAILED) {
            perror("Error in initialize_semaphores\n");
        }

    }
    return semaphores_list;
}

void change_semaphore_value(sem_t *semaphores_list[5], int sem_number, int difference) {
    if (difference == -1) {
        if (sem_wait(semaphores_list[sem_number]) == -1) {
            perror("Error in sem_wait\n");
            exit(EXIT_FAILURE);

        }

    } else if (difference == 1) {
        if (sem_post(semaphores_list[sem_number]) == -1) {
            perror("Error in sem_post\n");
            exit(EXIT_FAILURE);
        }

    } else {
        printf("Unsupported value\n");
        exit(EXIT_FAILURE);

    }

}

void unlink_semaphores() {
    for (int i = 0; i < 5; ++i) {
        if (sem_unlink(get_path_of_semaphore(i)) == -1) {
            perror("Error in clear_semaphore\n");
        }
    }

}

void close_semaphores(sem_t **semaphores) {

    for (int i = 0; i < 5; ++i) {
        if (sem_close(semaphores[i]) == -1) {
            perror("Error in close_semaphores\n");
        }
    }


}




void close_shared_memory(shared_memory_t *sharedMemory) {
    if (munmap(sharedMemory, sizeof(shared_memory_t)) == -1)
        perror("Error in close_shared_memory");
}

void delete_shared_memory() {
    if (shm_unlink(SHARED_NAME) == -1) {
        perror("Error in delete_shared_memory");
    }
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
