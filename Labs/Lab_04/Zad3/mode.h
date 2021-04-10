#ifndef LAB_04_MODE_H
#define LAB_04_MODE_H

typedef enum {
    KILL,
    SIGQUEUE,
    SIGRT
} signal_mode_t;


signal_mode_t get_mode_type(char *string) {
    if (strcmp(string, "KILL") == 0) return KILL;
    if (strcmp(string, "SIGQUEUE") == 0) return SIGQUEUE;
    else return SIGRT;

}

#define SIG_FIRST(MODE_TYPE) (MODE_TYPE == SIGRT ? SIGRTMIN : SIGUSR1)
#define SIG_SECOND(MODE_TYPE) (MODE_TYPE == SIGRT ? SIGRTMIN+1 : SIGUSR2)

#endif //LAB_04_MODE_H
