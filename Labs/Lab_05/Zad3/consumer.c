
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>


void override_file_with_modified_line(FILE *file, int row, char *string_to_write, char *file_path) {

    rewind(file);

    long int offset = 0;
    int n_lines = 0;
    char c;
    while ((c = (char) fgetc(file)) != EOF || n_lines < row) {
        if (c == EOF) {
            fputc('\n', file);
            n_lines++;
        } else if (c == '\n' && n_lines < row) {
            n_lines++;
        } else if (n_lines == row && c == '\n') {
            break;
        } else {

        }
        offset++;
    }
//   Copy the rest of file
    fseek(file, offset, SEEK_SET);
    char *buf = malloc(32 * sizeof(char));
    size_t length = 0;
    char n_c;
    size_t size = 32;
    while ((n_c = (char) fgetc(file)) != EOF) {
        if (length == size) {
            size *= 2;
            buf = realloc(buf, size);
            if (buf == NULL) {
                exit(1);
            }
        }
        buf[length++] = n_c;
    }
    buf[length] = '\0';
    rewind(file);
    fseek(file, offset, SEEK_SET);
    fwrite(string_to_write, sizeof(char), strlen(string_to_write), file);
    fwrite(buf, sizeof(char), strlen(buf), file);
}


/**
 * Takes 3 args:
 *  path_to_pipe, path_to_out, N - number of chars to read
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    if (argc == 4) {

        int pip_file = open(argv[1], O_RDONLY);
        if (pip_file == -1) {
            exit(-1);
        }

        FILE *text_file = fopen(argv[2], "r+");
        if (text_file == NULL) {
            exit(-1);
        }
        int N = atoi(argv[3]);
        char *buff = calloc(N + 3, sizeof(char));
        char *to_text_file;

        int text_file_descriptor = fileno(text_file);

        int row_id;
        ssize_t chars_read;
        while ((chars_read = read(pip_file, buff, (N + 2) * sizeof(char))) > 0) {
            buff[chars_read] = '\0';
            row_id = atoi(&buff[0]);
            to_text_file = buff + 2;
            if (flock(text_file_descriptor, LOCK_EX) < 0) exit(-1);

            override_file_with_modified_line(text_file, row_id, to_text_file, argv[2]);

            if (flock(text_file_descriptor, LOCK_UN) < 0) exit(-1);
        }
        free(buff);
        fclose(text_file);
        return 0;


    } else {
        perror("Invalid number of arguments\n");
        exit(-1);
    }
}




