#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_SIZE 256
#define SHM_KEY 1
#define SEM_KEY 5

// операции с семафором
void sem_lock(int sem_id) {
    struct sembuf op = {0, -1, 0};
    semop(sem_id, &op, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf op = {0, 1, 0};
    semop(sem_id, &op, 1);
}

int main() {
    int shm_id, sem_id;
    char* shared_memory;
    time_t rawtime;
    struct tm* timeinfo;
    
    // получаем разделяемую память
    shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }
    
    // присоединяем разделяемую память
    shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    // получаем семафор
    sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id < 0) {
        perror("semget failed");
        exit(1);
    }
    
    printf("Receiver started. PID: %d\n", getpid());
    
    while (1) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        char received[SHM_SIZE];
       
        sem_lock(sem_id);
        strcpy(received, shared_memory);
        sem_unlock(sem_id);   
        
        // Выводим информацию
        printf("[Receiver %02d:%02d:%02d PID:%d] Got: %s\n",
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               getpid(), received);       
        sleep(1);
    }
    
    shmdt(shared_memory);
    
    return 0;
}
