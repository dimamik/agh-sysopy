#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"
#include <unistd.h>
#include <sys/times.h>

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN fopen(write_path, "a+")
#define M_OPEN_READ fopen(read_path, "r")
#define M_CLOSE fclose
#define M_WRITE fwrite(string, sizeof(char), strlen(string), ptr)
#define M_STDIN stdin
#define M_NULL NULL

#else

#include <zlib.h>
#include <fcntl.h>

#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN open(write_path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_READ open(read_path, O_RDONLY)
#define M_CLOSE close
#define M_WRITE write(ptr, string, strlen(string))
#define M_STDIN STDIN_FILENO
#define M_NULL 0
#endif

int START_SIZE = 256;

/**
 * Reads string from fp and returns address to allocated memory
 * @param fp
 * @param start_size
 * @return
 */
char *inputString(M_FILE fp, size_t start_size)
{
    char *str;
    int ch = 0;
    u_long k = 0;
    size_t len = 0;
    str = realloc(NULL, sizeof(*str) * start_size);
    if (!str)
        return str;

    while (0 != (k = M_READ) && ch != '\n')
    {
        str[len++] = (char)ch;
        if (len == start_size - 2)
        {
            str = realloc(str, sizeof(*str) * (start_size += 16));
            if (!str)
                return str;
        }
    }
    if (k == 0 && len == 0)
    {
        free(str);
        return NULL;
    }
    str[len++] = '\n';
    str[len++] = '\0';
    return realloc(str, sizeof(*str) * len);
}

char *replaceStringIfNeeded(char *line, char *first_replace, char *second_to_replace)
{
    if (strcmp(line, first_replace) == 0)
    {
        return second_to_replace;
    }
    else
    {
        return line;
    }
}

char *addEndLineCharToString(char *s)
{
    char *prev_s = calloc(strlen(s) + 1, sizeof(char));
    strcpy(prev_s, s);
    s = calloc(strlen(prev_s) + 3, sizeof(char));
    strcpy(s, prev_s);
    s[strlen(prev_s)] = '\r';
    s[strlen(prev_s) + 1] = '\n';
    free(prev_s);
    return s;
}

void zad4(char *read_path, char *write_path, char *first_replace, char *second_to_replace)
{

    first_replace = addEndLineCharToString(first_replace);
    second_to_replace = addEndLineCharToString(second_to_replace);

    M_FILE dane = M_OPEN_READ;
    if (dane == M_NULL)
    {
        printf("%s", "There is no file dane.txt at given path");
        exit(-1);
    }
    char *first_buffer = inputString(dane, START_SIZE);
    M_FILE ptr = M_OPEN;
    while (first_buffer != NULL)
    {

        char *string = replaceStringIfNeeded(first_buffer, first_replace, second_to_replace);
        M_WRITE;

        free(first_buffer);
        first_buffer = inputString(dane, 10);
    }
    M_CLOSE(ptr);
    free(first_replace);
    free(second_to_replace);
}
double subtract_time(clock_t start, clock_t end)
{
    return (double)(end - start) / (double)sysconf(_SC_CLK_TCK);
}

void write_to_file_and_console(char *procedure_name, char *file_name, clock_t real_time[2], struct tms **tms_time)
{

    FILE *ptr = fopen(file_name, "ab");

    fprintf(ptr, "Operation: %s\n", procedure_name);
    fprintf(ptr, "REAL: %lf   ", subtract_time(real_time[0], real_time[1]));
    fprintf(ptr, "USER CPU: %lf   ", subtract_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    fprintf(ptr, "SYSTEM CPU: %lf ", subtract_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
    fprintf(ptr, "\n\n");

    printf("Operation: %s\n", procedure_name);
    printf("REAL: %lf   ", subtract_time(real_time[0], real_time[1]));
    printf("USER CPU: %lf   ", subtract_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    printf("SYSTEM CPU: %lf ", subtract_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
    printf("\n\n");

    fclose(ptr);
}
int main(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("%s", "too less arguments!");
        exit(-1);
    }

    clock_t real_time[2];
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    for (int i = 0; i < 2; i++)
    {
        tms_time[i] = (struct tms *)malloc(sizeof(struct tms));
    }
    real_time[0] = times(tms_time[0]);

    zad4(argv[2], argv[3], argv[4], argv[5]);
    real_time[1] = times(tms_time[1]);

    write_to_file_and_console(argv[1], "pomiar_zad_4.txt", real_time, tms_time);

    for (int i = 0; i < 2; i++)
    {
        free(tms_time[i]);
    }
    free(tms_time);
    return 0;
}
