#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mode.h"

int sig_count = 0;
int pid_sender = 0;
signal_mode_t mode;

void handler_SIG_FIRST(int sig_no, siginfo_t *sigInfo, void *ucontext) {
    sig_count++;
    if (mode == SIGQUEUE) {
        if (sigqueue(sigInfo->si_pid, SIG_FIRST(mode), (union sigval) {.sival_int = sig_count}) == 0) {
//            printf("Signal sent successfully!!\n");
        } else {
            perror("SIGSENT-ERROR:");
        }
    } else {
        kill(sigInfo->si_pid, SIG_FIRST(mode));
    }
    sigset_t set_to_wait;
    sigfillset(&set_to_wait);
    sigdelset(&set_to_wait, SIG_FIRST(mode));
    sigdelset(&set_to_wait, SIG_SECOND(mode));
    sigsuspend(&set_to_wait);
}

void handler_SIG_SECOND(int sig_no, siginfo_t *sigInfo, void *ucontext) {
    printf("Hello in handler_SIG_SECOND\n");
    pid_sender = sigInfo->si_pid;
}


int main(int argc, char **argv) {
    mode = get_mode_type(argv[1]);
    printf("Process ID: %d\n", getpid());

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

    pause();

    printf("Got %d signals from process with PID: %d\n", sig_count, pid_sender);

    return 0;
}