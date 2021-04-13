#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void signalHandler(int sig_no) {
    printf("Got signal: %d, and I am process: %d\n", sig_no, getpid());
}

int main(int argc, char **argv) {

    sigset_t maskSet, testMask;
    sigemptyset(&maskSet);

    if (argc >= 2) {
        printf("\n------------------%s---------------------\n", argv[1]);
        if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
            sigaddset(&maskSet, SIGUSR1);
            if (sigprocmask(SIG_BLOCK, &maskSet, NULL) < 0)
                perror("There is a problem while masking a signal\n");
        } else if (strcmp(argv[1], "ignore") == 0) {
            signal(SIGUSR1, SIG_IGN);
        } else if (strcmp(argv[1], "handler") == 0) {
            signal(SIGUSR1, signalHandler);
        }

        printf("Signal to parent from parent sent!\n");
        raise(SIGUSR1);
        if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
            sigpending(&testMask);
            if (sigismember(&testMask, SIGUSR1)) {
                printf("I am a parent, and there is a signal SIGUSR1 pending!\n");
            }
        }
        if (argc != 3) {
            int pid = fork();

            if (pid == 0) {
                if (strcmp(argv[1], "pending") != 0) {
                    printf("Signal to child from child sent!\n");
                    raise(SIGUSR1);
                }
                if (strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
                    sigpending(&testMask);
                    if (sigismember(&testMask, SIGUSR1)) {
                        printf("I am a child, and there is a signal SIGUSR1 pending!\n");
                    }
                }
            }
        } else {
            printf("RUNNING EXEC\n");
            execl("./to_execute", "./to_execute", argv[1], NULL);
        }
    } else {
        printf("Not enough arguments");
        exit(-1);
    }

    while (wait(NULL) > 0) {
    }
}