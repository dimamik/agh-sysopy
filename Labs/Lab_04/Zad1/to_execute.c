#include <stdio.h>
#include <zlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {
    sigset_t testMask;
        if (strcmp(argv[1], "pending") != 0) {
            printf("Signal from exec to exec sent!\n");
            raise(SIGUSR1);
        }
        if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
            sigpending(&testMask);
            if (sigismember(&testMask, SIGUSR1)) {
                printf("I am an !EXEC!, and there is a signal SIGUSR1 pending!\n");
            }
        }

    }

