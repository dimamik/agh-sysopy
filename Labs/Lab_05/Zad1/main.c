#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


int MAX_COMMANDS_PER_VAR = 15;
int MAX_WORDS_PER_COMMAND = 5;
int MAX_VARIABLES = 10;
/**
 * Complete command before |
 * list_of_commands - commands which were separated by space
 */
typedef struct {
    char **list_of_single_commands;
    int size_of_list;
} command_t;

void free_command(command_t *command) {
    for (int i = 0; i < command->size_of_list; ++i) {
        free(command->list_of_single_commands[i]);
    }
    free(command->list_of_single_commands);
    free(command);
}

/**
 * Variable that wraps the command
 */
typedef struct {
    char *var_name;
    command_t **list_of_commands;
    int size;
} var_command_t;

void free_var_command(var_command_t *varCommand) {
    free(varCommand->var_name);
    for (int i = 0; i < varCommand->size; ++i) {
        free_command(varCommand->list_of_commands[i]);
    }
    free(varCommand->list_of_commands);
    free(varCommand);
}

void free_var_command_list(var_command_t **varCommand, int size) {

    for (int i = 0; i < size; ++i) {
        free_var_command(varCommand[i]);
    }
    free(varCommand);
}

void remove_spaces(char *s) {
    const char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));
}

void create_command(char *line, command_t *command) {
    char *save;

    char *single_command = strtok_r(line, " ", &save);
    command->list_of_single_commands = malloc(sizeof(char *) * MAX_WORDS_PER_COMMAND);
    command->list_of_single_commands[0] = malloc(sizeof(char) * (strlen(single_command) + 1));
    strcpy(command->list_of_single_commands[0], single_command);
    int position = 1;
    while ((single_command = strtok_r(NULL, " ", &save)) != NULL) {
        command->list_of_single_commands[position] = calloc(strlen(single_command) + 1, sizeof(char));
        strcpy(command->list_of_single_commands[position++], single_command);
    }
    command->size_of_list = position;
}


var_command_t *extract_variable(char *line) {

    var_command_t *var_command = malloc(sizeof(var_command_t));
    var_command->list_of_commands = malloc(sizeof(command_t *) * MAX_COMMANDS_PER_VAR);

    char *save;
    char *name = strtok_r(line, "=", &save);
    char *command;
    int command_index = 0;

    while ((command = strtok_r(NULL, "|", &save)) != NULL) {

        var_command->list_of_commands[command_index] = malloc(sizeof(command_t));
        create_command(command, var_command->list_of_commands[command_index++]);
        var_command->size += 1;
    }
    name[strlen(name) - 1] = '\0';
    var_command->var_name = calloc(strlen(name), sizeof(char));
    strcpy(var_command->var_name, name);
    return var_command;

}

int find_command(var_command_t **varCommand, char *command_to_find, int size) {
    int i = -1;
    while (++i < size) {
        if (strcmp(varCommand[i]->var_name, command_to_find) == 0) {
            return i;
        }
    }
    return -1;

}


void connect_and_execute_commands(var_command_t **varCommand, char *commands_to_exec, int list_size) {
    if (strcmp(commands_to_exec, "") == 0) {
        return;
    }
    printf("------------------%s-------------------\n", commands_to_exec);

    int current[2];
    int prev[2];
    int first_time_there = 1;
    char *command;
    char *save;
    int current_index = -1;
    int command_number = 0;
    var_command_t *current_command;
    while ((command = strtok_r(commands_to_exec, "|", &save)) != NULL) {

        commands_to_exec = NULL;
        remove_spaces(command);

        int i = find_command(varCommand, command, list_size);
        if (i == -1) {
            exit(-128);
        }
        current_command = varCommand[i];
        current_index = -1;
        while (++current_index < current_command->size) {
            if (strcmp(save, "") == 0 && current_index == current_command->size - 1) {
                break;
            }
            pipe(current);
            printf("! am doing now: %s %s %s\n",
                   current_command->list_of_commands[current_index]->list_of_single_commands[0],
                   current_command->list_of_commands[current_index]->list_of_single_commands[1],
                   current_command->list_of_commands[current_index]->list_of_single_commands[2]);
            pid_t pid = fork();
            if (pid == 0) {

                if (first_time_there != 1) {
                    dup2(prev[0], STDIN_FILENO);
                    close(prev[1]);
                }

                dup2(current[1], STDOUT_FILENO);
                if (execvp(current_command->list_of_commands[current_index]->list_of_single_commands[0],
                           current_command->list_of_commands[current_index]->list_of_single_commands) == -1) {
                    exit(1);
                }
                command_number++;
            }

            first_time_there = 0;
            close(current[1]);

            prev[0] = current[0];
            prev[1] = current[1];

        }

    }
    printf("I am doing now: %s %s %s\n",
           current_command->list_of_commands[current_index]->list_of_single_commands[0],
           current_command->list_of_commands[current_index]->list_of_single_commands[1],
           current_command->list_of_commands[current_index]->list_of_single_commands[2]);
    pipe(current);
    pid_t pid = fork();

    if (pid == 0) {
        close(prev[1]);
        dup2(prev[0], STDIN_FILENO);

        if (execvp(current_command->list_of_commands[current_index]->list_of_single_commands[0],
                   current_command->list_of_commands[current_index]->list_of_single_commands) == -1) {
            exit(1);
        }
    }
    for (int j = 0; j < command_number + 3; j++) {
        wait(NULL);
    }

    printf("--------------------\n\n");

}


int main(int argc, char **argv) {
    if (argc != 2) {
        perror("Invalid arguments");
        exit(-1);
    }


    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Invalid file path");
        exit(-1);
    }
    char *line = NULL;
    size_t line_size = 0;

    int list_size = 0;
    int curr_var = 0;
    var_command_t **varCommand = malloc(sizeof(var_command_t *) * MAX_VARIABLES);


    while (getline(&line, &line_size, file) != -1) {
        line[strlen(line) - 1] = '\0';
        if (strchr(line, '=') != NULL) {
//            varCommand[curr_var] = malloc(sizeof(var_command_t));
            varCommand[curr_var++] = extract_variable(line);
            list_size++;
        } else {

            connect_and_execute_commands(varCommand, line, list_size);
        }

    }
    free_var_command_list(varCommand, curr_var);
    free(line);
}