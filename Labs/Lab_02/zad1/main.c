#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN_1 fopen(first_path, "r")
#define M_OPEN_2 fopen(second_path, "r")
#define M_CLOSE fclose
#define M_STDIN stdin
#define M_NULL NULL

#else

#include <fcntl.h>

#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN_1 open(first_path, O_RDONLY)
#define M_OPEN_2 open(second_path, O_RDONLY)
#define M_CLOSE close
#define M_STDIN STDIN_FILENO
#define M_NULL 0
#endif

int START_SIZE = 10;

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

/**
 * Only using fread() i fwrite()
 * @param first_path
 * @param second_path
 */
void zad1(char *first_path, char *second_path)
{
    if (first_path == NULL || second_path == NULL)
    {
        printf("\n\nPlease, give me file path to two files you want to print\n");
        first_path = inputString(M_STDIN, START_SIZE);
        second_path = inputString(M_STDIN, START_SIZE);
    }
    M_FILE first = M_OPEN_1;
    M_FILE second = M_OPEN_2;
    if (first == M_NULL || second == M_NULL)
    {

        perror("Can't open files: ");
        exit(-1);
    }
    char *first_buffer = inputString(first, START_SIZE);
    char *second_buffer = inputString(second, START_SIZE);

    while (first_buffer != NULL || second_buffer != NULL)
    {
        if (first_buffer != NULL)
        {
            printf("%s", first_buffer);
            free(first_buffer);
            first_buffer = inputString(first, 10);
        }
        if (second_buffer != NULL)
        {
            printf("%s", second_buffer);
            free(second_buffer);
            second_buffer = inputString(second, 10);
        }
    }
    M_CLOSE(first);
    M_CLOSE(second);
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

/**
 * ../1.txt
 * ../2.txt
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char **argv)
{
    if (argc < 1)
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
    zad1(argv[2], argv[3]);
    real_time[1] = times(tms_time[1]);

    write_to_file_and_console(argv[1], "pomiar_zad_1.txt", real_time, tms_time);

    for (int i = 0; i < 2; i++)
    {
        free(tms_time[i]);
    }
    free(tms_time);
    return 0;
}
