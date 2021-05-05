#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int main(int argc, char *argv[])
{
  if (argc!=2) {
     printf("Prawidłowe wywołanie %s liczba\n",argv[0]); 
     exit(EXIT_FAILURE);
  }
  pid_t child;
  int status = 0;
  if((child = fork()) < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(child == 0) {
    sleep(2);
    exit(EXIT_SUCCESS);
  }
  else {
/* Proces macierzysty usypia na liczbę sekund będącą argumentem programu
 * Proces macierzysty pobiera status  zakończenia potomka child, nie zawieszając swojej pracy. 
 * Jeśli proces się nie zakończył, wysyła do dziecka sygnał zakończenia.
 * Jeśli wysłanie sygnału się nie powiodło, proces zwróci błąd.
 * Jeśli się powiodło, wypisuje komunikat sukcesu zakończenia procesu potomka 
 * z numerem jego PID i sposobem zakończenia (Proces xx zakończony przez exit albo sygnałem). */

// Solution:

      int sleep_time = atoi(argv[1]);
      sleep(sleep_time);
      int does_exit = (int)waitpid(child,&status,WNOHANG);
      printf("I am here\n");
      if ( does_exit != 0) {
          printf("Exit child status was %d\n", status);
      }else{
          if (kill(child,SIGKILL)==-1){
              perror("Error in kill");
              exit(EXIT_FAILURE);
          }
          printf("Child was killed with a signal\n");
      }

//      End of solution


 } //else
  return 0;
}
