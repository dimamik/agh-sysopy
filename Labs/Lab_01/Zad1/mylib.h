#ifndef ZAD_1_MYLIB_H
#define ZAD_1_MYLIB_H

#include <bits/types/FILE.h>

typedef struct Files_pair {
    char *first_file_path;
    char *second_file_path;
    FILE *temp_file;
    int size_lines;
} Files_pair;

/**
 * list_of_rows is a table of (char*) representing lines of merged files
 * size real is an amount of occupied (taken) lines
 * size_max_taken is size where last element was added + 1
 */
typedef struct Wrapped_block_of_lines {
    char **list_of_rows;
    int size_real;
    int size_max_taken;
} Wrapped_block_of_lines;

/**
 * size_real is the taken size
 * size_init is size when initialized
 * size_max_taken is size where last element was added (should check to be less then initialized
 * , else throw error) + 1
 */
typedef struct Main_Table {
    Wrapped_block_of_lines **pWrappedBlockOfLines;
    int size_real;
    int size_init;
    int size_max_taken;
} Main_Table;

Main_Table *init_main_table(int size_of_table);

Wrapped_block_of_lines *init_wrapped_block_of_lines();

void free_files_pair(Files_pair *filesPair);

void free_block(Wrapped_block_of_lines *wrappedBlockOfLines);

void free_main_table(Main_Table *mainTable);

/**
 * Adding \r\n at the end of line in case of bad formatted input
 */
char *add_new_line(char *s);

void merge_two_files(Files_pair *filesPair);

int count_lines_merged_file(FILE *file);

char **convert_file_to_string_list(FILE *file, int file_size);

Files_pair *group_files(char *file1, char *file2);

void delete_row_from_block(Main_Table *mainTable, int block_number, int row_number);

/**
 * Takes not merged files and adds their version to Main Table
 * @param filesPair
 */
void add_new_row_to_main_table(char *file1, char *file2, Main_Table *mainTable);

void delete_block_from_main_table(Main_Table *mainTable, int block_number);

void print_block(Wrapped_block_of_lines *wrappedBlockOfLines);

void printMainTable(Main_Table *mainTable);

int is_string_right_file_sequence(const char *string);

int merge_files_command(char **argv, int i, Main_Table *mainTable);

#endif //ZAD_1_MYLIB_H
