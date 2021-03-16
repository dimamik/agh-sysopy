#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

Wrapped_block_of_lines *init_wrapped_block_of_lines()
{
    Wrapped_block_of_lines *wrappedBlockOfLines = calloc(1, sizeof(Wrapped_block_of_lines));
    return wrappedBlockOfLines;
}

Main_Table *init_main_table(int size_of_table)
{
    Main_Table *mainTable = calloc(size_of_table, sizeof(Main_Table));
    mainTable->pWrappedBlockOfLines = calloc(size_of_table, sizeof(Wrapped_block_of_lines));
    mainTable->size_init = size_of_table;
    mainTable->size_real = 0;
    mainTable->size_max_taken = 0;
    return mainTable;
}

void free_files_pair(Files_pair *filesPair)
{
    free(filesPair->first_file_path);
    free(filesPair->second_file_path);
    fclose(filesPair->temp_file);
    free(filesPair);
}

void free_block(Wrapped_block_of_lines *wrappedBlockOfLines)
{
    if (wrappedBlockOfLines == NULL)
    {
        return;
    }
    int j = -1;
    while (++j < wrappedBlockOfLines->size_max_taken)
    {
        if (wrappedBlockOfLines->list_of_rows[j] != NULL)
            free(wrappedBlockOfLines->list_of_rows[j]);
    }
    free(wrappedBlockOfLines->list_of_rows);
    free(wrappedBlockOfLines);
}

void free_main_table(Main_Table *mainTable)
{

    int i = -1;
    while (++i < mainTable->size_max_taken)
    {
        free_block(mainTable->pWrappedBlockOfLines[i]);
    }
    free(mainTable->pWrappedBlockOfLines);
    free(mainTable);
}

/**
 * Adding \r\n at the end of line in case of bad formatted input
 * @param s
 * @return
 */
char *add_new_line(char *s)
{
    char *prev_s = calloc(strlen(s) + 1, sizeof(char));
    strcpy(prev_s, s);
    size_t size = strlen(s);
    char ch = s[size - 1];
    if (ch != '\n')
        s = calloc(strlen(prev_s) + 2, sizeof(char));
    else
    {
        free(prev_s);
        return s;
    }
    strcpy(s, prev_s);
    s[strlen(prev_s)] = '\r';
    s[strlen(prev_s) + 1] = '\n';
    free(prev_s);
    return s;
}

void merge_two_files(Files_pair *filesPair)
{

    FILE *first_file = fopen(filesPair->first_file_path, "r");
    FILE *second_file = fopen(filesPair->second_file_path, "r");
    if (first_file == NULL || second_file == NULL)
    {
        return;
    }
    filesPair->temp_file = tmpfile();
    char *line1 = NULL;
    char *line2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;
    ssize_t r1 = (getline(&line1, &len1, first_file));
    ssize_t r2 = (getline(&line2, &len2, second_file));
    while (r1 != -1 || r2 != -1)
    {
        if (r1 != -1 && line1 != NULL && strcmp(line1, "\r\n") != 0)
        {
            line1 = add_new_line(line1);
            fprintf(filesPair->temp_file, "%s", line1);
        }
        if (r2 != -1 && line2 != NULL && strcmp(line1, "\r\n") != 0)
        {
            line2 = add_new_line(line2);
            fprintf(filesPair->temp_file, "%s", add_new_line(line2));
        }
        (r1 = (getline(&line1, &len1, first_file)));
        (r2 = (getline(&line2, &len2, second_file)));
    }
    fclose(first_file);
    fclose(second_file);
    free(line1);
    free(line2);
}

int count_lines_merged_file(FILE *file)
{
    char *line = NULL;
    int line_count = 0;
    size_t len = 0;
    rewind(file);
    ssize_t r = (getline(&line, &len, file));
    while (r != -1)
    {
        line_count += 1;
        r = (getline(&line, &len, file));
    }
    free(line);
    line = NULL;
    return line_count;
}

char **convert_file_to_string_list(FILE *file, int file_size)
{
    char *line = NULL;
    char **string_list = calloc(file_size, sizeof(char *));
    int i = -1;
    size_t len = 0;
    rewind(file);
    while ((getline(&line, &len, file)) != -1)
    {
        string_list[++i] = calloc(len, sizeof(char));
        strcpy(string_list[i], line);
    }
    free(line);
    return string_list;
}

