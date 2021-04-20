#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>

/**
 * 4 arguments: path_to_pipe, row_number, path_to_txt, N - number of chars to read at a time
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc == 5) {
        char *path_to_pipe = argv[1];
        int row_number = atoi(argv[2]);
        char *path_to_txt = argv[3];
        int N = atoi(argv[4]);


        int pipe_file = open(path_to_pipe, O_WRONLY);
        if (pipe_file == -1) {
            perror("Can't open pipe\n");
            exit(-1);
        }

        int text_file = open(path_to_txt, O_RDONLY);
        if (text_file == -1) {
            perror("Can't open pipe\n");
            exit(-1);
        }
        ssize_t read_symbols = 0;
        char *buff = calloc(N + 1, sizeof(char));
        while ((read_symbols = read(text_file, buff, sizeof(char) * N)) > 0) {
            sleep(1);
            buff[read_symbols] = '\0';
            char temp_buff[read_symbols + 3];
//            Adding to pipe
            sprintf(temp_buff, "%d|%s", row_number, buff);
            if (flock(pipe_file, LOCK_EX) == -1) {
                exit(-1);
            }
            write(pipe_file, temp_buff, strlen(temp_buff));
            if (flock(pipe_file, LOCK_UN) == -1) {
                exit(-1);
            }

        }
        free(buff);
        close(text_file);

    } else {
        perror("Invalid number of arguments\n");
        exit(-1);
    }
}
