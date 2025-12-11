#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FILENAME "shmem.info"
#define BUFSIZE 512
char* shmAddr;

int shmId;

void free_at_exit(int sig){
    shmdt(shmAddr);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    key_t shmKey = ftok(FILENAME, 1);
    if(shmKey == -1){
        perror("shmkey");
        return -1;
    }

    shmId = shmget(shmKey, BUFSIZE, 0);
    shmAddr = (char*)shmat(shmId, shmAddr, SHM_RDONLY);
    
    signal(SIGINT, free_at_exit);
    signal(SIGTERM, free_at_exit);

    char buffer[BUFSIZE];

    while (1) {
        time_t t = time(NULL);
        
        printf("\n=== Читатель ===\n");
        printf("Время: %s", ctime(&t));
        printf("PID читателя: %d\n", getpid());
        
      
        strcpy(buffer, shmAddr);
        
        
        printf("Получено: %s\n", buffer);
        
        sleep(2);
    }





}
