//
// Program sender przyjmuje trzy parametry: PID procesu catcher, ilość sygnałów do wysłania i tryb wysłania sygnałów.
//
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "mode.h"

int last_signal;
int sig_count = 0;
int pid_sender = -1;

void handler_SIG_FIRST(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_FIRST(mode)\n");
    sig_count++;
}

void handler_SIG_SECOND(int sig_no, siginfo_t *sigInfo, void *ucontext) {
//    printf("Hello in handler_SIG_SECOND(mode)\n");
    pid_sender = sigInfo->si_pid;
    last_signal = sig_no;
//    printf("Sender of signal PID is : %d\n", pid_sender);
}


int main(int argc, char **argv) {


    signal_mode_t mode = get_mode_type(argv[1]);
    last_signal = SIG_FIRST(mode);
    printf("Process ID: %d\n", getpid());


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


//    First waiting for a portion of SIG_FIRST(mode) signals, by invoking waiter for
    while (last_signal == SIG_FIRST(mode)) {
        sigsuspend(&set_to_wait);
    }

//    Wysyłam count_sig sygnałów do pid_sender

//    Sending signals
    for (int i = 0; i < sig_count; ++i) {

        if (mode == SIGQUEUE) {
            if (sigqueue(pid_sender, SIG_FIRST(mode), (union sigval) {.sival_int = sig_count}) == 0) {
                printf("Signal sent successfully!!\n");
            } else {
                perror("SIGSENT-ERROR:");
            }
        } else {
            kill(pid_sender, SIG_FIRST(mode));
        }


    }
    if (mode == SIGQUEUE) {
        if (sigqueue(pid_sender, SIG_SECOND(mode), (union sigval) {.sival_int = sig_count}) == 0) {
            printf("SIG_SECOND sent successfully!!\n");
        } else {
            perror("SIGSENT-ERROR:");
        }
    } else {
        kill(pid_sender, SIG_SECOND(mode));
        printf("SIG_SECOND sent successfully!!\n");
    }

    printf("Got %d signals from process with PID: %d\n", sig_count, pid_sender);

    return 0;
}