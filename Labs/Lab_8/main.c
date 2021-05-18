
#include <pthread.h>
#include <stdio.h>
#include "pgma.h"
#include "math.h"


pgm_t *image_in;
pgm_t *image_out;
int n_threads;
int mode;

typedef struct {
    int index;
    int number_start;
    int number_finish;
} thread_info_t;

void init_threads_info(thread_info_t threadInfo[n_threads], int range) {
    for (int i = 0; i < n_threads; ++i) {
        threadInfo[i].index = i;
        threadInfo[i].number_start = (int) (i * ceil((double) range / n_threads));
        threadInfo[i].number_finish = (int) ((i + 1) * ceil((double) range / n_threads)) - 1;
    }
    if (mode == NUMBERS) {
        threadInfo[n_threads - 1].number_finish += 1;
    }
}

void print_threads_info(thread_info_t threadInfo[n_threads]) {
    for (int i = 0; i < n_threads; ++i) {
        printf("index: %d n_start: %d n_finish: %d\n",
               threadInfo[i].index,
               threadInfo[i].number_start,
               threadInfo[i].number_finish
        );
    }
}

typedef struct {
    unsigned long time;

} response_t;

void *single_thread_run(void *arg) {

    thread_info_t *threadInfo = (thread_info_t *) arg;

    long unsigned time;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    if (mode == BLOCK) {
        for (int i = 0; i < image_in->height; ++i) {
            for (int j = threadInfo->number_start; j <= threadInfo->number_finish; ++j) {
                image_out->matrix[j][i] = (short int) (image_out->max_gray - image_in->matrix[j][i]);
            }
        }

    } else {
        for (int i = 0; i < image_in->width; ++i) {
            for (int j = 0; j < image_in->height; ++j) {
                if (image_in->matrix[i][j] <= threadInfo->number_finish &&
                    image_in->matrix[i][j] >= threadInfo->number_start
                        )
                    image_out->matrix[i][j] = (short int) (image_out->max_gray - image_in->matrix[i][j]);
            }
        }

    }
    gettimeofday(&end_time, NULL);
    time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec;
    response_t *response = malloc(sizeof(response_t));
    response->time = time;

    pthread_exit(response);
}


void run_threads(pthread_t *threads, thread_info_t threads_info[n_threads],
                 response_t *response[n_threads]) {


    for (int i = 0; i < n_threads; ++i) {
        pthread_create(&threads[i], NULL, single_thread_run, &threads_info[i]);
    }

    for (int i = 0; i < n_threads; ++i) {
        pthread_join(threads[i], (void **) &response[i]);
    }

    for (int i = 0; i < n_threads; ++i) {
        printf("%lu\n", response[i]->time);
    }
}

void write_results_to_file(response_t *response[n_threads], long unsigned main_time) {
    FILE *file;
    file = fopen("wnioski.txt", "a");
    if (file == NULL) {
        perror("cannot open file to read");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "--------Run with %d threads--------\n", n_threads);
    fprintf(file, "Main thread took: %lu [mks]\n", main_time);
    for (int i = 0; i < n_threads; ++i) {
        fprintf(file, "%d thread took: %lu [mks]\n", i, response[i]->time);
    }
}


/**
 * Takes next arguments:
 * n_threads
 * mode = <BLOCK|NUMBERS>
 * input_path
 * out_path
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    if (argc != 5) {
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }

    n_threads = atoi(argv[1]);

    if (strcmp(argv[2], "BLOCK") == 0) {
        mode = BLOCK;
    } else if (strcmp(argv[2], "NUMBERS") == 0) {
        mode = NUMBERS;
    } else {
        printf("Bad format of arguments\n");
        exit(EXIT_FAILURE);
    }
    image_in = read_image(argv[3]);
    image_out = read_image(argv[3]);
    thread_info_t threadInfo[n_threads];
    if (mode == BLOCK) {
        init_threads_info(threadInfo, image_in->width);
    } else {
        init_threads_info(threadInfo, image_in->max_gray);
    }

    response_t *response[n_threads];
    pthread_t threads[n_threads];

    long unsigned time;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    print_threads_info(threadInfo);
    run_threads(threads, threadInfo, response);

    gettimeofday(&end_time, NULL);
    time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec;

    write_image(argv[4], image_out, 20);
    write_results_to_file(response, time);
    free_image(image_in);
    free_image(image_out);

    return 0;
}
