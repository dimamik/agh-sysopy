#include <stdio.h>
#include <stdlib.h>

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN_1 fopen(first_path, "r")
#define M_OPEN_2 fopen(second_path, "r")
#define M_CLOSE fclose
#define M_STDIN stdin
#define M_NULL NULL

#else
#include <zlib.h>
#include <fcntl.h>
#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN_1 open(first_path, O_RDONLY)
#define M_OPEN_2 open(second_path, O_RDONLY)
#define M_CLOSE close
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

int checkIfThereIsCharInString(char ch, const char *string)
{
    int i = -1;

    while (string[++i] != '\n')
    {
        if (string[i] == ch)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * Only using fread() i fwrite()
 * @param first_path
 * @param second_path
 */
void zad1_lib(char ch, char *first_path)
{
    if (first_path == NULL || ch == 0)
    {
        printf("Please, give normal char and path");
        exit(1);
    }
    M_FILE first = M_OPEN_1;
    if (first == M_NULL)
    {
        perror("Can't open file: ");
    }

    char *first_buffer = inputString(first, START_SIZE);

    while (first_buffer != NULL)
    {
        if (first_buffer != NULL)
        {
            if (checkIfThereIsCharInString(ch, first_buffer))
            {
                printf("%s", first_buffer);
            }
            free(first_buffer);
            first_buffer = inputString(first, 10);
        }
    }
    M_CLOSE(first);
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
    if (argc < 2){
        printf("You need to give 2 arguments");
        exit(1);
    }

    zad1_lib(argv[1][0], argv[2]);
    return 0;
}
