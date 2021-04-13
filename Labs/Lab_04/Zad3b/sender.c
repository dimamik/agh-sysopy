#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "mode.h"

int sig_count = 0;
int last_signal;

int catcher_get = 0;
signal_mode_t mode;

int got_signal = 0;

void handler_SIG_FIRST(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_FIRST\n");
    sig_count++;
    catcher_get = sigInfo->si_value.sival_int;
    got_signal = 1;

}

void handler_SIG_SECOND(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_SECOND\n");
    last_signal = sig_no;
    catcher_get = sigInfo->si_value.sival_int;
//    printf("Setting CATCHER_GET to %d",catcher_get);
    got_signal = 1;
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
    mode = get_mode_type(argv[3]);

    last_signal = SIG_FIRST(mode);

//    Collecting signals

    sigset_t mask_for_process;
    sigfillset(&mask_for_process);
    sigdelset(&mask_for_process, SIG_FIRST(mode));
    sigdelset(&mask_for_process, SIG_SECOND(mode));
    sigprocmask(SIG_SETMASK, &mask_for_process, NULL);

    struct sigaction sig_act_usr1;
    sig_act_usr1.sa_flags = SA_SIGINFO;
    sig_act_usr1.sa_sigaction = handler_SIG_FIRST;
    sigaction(SIG_FIRST(mode), &sig_act_usr1, NULL);


    struct sigaction sig_act_usr2;
    sig_act_usr2.sa_flags = SA_SIGINFO;
    sig_act_usr2.sa_sigaction = handler_SIG_SECOND;
    sigaction(SIG_SECOND(mode), &sig_act_usr2, NULL);

//    Sending signals
    for (int i = 0; i < n; ++i) {
        got_signal = 0;
        if (mode == SIGQUEUE) {
            if (sigqueue(catcher_pid, SIG_FIRST(mode), (union sigval) {}) == 0) {
//                printf("Signal sent successfully!!\n");
            } else {
                perror("SIGSENT-ERROR:");
            }
        } else {
            kill(catcher_pid, SIG_FIRST(mode));

        }
        while (!got_signal) {

        }
//        sigset_t set_to_wait;
//        sigfillset(&set_to_wait);
//        sigdelset(&set_to_wait, SIG_FIRST(mode));
//        sigdelset(&set_to_wait, SIG_SECOND(mode));
//        sigsuspend(&set_to_wait);

    }

    if (mode == SIGQUEUE) {
        if (sigqueue(catcher_pid, SIG_SECOND(mode), (union sigval) {}) == 0) {
//            printf("Signal sent successfully!!\n");
        } else {
            perror("SIGSENT-ERROR:");
        }
    } else {
        kill(catcher_pid, SIG_SECOND(mode));
    }


//    Now summing up the results

    printf("Sent %d signals", n);
    if (mode == SIGQUEUE)
        printf(", Catcher get %d signals", catcher_get);
    printf(", received back %d signals\nWas working in mode: %s\n", sig_count, argv[3]);

}