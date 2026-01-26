#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>

#define SHM_SIZE 256
#define SHM_KEY 1
#define SEM_KEY 5


void cleanup(int sig) {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id >= 0) {
        shmctl(shm_id, IPC_RMID, NULL);
    }
    
    int sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id >= 0) {
        semctl(sem_id, 0, IPC_RMID);
    }
    
    exit(0);
}

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
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    
    int shm_id, sem_id;
    char* shared_memory;
    time_t rawtime;
    struct tm* timeinfo;
    
    // создаем разделяемую память
    shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(1);
    }
    
    // присоединение разделяемой памяти
    shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    // создаем семафор
    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        perror("semget failed");
        exit(1);
    }
    
    // инициализируем семафор (разблокирован)
    semctl(sem_id, 0, SETVAL, 1);
    
    printf("Sender started. PID: %d\n", getpid());
    
    while (1) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        char message[SHM_SIZE];
        snprintf(message, SHM_SIZE, "[%02d:%02d:%02d] PID: %d", 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, 
                getpid());
        
        sem_lock(sem_id);
        strcpy(shared_memory, message);
        sem_unlock(sem_id);
        
        sleep(3);
    }
    
    shmdt(shared_memory);
    
    return 0;
}
