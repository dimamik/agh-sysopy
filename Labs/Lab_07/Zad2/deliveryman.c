#include <time.h>
#include <signal.h>
#include "common.h"

shared_memory_t* shared_memory;
sem_t** semaphores_list;

void at_exit() {
    exit(0);
}

void clear_all() {
    close_semaphores(semaphores_list);
    close_shared_memory(shared_memory);
}


int main() {
    //    Define at_exit
    atexit(clear_all);

    signal(SIGINT, at_exit);

    shared_memory = initialize_shared_memory();
    table_t *table = &shared_memory->table;

    semaphores_list = get_semaphores();

    srand(getpid() * time(NULL));
    pizza_t current_pizza;
    int currently_on_table;
    while (1) {


        change_semaphore_value(semaphores_list, S_DELIVERYMAN_TABLE_IS_EMPTY, DECREASE);


        change_semaphore_value(semaphores_list, S_TABLE_IN_USE, LOCKED);


        current_pizza = *take_pizza_from_table(table, TABLE_SIZE);

        change_semaphore_value(semaphores_list, S_COOKER_TABLE_IS_FULL, INCREASE);

//        currently_on_table = TABLE_SIZE - semaphore_get_value(semaphores_id, S_COOKER_TABLE_IS_FULL);
        currently_on_table = (table->last_added - table->last_taken) % TABLE_SIZE;

        printf("D: %d ", getpid());
        get_time();
        printf(" Delivering pizza of type: %d , currently on table: %d\n", current_pizza.type, currently_on_table);

        change_semaphore_value(semaphores_list, S_TABLE_IN_USE, UNLOCKED);

        usleep(DELIVERY_TIME * 1000 * 1000);

        printf("D: %d ", getpid());
        get_time();
        printf(" Successfully delivered pizza of type: %d \n", current_pizza.type);


    }


}