Files_pair *group_files(char *file1, char *file2)
{
    Files_pair *filesPair = calloc(1, sizeof(*filesPair));
    filesPair->first_file_path = calloc(strlen(file1) + 1, sizeof(char));
    filesPair->second_file_path = calloc(strlen(file2) + 1, sizeof(char));
    strcpy(filesPair->first_file_path, file1);
    strcpy(filesPair->second_file_path, file2);
    merge_two_files(filesPair);
    filesPair->size_lines = count_lines_merged_file(filesPair->temp_file);

    return filesPair;
}

void delete_row_from_block(Main_Table *mainTable, int block_number, int row_number)
{
    if (block_number >= mainTable->size_max_taken ||
        mainTable->pWrappedBlockOfLines[block_number]->size_max_taken < row_number ||
        mainTable->pWrappedBlockOfLines[block_number]->list_of_rows[row_number] == NULL)
    {
        printf("CAN'T DELETE BLOCK'S ROW!\n");
        return;
    }
    mainTable->pWrappedBlockOfLines[block_number]->size_real--;
    free(mainTable->pWrappedBlockOfLines[block_number]->list_of_rows[row_number]);
    mainTable->pWrappedBlockOfLines[block_number]->list_of_rows[row_number] = NULL;
}

/**
 * Takes not merged files and adds their version to Main Table
 * @param filesPair
 */
void add_new_row_to_main_table(char *file1, char *file2, Main_Table *mainTable)
{
    if (mainTable->size_init <= mainTable->size_max_taken)
    {
        printf("Can't add block to main_table\n");
        return;
    }
    Files_pair *filesPair = group_files(file1, file2);
    Wrapped_block_of_lines *wrappedBlockOfLines = init_wrapped_block_of_lines();
    wrappedBlockOfLines->size_max_taken = filesPair->size_lines;
    wrappedBlockOfLines->size_real = filesPair->size_lines;
    wrappedBlockOfLines->list_of_rows = convert_file_to_string_list(filesPair->temp_file, filesPair->size_lines);
    mainTable->pWrappedBlockOfLines[mainTable->size_real] = wrappedBlockOfLines;
    mainTable->pWrappedBlockOfLines[mainTable->size_real]->size_real = filesPair->size_lines;
    mainTable->size_real++;
    mainTable->size_max_taken++;
    free_files_pair(filesPair);
}

void delete_block_from_main_table(Main_Table *mainTable, int block_number)
{
    if (mainTable->size_max_taken <= block_number)
    {
        printf("CAN'T DELETE BLOCK!\n");
        return;
    }
    free_block(mainTable->pWrappedBlockOfLines[block_number]);
    mainTable->pWrappedBlockOfLines[block_number] = NULL;
    mainTable->size_real--;
}

void print_block(Wrapped_block_of_lines *wrappedBlockOfLines)
{
    if (wrappedBlockOfLines != NULL)
    {
        int j = -1;
        while (wrappedBlockOfLines->size_max_taken > ++j)
        {
            if (wrappedBlockOfLines->list_of_rows[j] != NULL)
                printf("%s", wrappedBlockOfLines->list_of_rows[j]);
        }
        printf("\n");
    }
}

void printMainTable(Main_Table *mainTable)
{
    int i = -1;
    while (++i < mainTable->size_max_taken)
    {
        print_block(mainTable->pWrappedBlockOfLines[i]);
    }
    printf("Parameters of the table are: size_real: "
           "%d\tsize_init: %d\tsize_max_taken: %d\n\n",
           mainTable->size_real, mainTable->size_init, mainTable->size_max_taken);
}

int is_string_right_file_sequence(const char *string)
{
    int i = -1;
    while (string[++i] != ' ' && i < strlen(string))
    {
        if (string[i] == ':')
        {
            return i;
        }
    }
    return 0;
}

int merge_files_command(char **argv, int i, Main_Table *mainTable)
{
    if (argv[i + 1] == NULL)
    {
        printf("INVALID ARGUMENTS");
        return -1;
    }
    char **files_list = calloc(2, sizeof(char *));

    files_list[0] = strsep(&argv[i + 1], ":");
    files_list[1] = strsep(&argv[i + 1], ":");

    add_new_row_to_main_table(files_list[0], files_list[1], mainTable);
    free(files_list);
    i++;
    return i;
}

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