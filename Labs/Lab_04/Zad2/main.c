#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int how_many_signals = 5;

void handler_child(int sig_no, siginfo_t *sigInfo, void *ucontext) {
    printf("I am a signal handler with next parameters:\n");
    printf("SIG number: %d\n", sigInfo->si_signo);
    printf("Sent Process ID: %d\n", sigInfo->si_pid);
    printf("Error number: %d\n", sigInfo->si_errno);
    printf("Child Exit Value: %d\n", sigInfo->si_status);
}

void handle_parallel_signals(int sig_no) {
    if (how_many_signals > 0) {
        int sig_num = how_many_signals;
        printf("Hello in handle_parallel_signals, currently working on SIG: %d\n", how_many_signals);
        how_many_signals--;
        raise(sig_no);

        printf("Exiting handle_parallel_signals SIG: %d\n", sig_num);
    }
}

void handler_value(int sig_no, siginfo_t *sigInfo, void *ucontext) {
    printf("I am a handler with next parameters:\n");
    printf("Signal number: %d\n", sig_no);
    printf("Sent Process ID: %d\n", sigInfo->si_pid);
    printf("Error number: %d\n", sigInfo->si_errno);
    printf("si_code (Who sent a signal) : ");
    if (sigInfo->si_code == SI_USER) {
        printf("user\n");
    } else if (sigInfo->si_code == SI_KERNEL) {
        printf("kernel\n");
    } else if (sigInfo->si_code == SI_QUEUE) {
        printf("sigqueue\n");
    }
    printf("Value sent: %d\n", sigInfo->si_value.sival_int);
}

void handle_sigchld(int sig) {
    printf("I am handler: handle_sigchld and I am calling waitpid not to reap zombie processes!\n");
    while (waitpid((pid_t) (-1), 0, WNOHANG) > 0) {
    }
}

int main(int argc, char **argv) {

    printf("\n\n------------%s---------------\n", argv[1]);

    if (strcmp(argv[1], "siginfo") == 0) {

        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        sigemptyset(&act.sa_mask);

        printf("------------%s---------------\n", argv[2]);
        if (strcmp(argv[2], "child") == 0) {
            act.sa_sigaction = &handler_child;
            sigaction(SIGCHLD, &act, NULL);
            pid_t child_pid = fork();

            if (child_pid == 0) {
                exit(155);
            }
        } else if (strcmp(argv[2], "value") == 0) {

            act.sa_sigaction = handler_value;
            act.sa_flags = SA_SIGINFO;
            sigemptyset(&act.sa_mask);
            sigaction(SIGUSR1, &act, NULL);

            if (sigqueue(getpid(), SIGUSR1, (union sigval) {.sival_int = 1998}) == 0) {
                printf("Signal sent successfully!!\n");
            } else {
                perror("SIGSENT-ERROR:");
            }
        } else if (strcmp(argv[2], "alarm") == 0) {
            act.sa_sigaction = handler_value;
            act.sa_flags = SA_SIGINFO;
            sigemptyset(&act.sa_mask);
            sigaction(SIGALRM, &act, NULL);
            kill(getpid(), SIGALRM);
        }
    } else if (strcmp(argv[1], "SA_RESTART__SA_NOCLDSTOP__SA_NOCLDWAIT") == 0) {
        /**
         * Scenario: Ensure that zombie process are removed
         */
        struct sigaction sa;
        sa.sa_handler = &handle_sigchld;
        sigemptyset(&sa.sa_mask);
        /**
         * Flags description:
         * SA_RESTART - Provide behavior compatible with BSD signal semantics by
              making certain system calls restartable across signals.
           SA_NOCLDSTOP - If signum is SIGCHLD, do not receive notification when
              child processes stop (i.e., when they receive one of
              SIGSTOP, SIGTSTP, SIGTTIN, or SIGTTOU) or resume (i.e.,
              they receive SIGCONT)
           SA_NOCLDWAIT - If signum is SIGCHLD, do not transform children into
              zombies when they terminate.
         */
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT;
        if (sigaction(SIGCHLD, &sa, 0) == -1) {
            perror(0);
            exit(1);
        }
        pid_t child_pid = fork();

        if (child_pid == 0) {
            printf("I am a child that does it's stuff\n");
            exit(0);
        }
    } else if (strcmp(argv[1], "SA_NODEFER") == 0) {

        /**
         * With SA_NODEFER the queue is LIFO
         * SA_NODEFER: Do not prevent the signal from being received from within its own signal handler.
         * SA_NOMASK is an obsolete, non-standard synonym for this flag.
         *
         *  Do not add the signal to the thread's signal mask while
              the handler is executing, unless the signal is specified
              in act.sa_mask.
         */
        struct sigaction sa;
        sa.sa_handler = &handle_parallel_signals;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_NODEFER;
        if (sigaction(SIGUSR1, &sa, 0) == -1) {
            perror(0);
            exit(1);
        }

        raise(SIGUSR1);

        printf("------------WITHOUT SA_NODEFER---------------\n");
        /**
         * Without SA_NODEFER the type of queue is FIFO
         */
        struct sigaction sa2;
        sa2.sa_handler = &handle_parallel_signals;
        sigemptyset(&sa2.sa_mask);
        if (sigaction(SIGUSR1, &sa2, 0) == -1) {
            perror(0);
            exit(1);
        }
        how_many_signals = 5;
        raise(SIGUSR1);

    }


    while (wait(NULL) > 0) {
    }
}
