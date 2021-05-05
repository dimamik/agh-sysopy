
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        exit(1);
    }

    int toChildFD[2];
    int toParentFD[2];

    pipe(toChildFD);
    pipe(toParentFD);

    int val1, val2, val3 = 0;

    pid_t pid;

    if ((pid = fork()) == 0) {

        char buff[30];
        read(toChildFD[0], buff, 30);
        val2 = atoi(buff);
        //odczytaj z potoku nienazwanego wartosc przekazana przez proces macierzysty i zapisz w zmiennej val2

        val2 = val2 * val2;

        //wyslij potokiem nienazwanym val2 do procesu macierzysego

        char new_number[5];
        sprintf(new_number, "%d", val2);
        write(toParentFD[1], new_number, 30);

    } else {

        val1 = atoi(argv[1]);

        //wyslij val1 potokiem nienazwanym do priocesu potomnego

        write(toChildFD[1],argv[1],30);

//        sleep(1);

        //odczytaj z potoku nienazwanego wartosc przekazana przez proces potomny i zapisz w zmiennej val3

        char buffer[30];

        read(toParentFD[0],buffer,30);
        val3 = atoi(buffer);

        printf("%d square is: %d\n", val1, val3);
    }
    return 0;
}
