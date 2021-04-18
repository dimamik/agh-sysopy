#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    int current[2], prev[2];
    char **commands = malloc(sizeof(char *) * 2);
    commands[0] = malloc(sizeof(char) * 50);
    commands[1] = malloc(sizeof(char) * 50);
    strcpy(commands[0], "cat");
    strcpy(commands[1], "/etc/passwd");
    char **commands2 = malloc(sizeof(char *) * 2);
    commands2[0] = malloc(sizeof(char) * 50);
    commands2[1] = malloc(sizeof(char) * 50);
    strcpy(commands2[0], "wc");
    strcpy(commands2[1], "-l");
    int first_entry = 1;
    for (int i = 0; i < 2; ++i) {
        pipe(current);
        pid_t pid = fork();

        if (pid == 0) {
            if (first_entry != 1){
                close(prev[1]);
                dup2(prev[0], STDIN_FILENO);
            }

            if (i == 1) {
                if (execvp(commands[0], commands) == -1) {
//                    printf("Can't execute command\n");
                    exit(1);
                }
            } else {
                if (execvp(commands2[0], commands2) == -1) {
//                    printf("Can't execute command\n");
                    exit(1);
                }
            }
        }
        close(current[1]);

        prev[0] = current[0];
        prev[1] = current[1];
        first_entry = 0;
    }

}