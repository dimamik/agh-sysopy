//
// Created by dzmitry on 5/18/21.
//

#ifndef LAB_8_PGMA_H
#define LAB_8_PGMA_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <ctype.h>

#define NUMBERS 0
#define BLOCK 1

typedef struct {
    short int **matrix;
    int width;
    int height;
    short int max_gray;
} pgm_t;

pgm_t *read_image(char *file_path) {

    char version[3];
    FILE *pgmFile;
    pgmFile = fopen(file_path, "rb");
    if (pgmFile == NULL) {
        perror("cannot open file to read");
        exit(EXIT_FAILURE);
    }
    fgets(version, sizeof(version), pgmFile);
    if (strcmp(version, "P2") != 0) {
        fprintf(stderr, "Wrong file type: %s\n", version);
        exit(EXIT_FAILURE);
    }
    pgm_t *image = calloc(1, sizeof(pgm_t));

    fscanf(pgmFile, "%d %d\n %hd", &image->width, &image->height, &image->max_gray);
    image->matrix = (short int **) malloc(image->height * sizeof(short int *));
    for (int i = 0; i < image->height; i++)
        image->matrix[i] = (short int *) malloc(image->width * sizeof(short int));
    fgetc(pgmFile);

    for (int i = 0; i < image->width; ++i) {
        for (int j = 0; j < image->height; ++j) {
            fscanf(pgmFile, "%hd", &image->matrix[i][j]);
        }
    }

    fclose(pgmFile);

    return image;

}

void write_image(char* file_path, pgm_t* image, int new_line_at){

    FILE *pgmFile;
    pgmFile = fopen(file_path, "w");
    if (pgmFile == NULL) {
        perror("cannot open file to read");
        exit(EXIT_FAILURE);
    }
    fputs("P2\n",pgmFile);

    fprintf(pgmFile, "%d %d\n %hd\n", image->width, image->height, image->max_gray);
    int numbers_on_line = 0;
    for (int i = 0; i < image->width; ++i) {
        for (int j = 0; j < image->height; ++j) {
            if (numbers_on_line > new_line_at){
                fputc('\n',pgmFile);
                numbers_on_line = 0;
            }else{
                fputc(' ',pgmFile);
            }
            fprintf(pgmFile, "%hd", image->matrix[i][j]);
            numbers_on_line++;
        }
    }

    fclose(pgmFile);
}

void free_image(pgm_t* image){
    for (int i = 0; i < image->height; i++)
        free(image->matrix[i]);
    free(image->matrix);
    free(image);
}


#endif //LAB_8_PGMA_H
