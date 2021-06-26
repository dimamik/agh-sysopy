#include <time.h>
#include <signal.h>
#include "common.h"

int shared_memory_id;
int semaphores_id;

void at_exit() {
    exit(0);
}

void clear_all() {
}

int main() {

    //    Define at_exit
    atexit(clear_all);

    signal(SIGINT, at_exit);

    shared_memory_t *shared_memory = initialize_shared_memory(&shared_memory_id);
    table_t *oven = &shared_memory->oven;
    table_t *table = &shared_memory->table;

    semaphores_id = initialize_semaphores();

    srand(getpid() * time(NULL));
    pizza_t current_pizza;
    int currently_in_oven;
    int currently_on_table;
    while (1) {


        int pizza_type = rand() % PIZZA_MAX_TYPE;
        current_pizza.type = pizza_type;
        current_pizza.is_there_pizza = 1;

        printf("C: %d ", getpid());
        get_time();
        printf(" Cooking pizza of type: %d\n", pizza_type);

        usleep(COOKING_TIME * 1000 * 1000);


        change_semaphore_value(semaphores_id, S_BAKERY_IS_FULL, DECREASE);


        change_semaphore_value(semaphores_id, S_BAKERY_IN_USE, LOCKED);


        currently_in_oven = OVEN_SIZE - semaphore_get_value(semaphores_id, S_BAKERY_IS_FULL);

        add_pizza_to_table(oven, current_pizza, OVEN_SIZE);

        printf("C: %d ", getpid());
        get_time();
        printf(" Added pizza of type: %d to oven, in oven are: %d\n", pizza_type, currently_in_oven);

        change_semaphore_value(semaphores_id, S_BAKERY_IN_USE, UNLOCKED);

        usleep(BAKING_TIME * 1000 * 1000);


        change_semaphore_value(semaphores_id, S_BAKERY_IN_USE, LOCKED);

        current_pizza = *take_pizza_from_table(oven, OVEN_SIZE);
        currently_in_oven = OVEN_SIZE - 1 - semaphore_get_value(semaphores_id, S_BAKERY_IS_FULL);

        printf("C: %d ", getpid());
        get_time();
        printf(" Took pizza of type: %d from oven, in oven are: %d\n", current_pizza.type, currently_in_oven);


        change_semaphore_value(semaphores_id, S_BAKERY_IN_USE, UNLOCKED);

        change_semaphore_value(semaphores_id, S_BAKERY_IS_FULL, INCREASE);

//    Taking pizzas to table
//
        change_semaphore_value(semaphores_id, S_COOKER_TABLE_IS_FULL, DECREASE);
        currently_on_table = abs(table->last_added - table->last_taken) % TABLE_SIZE + 1;

        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, LOCKED);


        current_pizza.is_there_pizza = 1;
        add_pizza_to_table(table, current_pizza, TABLE_SIZE);

        change_semaphore_value(semaphores_id, S_DELIVERYMAN_TABLE_IS_EMPTY, INCREASE);


        printf("C: %d ", getpid());
        get_time();
        printf(" Placed pizza on table of type: %d, on table are: %d\n", pizza_type, currently_on_table);


        change_semaphore_value(semaphores_id, S_TABLE_IN_USE, UNLOCKED);

    }


}