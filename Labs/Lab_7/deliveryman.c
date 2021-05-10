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
    int currently_in_oven;
    int currently_on_table;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    while (1) {


        change_semaphore_value(semaphores_id, S_DELIVERYMAN_TABLE_IS_EMPTY, DECREASE);
        printf("D: DECREASE\n");
        change_semaphore_value(semaphores_id, S_COOKER_TABLE_IS_FULL, INCREASE);
        printf("D: INCREASE\n");


        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, LOCKED);

        printf("D: LOCKED\n");


        printf("DELIVERY RUNNING TAKE\n");
        current_pizza = *take_pizza_from_table(table, TABLE_SIZE);
//        currently_on_table = TABLE_SIZE - semaphore_get_value(semaphores_id, S_COOKER_TABLE_IS_FULL);
        currently_on_table = (table->last_added - table->last_taken)%TABLE_SIZE;

        printf("D: %d ", getpid());
        get_time();
        printf(" Took pizza of type: %d , currently on table: %d\n", current_pizza.type, currently_on_table);

        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, UNLOCKED);
        printf("D: UNLOCKED\n");


    }
#pragma clang diagnostic pop


}