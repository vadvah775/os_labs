#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 256
#define READERS_COUNT 10

char* shared_buffer;
int write_counter = 0;
int running = 1;
pthread_rwlock_t rwlock;

void* writer_thread(void* arg) {
    char* buffer = (char*)arg;
    
    while (running) {
        pthread_rwlock_wrlock(&rwlock);
        
        snprintf(buffer, BUFFER_SIZE, "Record %d", ++write_counter);
        
        pthread_rwlock_unlock(&rwlock);
        
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void* reader_thread(void* arg) {
    char* buffer = (char*)arg;
    pthread_t tid = pthread_self();
    
    while (running) {
        pthread_rwlock_rdlock(&rwlock);
        
        printf("[Reader TID: %lu] Buffer: %s\n", (unsigned long)tid, buffer);
        
        pthread_rwlock_unlock(&rwlock);
        
        usleep(100000); // 100 мс
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t writer;
    pthread_t readers[READERS_COUNT];
    
    shared_buffer = malloc(BUFFER_SIZE);
    if (!shared_buffer) {
        perror("malloc failed");
        return 1;
    }
    
    strcpy(shared_buffer, "Initial value");
    
    pthread_rwlock_init(&rwlock, NULL);
    
    pthread_create(&writer, NULL, writer_thread, shared_buffer);
    
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_create(&readers[i], NULL, reader_thread, shared_buffer);
    }
    
    printf("5 секунд работы\n");
    sleep(5);
    
    // флаг завершения
    running = 0;
    
    pthread_join(writer, NULL);
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }
    
    pthread_rwlock_destroy(&rwlock);
    free(shared_buffer);
    
    printf("\nпрограмма завершена\n");
    
    return 0;
}
