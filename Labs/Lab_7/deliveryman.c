#include <time.h>
#include "common.h"

int shared_memory_id;
int semaphores_id;

int main() {
    shared_memory_t *shared_memory = initialize_shared_memory(&shared_memory_id);
    table_t *table = &shared_memory->table;

    semaphores_id = initialize_semaphores();

    srand(getpid() * time(NULL));
    pizza_t current_pizza;
    int currently_on_table;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    while (1) {



        change_semaphore_value(semaphores_id, S_DELIVERYMAN_TABLE_IS_EMPTY, DECREASE);


        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, LOCKED);


        current_pizza = *take_pizza_from_table(table, TABLE_SIZE);

        change_semaphore_value(semaphores_id, S_COOKER_TABLE_IS_FULL, INCREASE);

//        currently_on_table = TABLE_SIZE - semaphore_get_value(semaphores_id, S_COOKER_TABLE_IS_FULL);
        currently_on_table = (table->last_added - table->last_taken) % TABLE_SIZE;

        printf("D: %d ", getpid());
        get_time();
        printf(" Delivering pizza of type: %d , currently on table: %d\n", current_pizza.type, currently_on_table);

        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, UNLOCKED);

        usleep(DELIVERY_TIME * 1000 * 1000);

        printf("D: %d ", getpid());
        get_time();
        printf(" Successfully delivered pizza of type: %d \n", current_pizza.type);


    }
#pragma clang diagnostic pop


}