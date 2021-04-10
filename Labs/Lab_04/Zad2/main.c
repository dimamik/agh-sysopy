#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

void handlerChild(int sig_no, siginfo_t *sigInfo, void *ucontext)
{
    printf("I am a signal handler with next parameters:\n");
    printf("SIG number: %d\n", sigInfo->si_signo);
    printf("Process sent ID: %d\n", sigInfo->si_pid);
    printf("Error number: %d\n", sigInfo->si_errno);
    printf("Child Exit Value: %d\n", sigInfo->si_status);
}

void handler_value(int sig_no, siginfo_t *sigInfo, void *ucontext)
{
    printf("I am a handler with next parameters:\n");
    printf("Signal number: %d\n", sig_no);
    printf("Process sent ID: %d\n", sigInfo->si_pid);
    printf("Error number: %d\n", sigInfo->si_errno);
    printf("si_code (Who sent a signal) : ");
    if (sigInfo->si_code == SI_USER)
    {
        printf("user\n");
    }
    else if (sigInfo->si_code == SI_KERNEL)
    {
        printf("kernel\n");
    }
    else if (sigInfo->si_code == SI_QUEUE)
    {
        printf("sigqueue\n");
    }
    printf("Value sent: %d\n", sigInfo->si_value.sival_int);
}

void handle_sigchld(int sig)
{
    printf("I am handler: handle_sigchld and I am calling waitpid not to reap zombie processes!\n");
    int saved_errno = errno;
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0)
    {
    }
    errno = saved_errno;
}

int main(int argc, char **argv)
{

    printf("\n\n------------%s---------------\n", argv[1]);

    if (strcmp(argv[1], "siginfo") == 0)
    {

        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        sigemptyset(&act.sa_mask);

        printf("------------%s---------------\n", argv[2]);
        if (strcmp(argv[2], "child") == 0)
        {
            act.sa_sigaction = &handlerChild;
            sigaction(SIGCHLD, &act, NULL);
            pid_t child_pid = fork();

            if (child_pid == 0)
            {
                exit(155);
            }
        }
        else if (strcmp(argv[2], "value") == 0)
        {

            act.sa_sigaction = handler_value;
            act.sa_flags = SA_SIGINFO;
            sigemptyset(&act.sa_mask);
            sigaction(SIGUSR1, &act, NULL);

            if (sigqueue(getpid(), SIGUSR1, (union sigval){.sival_int = 1998}) == 0)
            {
                printf("Signal sent successfully!!\n");
            }
            else
            {
                perror("SIGSENT-ERROR:");
            }
        }
        else if (strcmp(argv[2], "alarm") == 0)
        {
            act.sa_sigaction = handler_value;
            act.sa_flags = SA_SIGINFO;
            sigemptyset(&act.sa_mask);
            sigaction(SIGALRM, &act, NULL);
            kill(getpid(), SIGALRM);
        }
    }
    else if (strcmp(argv[1], "SA_RESTART__SA_NOCLDSTOP__SA_NOCLDWAIT") == 0)
    {
        /**
         * Scenario: Ensure that zombie process are removed in timely manner
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
        if (sigaction(SIGCHLD, &sa, 0) == -1)
        {
            perror(0);
            exit(1);
        }
        pid_t child_pid = fork();

        if (child_pid == 0)
        {
            printf("I am a child that does it's stuff\n");
            exit(0);
        }
    }

    while (wait(NULL) > 0)
    {
    }
}
