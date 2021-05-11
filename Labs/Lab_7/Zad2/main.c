#include <wait.h>
#include "common.h"

//int semaphores_id;
sem_t **semaphores_list;
shared_memory_t *shared_memory;

int *delivery_processes;
int *cooker_processes;

int delivery_n;
int cooker_n;

void clear_all() {
    printf("\nEnding cookers jobs\n");

    for (int i = 0; i < cooker_n; ++i) {
        kill(cooker_processes[i], SIGINT);
    }
    for (int i = 0; i < delivery_n; ++i) {
        kill(delivery_processes[i], SIGINT);
    }

    close_shared_memory(shared_memory);
    delete_shared_memory();
    close_semaphores(semaphores_list);
    unlink_semaphores();

    free(delivery_processes);
    free(cooker_processes);

}

void at_exit(int signum) {

    exit(0);
}

int main(int argc, char **argv) {
//    Define at_exit
    atexit(clear_all);

    signal(SIGINT, at_exit);
//  Create shared memory

    cooker_n = atoi(argv[1]);
    delivery_n = atoi(argv[2]);


    cooker_processes = calloc(cooker_n, sizeof(int));
    delivery_processes = calloc(delivery_n, sizeof(int));


    shared_memory = initialize_shared_memory();
    table_t *oven = &shared_memory->oven;
    table_t *table = &shared_memory->table;

    initialize_tables(oven, table);

    semaphores_list = initialize_semaphores();

//    Execute cookers and deliveryman programs

    for (int i = 0; i < cooker_n; ++i) {
        int pid;
        if ((pid = fork()) == 0) {

            execlp("./cooker", "./cooker", NULL);
        } else if (pid != -1) {
            cooker_processes[i] = pid;
        } else {
            perror("ERROR in fork()\n");
        }
    }

    for (int i = 0; i < delivery_n; ++i) {
        int pid;
        if ((pid = fork()) == 0) {
            execlp("./deliveryman", "./deliveryman", NULL);
        } else {
            delivery_processes[i] = pid;
        }
    }

    while (wait(NULL) > 0) {

    }

    return 0;
}
