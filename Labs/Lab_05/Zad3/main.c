#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

/**
 * N - number of chars to send
 * producers_number - number of producers
 * size - size of single string in line
 * consumers_number - number of consumers
 * type_of_test - type of test being executed
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    if (argc != 6) {
        printf("Invalid number of arguments\n");
        exit(-1);
    }

    char *N = argv[1];
    int size = atoi(argv[3]);
    int number_of_producers = atoi(argv[2]);
    int number_of_consumers = atoi(argv[4]);

    char *type_of_test = argv[5];

    if (mkfifo("pipe", 0666) < 0) {
        perror("Problem while creating pipe\n");
        exit(-1);
    }
    time_t t;
    srand((unsigned) time(&t));

    for (unsigned int i = 0; i < number_of_producers; i++) {
        char randomLetter = (char) ('A' + (random() % 26));
        char file_name[50];
        sprintf(file_name, "res/%s/%d.txt", type_of_test, i);
        FILE *f = fopen(file_name, "w");
        char *line = calloc((size + 3), sizeof(char));
        for (int j = 0; j < size; j++) {
            line[j] = randomLetter;
        }
        fprintf(f, "%s", line);
        fclose(f);
        free(line);
    }
    for (int i = 0; i < number_of_producers; i++) {
        if (fork() == 0) {
            char buff[5];
            sprintf(buff, "%d", i);
            char file_name[50];
            sprintf(file_name, "res/%s/%d.txt", type_of_test, i);
            execl("./producer", "./producer", "pipe", buff, file_name, N, NULL);
            perror("There is a problem in producer\n");
            exit(-1);
        }
        sleep(1);
    }
    if (fork() == 0) {
        execlp("./consumer", "./consumer", "pipe", "output_consumer.txt", N, NULL);
        exit(-1);
    }
    wait(NULL);
    for (int i = 0; i < number_of_producers; i++) wait(NULL);

    return 0;

}
