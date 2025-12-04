#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* foo_write(void* arg){
    
    int* mas = (int*)arg;

    for(int i = 0; i < 5; ++i){
        
        pthread_mutex_lock(&mtx);
        mas[i] = i + 1;
        pthread_mutex_unlock(&mtx);
        usleep(500);
    }

    pthread_exit(NULL);
}

void* foo_read(void* arg){
    
    int* mas = (int*)arg;
    pthread_t tid = pthread_self();


    while(mas[4] == 0){
        pthread_mutex_lock(&mtx);
        fprintf(stdout, "[Thr#%lu] %d %d %d %d %d\n", (unsigned long)tid, mas[0], mas[1], mas[2], mas[3], mas[4]);
        pthread_mutex_unlock(&mtx);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv){
    pthread_t w_tid;
    pthread_t r_tid[10];
    
    int* mas = calloc(sizeof(int), 5);

    pthread_create(&w_tid, NULL, foo_write, mas);
    for(int i = 0; i < 10; ++i){
        pthread_create(&r_tid[i], NULL, foo_read, mas); 
    }

    pthread_join(w_tid, NULL);
    for(int i = 0; i < 10; ++i){
        pthread_join(r_tid[i], NULL);
    }
    free(mas);
    return 0;
}
