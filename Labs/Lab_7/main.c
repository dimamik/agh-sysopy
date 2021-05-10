#include <wait.h>
#include "common.h"

int shared_memory_id;
int semaphores_id;

void clear_all() {
    clear_shared_memory(shared_memory_id);
    clear_semaphores(semaphores_id);
}


int main(int argc, char **argv) {
//    Define at_exit
    atexit(clear_all);

//  Create shared memory


    shared_memory_t *shared_memory = initialize_shared_memory(&shared_memory_id);
    table_t *oven = &shared_memory->oven;
    table_t *table = &shared_memory->table;

    initialize_tables(oven, table);

    semaphores_id = initialize_semaphores();
    set_up_semaphores(semaphores_id);

//    Execute cookers and deliveryman programs

    for (int i = 0; i < atoi(argv[1]); ++i) {
        if (fork() == 0){
            execlp("./cooker","./cooker",NULL);
        }
    }

    for (int i = 0; i < atoi(argv[2]); ++i) {
        if (fork() == 0){
            execlp("./deliveryman","./deliveryman",NULL);
        }
    }

    while (wait(NULL) > 0) {

    }

    return 0;
}
