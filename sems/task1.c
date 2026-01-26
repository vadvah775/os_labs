#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>

#define BUFFER_SIZE 256

sem_t semaphore;

void* writer_thread(void* arg) {
    char* buffer = (char*)arg;
    int counter = 1;
    
    while (1) {
        
        snprintf(buffer, BUFFER_SIZE, "Record %d", counter++);
        
        sem_post(&semaphore);
        
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void* reader_thread(void* arg) {
    char* buffer = (char*)arg;
    
    pthread_t tid = pthread_self();
    
    while (1) {
        sem_wait(&semaphore);
        
        printf("[Reader TID: %lu] Buffer: %s\n", (unsigned long)tid, buffer);
        
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t writer, reader;
    
    char* buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc failed");
        return 1;
    }
    
    strcpy(buffer, "Initial value");
    
    sem_init(&semaphore, 0, 1);
    
    pthread_create(&writer, NULL, writer_thread, buffer);
    pthread_create(&reader, NULL, reader_thread, buffer);
    
    sleep(5);
    
    printf("\nFinishing program...\n");
    
    
    free(buffer);
    sem_destroy(&semaphore);
    
    return 0;
}
