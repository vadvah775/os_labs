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

int shmId;
char* shmAddr;

void free_at_exit(int sig){
    shmdt(shmAddr);
    shmctl(shmId, IPC_RMID, NULL);
    unlink(FILENAME);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    
    int fd = open(FILENAME, O_WRONLY | O_CREAT);
    if(fd == -1){
        perror("open file");
        return 1;
    }
    close(fd);

    key_t shmKey = ftok(FILENAME, 1);
    if(shmKey == -1){
        perror("ftok");
        return 1;
    } 

    shmId = shmget(shmKey, BUFSIZE, IPC_CREAT | IPC_EXCL | 0660);
    if (shmId == -1){
        perror("shmget");
        return 1;
    }

    signal(SIGINT, free_at_exit);
    signal(SIGTERM, free_at_exit);

    shmAddr = (char*)shmat(shmId, NULL, 0);
    memset(shmAddr, 0, BUFSIZE);


    char buffer[BUFSIZE]; 
    while(1){
        time_t t = time(NULL);

        sprintf(buffer, "Время %sPID отправителя: %d", ctime(&t), getpid());

        strcpy(shmAddr, buffer);

        sleep(1);
    }
    return 0;
}
