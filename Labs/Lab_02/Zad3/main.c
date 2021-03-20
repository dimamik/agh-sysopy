#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "string.h"

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN fopen(path, "a+")
#define M_OPEN_READ fopen(path, "r")
#define M_CLOSE fclose
#define M_WRITE fwrite(string, sizeof(char), strlen(string), ptr)
#define M_STDIN stdin
#define M_NULL NULL

#else

#include <zlib.h>
#include <fcntl.h>

#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN open(path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_READ open(path, O_RDONLY)
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

int ifNum(const char *string)
{
    int x = strspn(string, "0123456789\n\r") == strlen(string);
    if (x == 0)
    {
        printf("dane.txt Should consist only of numbers!");
    }
    return x;
}

void write_to_file(char *path, int number)
{
    char string[256];
    M_FILE ptr = M_OPEN;
    sprintf(string, "%d\n", number);
    M_WRITE;
    M_CLOSE(ptr);
}

int writeNumberToAppropriateFile(char *string)
{
    char *ptr;
    int number = (int)strtol(string, &ptr, 10);
    if ((number / 10 % 10 == 7 || number / 10 % 10 == 0) && number != 0)
    {
        write_to_file("b.txt", number);
    }
    else
    {
        int x = (int)sqrt(number);
        if (x * x == number && number != 0)
        {
            write_to_file("c.txt", number);
        }
    }

    return number % 2 == 0;
}

int main(int argc, char **argv)
{
    char path[256] = "dane.txt";
    int odd_number = 0;
    M_FILE dane = M_OPEN_READ;
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
    sprintf(string, "Liczb parzystych jest %d\n", odd_number);
    M_WRITE;
    M_CLOSE(ptr);
    M_CLOSE(dane);

    printf("%s\n", "All files are successfully written");
    return 0;
}
