#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"
#include <unistd.h>
#include <sys/times.h>

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN fopen(path, "a+")
#define M_OPEN_B fopen(path_b, "a+")
#define M_OPEN_C fopen(path_c, "a+")
#define M_OPEN_READ fopen(path, "r")
#define M_CLOSE fclose
#define M_WRITE fwrite(string, sizeof(char), strlen(string), ptr)
#define M_WRITE_B fwrite(string, sizeof(char), strlen(string), ptr_b)
#define M_WRITE_C fwrite(string, sizeof(char), strlen(string), ptr_c)
#define M_STDIN stdin
#define M_NULL NULL

#else

#include <fcntl.h>

#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_B open(path_b, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_C open(path_c, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_READ open(path, O_RDONLY)
#define M_CLOSE close
#define M_WRITE write(ptr, string, strlen(string))
#define M_WRITE_B write(ptr_b, string, strlen(string))
#define M_WRITE_C write(ptr_c, string, strlen(string))
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

int ifNum(const char *string)
{
    int x = strspn(string, "0123456789\n\r") == strlen(string);
    if (x == 0)
    {
        printf("dane.txt Should consist only of numbers!");
    }
    return x;
}

void write_to_file(M_FILE ptr, int number)
{
    char string[256];
    sprintf(string, "%d\n", number);
    M_WRITE;
}

int writeNumberToAppropriateFile(char *string)
{
    char *ptr;
    int number = (int)strtol(string, &ptr, 10);
    char path_b[] = "b.txt";
    char path_c[] = "c.txt";
    M_FILE ptr_b = M_OPEN_B;
    M_FILE ptr_c = M_OPEN_C;

    if ((number / 10 % 10 == 7 || number / 10 % 10 == 0) && number != 0)
    {
        write_to_file(ptr_b, number);
    }
    else
    {
        int x = (int)sqrt(number);
        if (x * x == number && number != 0)
        {
            write_to_file(ptr_c, number);
        }
    }

    M_CLOSE(ptr_b);
    M_CLOSE(ptr_c);
    return number % 2 == 0;
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
    if (argc < 2)
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

    char path[256] = "dane.txt";
    int odd_number = 0;
    M_FILE dane = M_OPEN_READ;
    if (dane == M_NULL)
    {
        printf("%s", "There is no file dane.txt at given path");
        exit(-1);
    }
    char *first_buffer = inputString(dane, START_SIZE);
    while (first_buffer != NULL)
    {
        if (ifNum(first_buffer))
        {
            odd_number += writeNumberToAppropriateFile(first_buffer);
        }
        free(first_buffer);
        first_buffer = inputString(dane, 10);
    }

    strcpy(path, "a.txt");
    char string[256];
    M_FILE ptr = M_OPEN;
    sprintf(string, "Liczb parzystych jest: %d\n", odd_number);
    M_WRITE;
    M_CLOSE(ptr);
    M_CLOSE(dane);

    printf("%s\n", "All files are successfully written");

    real_time[1] = times(tms_time[1]);
    write_to_file_and_console(argv[1], "pomiar_zad_3.txt", real_time, tms_time);

    for (int i = 0; i < 2; i++)
    {
        free(tms_time[i]);
    }
    free(tms_time);
    return 0;
}
