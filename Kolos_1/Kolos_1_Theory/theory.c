#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

double subtract_time(clock_t start, clock_t end) {
    return (double) (end - start) / (double) sysconf(_SC_CLK_TCK);
}

/**
 * Czekanie i mierzenie czasu
 */
void wait_and_measure() {

    sleep(1);


//    struct timespec {
//        time_t tv_sec; /* seconds */
//        long tv_nsec; /* nanoseconds */
//    };
    struct timespec timespec;
    timespec.tv_sec = 0;
//    Max value of nanoseconds
    timespec.tv_nsec = 999999999;
    nanosleep(&timespec, NULL);

//    Measure time
    clock_t real_time[2];
    struct tms **tms_time = malloc(2 * sizeof(struct tms *));
    for (int i = 0; i < 2; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms));
    }
    real_time[0] = times(tms_time[0]);
//    Do some actions:
    sleep(1);
//    End some actions
    real_time[1] = times(tms_time[1]);
//    Output
    printf("REAL: %lf   ", subtract_time(real_time[0], real_time[1]));
    printf("USER CPU: %lf   ", subtract_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    printf("SYSTEM CPU: %lf ", subtract_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
    for (int i = 0; i < 2; i++) {
        free(tms_time[i]);
    }
    free(tms_time);

}
/**
 * Operacje na plikach
 */

#ifdef LIB
#define M_FILE FILE *
#define M_READ fread(&ch, 1, 1, fp)
#define M_OPEN fopen(write_path, "a+")
#define M_OPEN_READ fopen(read_path, "r")
#define M_CLOSE fclose
#define M_WRITE fwrite(string, sizeof(char), strlen(string), ptr)
#define M_STDIN stdin
#define M_NULL NULL

#else

#include <zlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <wait.h>
#include <sys/stat.h>

#define M_FILE int
#define M_READ read(fp, &ch, sizeof(char))
#define M_OPEN open(write_path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)
#define M_OPEN_READ open(read_path, O_RDONLY)
#define M_CLOSE close
#define M_WRITE write(ptr, string, strlen(string))
#define M_STDIN STDIN_FILENO
#define M_NULL 0
#endif

/**
 * Reads string from fp and returns address to allocated memory
 * @param fp
 * @param start_size
 * @return
 */
char *inputString(M_FILE fp, size_t start_size) {
    char *str;
    int ch = 0;
    u_long k;
    size_t len = 0;
    str = realloc(NULL, sizeof(*str) * start_size);
    if (!str)
        return str;

    while (0 != (k = M_READ) && ch != '\n') {
        str[len++] = (char) ch;
        if (len == start_size - 2) {
            str = realloc(str, sizeof(*str) * (start_size += 16));
            if (!str)
                return str;
        }
    }
    if (k == 0 && len == 0) {
        free(str);
        return NULL;
    }
    str[len++] = '\n';
    str[len++] = '\0';
    return realloc(str, sizeof(*str) * len);
}

void files_operations() {
    char read_path[] = "text.txt";
    M_FILE file = M_OPEN_READ;
    char *string = inputString(file, 10);
    string = inputString(file, 10);
    string = inputString(file, 10);
    printf("%s", string);
    M_CLOSE;
    free(string);


}

/**
 * Operacje na katalogach
 * DIR* opendir(const char* dirname)
 * int closedir(DIR* dirp)
 * struct dirent* readdir(DIR* dirp)
 * void rewinddir(DIR* dirp)
 * void seekdir(DIR* dirp, long int loc)
 */
void catalogues() {
    struct dirent *dirEnt;
    DIR *main_directory = opendir("CMakeFiles");
    dirEnt = readdir(main_directory);
    printf("%s", dirEnt->d_name);
    closedir(main_directory);

}

/**
 * Procesy
 */
void processes() {
    if (fork() == 0) {
        printf("I am a child!\n");
        sleep(2);
        exit(0);
    }
    int status;
    wait(&status);
    printf("I am ended\n");
    exit(0);

}

void handler(int signo, siginfo_t *info, void *extra) {
    printf("The passed value is: %d\n", info->si_value.sival_int);
    if (info->si_value.sival_int != 0) {
        sleep(1);
        kill(info->si_value.sival_int, SIGUSR1);
    }

}

/**
 * Sygnaly
 * @param argc
 * @param argv
 * @return
 */
void signals() {


//    Ustawianie handlera
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler;
    sigemptyset(&act.sa_mask);

    sigaction(SIGUSR1, &act, NULL);

//Wysyanie sygnalu
    union sigval sigval;
    sigval.sival_int = getpid();

    if (fork() == 0) {
        if (sigqueue(getpid(), SIGUSR1, sigval) == 0) {
            printf("Signal sent successfully\n");
        } else {
            printf("Error sending signal\n");
        }
    }


// Making parent wait for a child to finish working
    sigset_t set_to_wait;
    sigfillset(&set_to_wait);
    sigdelset(&set_to_wait, SIGUSR1);
    sigsuspend(&set_to_wait);

}

/**
 * Potoki nienazwane oraz dip2
 */
void pipes_dip2() {

//    Nienazwane
// fd[0] - zapis
// fd[1] - odczyt
    int fd[2];
    if (pipe(fd)< 0){
        exit(-1);
    }
    pid_t pid = fork();
    if (pid == 0) { // dziecko
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execlp("grep", "grep", "Ala", NULL);


    } else { // rodzic
//        close(fd[0]);
        write(fd[1], "Ala", 20); // - przesłanie danych do grepa
    }
}
/**
 * Parent-child communication
 */
void pipes_usual(){
    int pipe_child[2];

    pipe(pipe_child);

    if (fork() == 0){
        //Child, will be writing
        close(pipe_child[0]);
        char string_to_write[] = "Hello from child!\n";
        sleep(1);
        write(pipe_child[1],string_to_write, strlen(string_to_write));
    }else{
        // Parent, will be reading
        close(pipe_child[1]);
        char read_buffer[30] = {'\0'};
        read(pipe_child[0],read_buffer, 50);
        printf("I am a parent, and I read: %s\n",read_buffer);

    }


}

void popen_use(){
    FILE *f = popen("wc -l", "w");
    fputs("Hello this\n Is America\n Special for you\n There",f);

    char buf[10];
    while (fgets(buf, 10, f) != NULL)
        printf("%s", buf);
    pclose(f);
}


// Potoki nazwane


void fifo_named(){
    char* fifo = "fifo_file";
    if(mkfifo(fifo, 0666) == -1){
        perror("Unable to create fifo");
    }
    printf("IM forkin\n");
    if(fork() == 0){
        execl("./prog2", "./prog2", "fifo_file", NULL);
        exit(0);
    }
    int f1 = open(fifo, O_RDWR);
    write(f1,"helno", sizeof("Helno"));
    wait(NULL);
    close(f1);
}

int main(int argc, char **argv) {
    popen_use();
}