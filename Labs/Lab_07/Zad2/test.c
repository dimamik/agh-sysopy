#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main() {
    // ftok to generate unique key
    key_t key = ftok("shmfile", 65);

    // shmget returns an identifier in shmid
    int shmid = shmget(key, sizeof(table_t), 0666 | IPC_CREAT);

    // shmat to attach to shared memory
    table_t *oven = (table_t *) shmat(shmid, (void *) 0, 0);

    printf("I am test, and value is %d\n", oven->list_of_pizzas[0].type);

    printf("Changing value!\n");

    oven->list_of_pizzas[0].type = 10;

    //detach from shared memory
    shmdt(oven);

}