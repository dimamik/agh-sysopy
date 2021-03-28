#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

int commands_interpreter(char *argv[])
{
    int i = 0;
    Main_Table *mainTable = NULL;
    char *ptr;
    while (argv[++i] != NULL)
    {

        if (strcmp(argv[i], "create_table") == 0 && argv[i + 1] != NULL)
        {
            if (argv[i + 1] == NULL)
            {
                printf("INVALID ARGUMENTS");
                exit(1);
            }
            int size = (int)strtol(argv[i + 1], &ptr, 10);
            mainTable = init_main_table(size);
            i++;
        }
        else if (strcmp(argv[i], "merge_files") == 0)
        {
            if (mainTable == NULL)
            {
                printf("TABLE SHOULD BE INITIALIZED FIRST");
                exit(1);
            }

            while (argv[i + 1] != NULL && is_string_right_file_sequence(argv[i + 1]) != 0)
            {
                merge_files_command(argv, i, mainTable);
                i++;
            }
        }
        else if (strcmp(argv[i], "remove_block") == 0)
        {
            if (argv[i + 1] == NULL)
            {
                printf("INVALID ARGUMENTS");
                return -1;
            }
            int block_index = (int)strtol(argv[i + 1], &ptr, 10);
            delete_block_from_main_table(mainTable, block_index);
            i++;
        }
        else if (strcmp(argv[i], "remove_row") == 0)
        {
            if (argv[i + 1] == NULL || argv[i + 2] == NULL)
            {
                printf("INVALID ARGUMENTS");
                return -1;
            }
            int block_index = (int)strtol(argv[i + 1], &ptr, 10);
            int row_index = (int)strtol(argv[i + 2], &ptr, 10);
            delete_row_from_block(mainTable, block_index, row_index);
            i += 2;
        }
        else if (strcmp(argv[i], "rows_in_block") == 0)
        {
            if (argv[i + 1] == NULL ||
                mainTable->pWrappedBlockOfLines[(int)strtol(argv[i + 1], &ptr, 10)] == NULL)
            {
                printf("INVALID ARGUMENTS");
                return -1;
            }
            int block_index = (int)strtol(argv[i + 1], &ptr, 10);
            int size = mainTable->pWrappedBlockOfLines[block_index]->size_real;
            printf("There are %d rows in %d block\n", size,
                   (int)strtol(argv[i + 1], &ptr, 10));
            i++;
        }
    }
    if (mainTable != NULL)
    {
        //        printMainTable(mainTable);
        free_main_table(mainTable);
    }
    return 0;
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
int merge_files_command(char **argv, int i, Main_Table *mainTable)
{
    if (argv[i + 1] == NULL)
    {
        printf("INVALID ARGUMENTS");
        return -1;
    }
    int x;
    char **files_list = calloc(2, sizeof(char *));

    files_list[0] = strsep(&argv[i + 1], ":");
    files_list[1] = strsep(&argv[i + 1], ":");

    x = fork();
    if (x == 0)
    {
        add_new_row_to_main_table(files_list[0], files_list[1], mainTable);
        exit(0);
    }

    free(files_list);
    i++;
    return i;
}

int main(int argc, char *argv[])
{

    clock_t real_time[2];
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    for (int i = 0; i < 2; i++)
    {
        tms_time[i] = (struct tms *)malloc(sizeof(struct tms));
    }
    real_time[0] = times(tms_time[0]);
    commands_interpreter(argv + 1);
    while (wait() > 0){
    }
    real_time[1] = times(tms_time[1]);
    write_to_file_and_console(argv[1], "zad2_report.txt", real_time, tms_time);
    //    Free
    for (int i = 0; i < 2; i++)
    {
        free(tms_time[i]);
    }
    free(tms_time);
}
