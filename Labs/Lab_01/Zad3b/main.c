#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include "mylib.h"

int main(int argc, char *argv[])
{
#ifdef DLL
    //open library
    //function returns library handler
    //takes dynamic library path and a flag
    void *handle = dlopen("./libmylib.so", RTLD_LAZY);
    //now - pointers to all used functions
    //function dlsym takes library handler and function name, returns function pointer
    int (*commands_interpreter)(char **) = dlsym(handle, "commands_interpreter");
    void (*write_to_file_and_console)(char *, char *, clock_t[2], struct tms **) = dlsym(handle, "write_to_file_and_console");
#endif
    clock_t real_time[2];
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    for (int i = 0; i < 2; i++)
    {
        tms_time[i] = (struct tms *)malloc(sizeof(struct tms));
    }
    real_time[0] = times(tms_time[0]);
    commands_interpreter(argv + 1);
    real_time[1] = times(tms_time[1]);

#ifdef DLL
    write_to_file_and_console(argv[1], "results_3a_dll.txt", real_time, tms_time);
#endif
#ifdef SHARED
    write_to_file_and_console(argv[1], "results_3a_shared.txt", real_time, tms_time);
#endif
#ifdef STATIC
    write_to_file_and_console(argv[1], "results_3a_static.txt", real_time, tms_time);
#endif
    //    Free
    for (int i = 0; i < 2; i++)
    {
        free(tms_time[i]);
    }
    free(tms_time);
#ifdef DLL
    dlclose(handle);
#endif
}
