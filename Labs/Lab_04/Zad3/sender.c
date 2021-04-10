//
// Created by dzmitry on 10.04.2021.
//

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "mode.h"

int sig_count = 0;
int last_signal;

int  catcher_get = 0;

void handler_SIG_FIRST(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_FIRST\n");
    sig_count++;
//    catcher_get = sigInfo->si_value.sival_int;

}

void handler_SIG_SECOND(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_SECOND\n");
    last_signal = sig_no;
    catcher_get = sigInfo->si_value.sival_int;
//    printf("Setting CATCHER_GET to %d",catcher_get);
}


/**
 * Takes three arguments:
 * PID of catcher, n - signals to send, mode - how to send them
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

//    Collecting data
    int catcher_pid = atoi(argv[1]);
    int n = atoi(argv[2]);
    signal_mode_t mode = get_mode_type(argv[3]);

    last_signal = SIG_FIRST(mode);

//    Sending signals
    for (int i = 0; i < n; ++i) {
        if (mode == SIGQUEUE) {
            if (sigqueue(catcher_pid, SIG_FIRST(mode), (union sigval) {.sival_int = getpid()}) == 0) {
                printf("Signal sent successfully!!\n");
            } else {
                perror("SIGSENT-ERROR:");
            }
        } else {
            kill(catcher_pid, SIG_FIRST(mode));
        }


    }

    if (mode == SIGQUEUE) {
        if (sigqueue(catcher_pid, SIG_SECOND(mode), (union sigval) {}) == 0) {
            printf("Signal sent successfully!!\n");
        } else {
            perror("SIGSENT-ERROR:");
        }
    } else {
        kill(catcher_pid, SIG_SECOND(mode));
    }

//    Collecting signals

    struct sigaction sig_act_usr1;
    sig_act_usr1.sa_flags = SA_SIGINFO;


    sigset_t mask_for_process;
    sigfillset(&mask_for_process);
    sigdelset(&mask_for_process, SIG_FIRST(mode));
    sigdelset(&mask_for_process, SIG_SECOND(mode));

    sigprocmask(SIG_SETMASK, &mask_for_process, NULL);


    sig_act_usr1.sa_sigaction = handler_SIG_FIRST;
    sigaction(SIG_FIRST(mode), &sig_act_usr1, NULL);


    struct sigaction sig_act_usr2;
    sig_act_usr2.sa_flags = SA_SIGINFO;


    sig_act_usr2.sa_sigaction = handler_SIG_SECOND;

    sigaction(SIG_SECOND(mode), &sig_act_usr2, NULL);

//    Configuring set of sigsuspend()
    sigset_t set_to_wait;
    sigfillset(&set_to_wait);
    sigdelset(&set_to_wait, SIG_FIRST(mode));
    sigdelset(&set_to_wait, SIG_SECOND(mode));


//    First waiting for a portion of SIG_FIRST(mode) signals, by invoking sigsuspend
    while (last_signal == SIG_FIRST(mode)) {
        sigsuspend(&set_to_wait);
    }


//    Now summing up the results

    printf("Sent %d signals",n);
    if (mode == SIGQUEUE)
        printf(", Catcher get %d signals",catcher_get);
    printf(", received back %d signals\nWas working in mode: %s\n",sig_count,argv[3]);

}