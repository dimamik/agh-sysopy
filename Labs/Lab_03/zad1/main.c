#include <stdio.h>
#include <zlib.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        int status;
        int n = atoi(argv[1]);
        int x;
        for (int i = 0; i < n; ++i)
        {
            x = fork();
            wait(&status);
            if (x == 0)
            {
                printf("I'm a poor child process with pid: %d, number: %d\n", getpid(), i);
                exit(0);
            }
        }
    }
    else
    {
        exit(-1);
    }
}
