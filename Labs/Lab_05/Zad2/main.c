#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes 1 or 3 arguments, and
 *
 * If 1:
 * mode:= <DATA|EMAIL>
 *
 * If 3:
 * Address
 * Title
 * Body
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    char command[1024];
    if (argc == 2) {

        if (strcmp(argv[1], "DATA") == 0) {
//            -M to sort month, -k to specify column
            sprintf(command, "mail | sed '1d;$d' | sort -k5M -k6 -k7");
            if ((popen(command, "w")) == NULL) {
                exit(-1);
            }
        } else if (strcmp(argv[1], "EMAIL") == 0) {
            sprintf(command, "mail | sed '1d;$d' | sort -k3");
            if ((popen(command, "w")) == NULL) {
                exit(-1);
            }
        }


    } else if (argc == 4) {

        sprintf(command, "echo '%s' | mail -s '%s' %s", argv[3], argv[2], argv[1]);
        if (popen(command, "w") == NULL) {
            exit(-1);
        }
    } else {
        perror("Invalid number of arguments");
        exit(-1);
    }
    exit(0);

}