#include <stdio.h>
#include <zlib.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN fopen(write_path, "a+")
#define M_OPEN_READ fopen(read_path, "r")
#define M_CLOSE fclose
#define M_WRITE fwrite(string, sizeof(char), strlen(string), ptr)
#define M_STDIN stdin
#define M_NULL NULL

char *freeBuffer(char *string, size_t size)
{
    free(string);
    string = calloc(size, sizeof(char));
    return string;
}

int isStringPresentInFile(char *read_path, char *string_to_find)
{

    M_FILE fp = M_OPEN_READ;
    size_t word_len = strlen(string_to_find);
    char *buffer = calloc(strlen(string_to_find) + 1, sizeof(char));
    char ch;
    int is_buffer_free = 1;
    int i = -1;
    if (fp == M_NULL)
    {
        printf("There is no file at given path: %s", read_path);
        exit(-1);
    }

    while (0 != (M_READ))
    {
        if (is_buffer_free)
        {
            if (ch == string_to_find[0])
            {
                buffer[++i] = (char)ch;
                is_buffer_free = 0;
                continue;
            }
        }
        else
        {
            if (word_len == i + 1)
            {
                return 1;
            }

            if (buffer[i] != string_to_find[i])
            {
                buffer = freeBuffer(buffer, strlen(string_to_find) + 1);
                i = -1;
                is_buffer_free = 1;
                continue;
            }
            else
            {
                buffer[++i] = (char)ch;
            }
        }
    }

    M_CLOSE(fp);
    free(buffer);
    return 0;
}

char *concatenatePaths(const char *first_main, const char *second_to_add)
{
    char *pathname = calloc(strlen(first_main) + strlen(second_to_add) + 2, sizeof(char));
    sprintf(pathname, "%s/%s", first_main, second_to_add);
    return pathname;
}

FILE *openfile(const char *dirname, struct dirent *dir, const char *mode)
{
    FILE *fp;
    fp = fopen(concatenatePaths(dirname, dir->d_name), mode);
    return fp;
}

/**
 * @return 1 if file is txt, 2 if file is dir, -1 in other cases
 */
int checkIfFileIsTxtOrDir(char *name)
{
    if (name == NULL || strlen(name) < 4)
    {
        return -1;
    }
    if (name[strlen(name) - 1] == 't' && name[strlen(name) - 2] == 'x' && name[strlen(name) - 3] == 't' && name[strlen(name) - 4] == '.')
    {
        return 1;
    }
    int i = -1;
    while (name[++i] != '\0')
    {
        if (name[i] == '.')
        {
            return -1;
        }
    }
    return 2;
}

int main(int argc, char *argv[])
{
    if (argc == 4)
    {
        struct dirent *dirEnt;
        DIR *main_directory = opendir(argv[1]);
        int current_depth = 0;
        char *full_path = calloc(strlen(argv[1]) + 1, sizeof(char));
        strcpy(full_path, argv[1]);
        while ((dirEnt = readdir(main_directory)) != NULL)
        {
            int res = checkIfFileIsTxtOrDir(dirEnt->d_name);
            //            printf("Name: %s, type: %d\n", dirEnt->d_name, res);
            if (res == 2)
            {
                int x = fork();
                if (x == 0 && current_depth < atoi(argv[3]))
                {
                    //                    printf("Curr depth = %d\n",current_depth);
                    main_directory = opendir(concatenatePaths(full_path, dirEnt->d_name));
                    full_path = realloc(full_path, strlen(full_path) + 2 + strlen(dirEnt->d_name));
                    strcpy(full_path, concatenatePaths(full_path, dirEnt->d_name));
                    current_depth++;
                }
            }
            else if (res == 1)
            {
                int is_string = isStringPresentInFile(concatenatePaths(full_path, dirEnt->d_name), argv[2]);
                if (is_string)
                {
                    printf("Pattern found at %s/%s with the great help of process with PID: %d\n", full_path,
                           dirEnt->d_name, getpid());
                }
            }
        }
    }
    else
    {
        printf("Invalid number of parameters");
        exit(-1);
    }
    while (wait(NULL) > 0)
    {
    }
    return 0;
}
