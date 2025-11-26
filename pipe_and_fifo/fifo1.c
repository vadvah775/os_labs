#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 1024
#define SLEEP_TIME 10

int main() {
    int fd;
    char buffer[BUFFER_SIZE];
    
    // cоздаем FIFO
    mkfifo(FIFO_NAME, 0666);
    
    printf("Process 1: Открываю FIFO для записи...\n");
    fd = open(FIFO_NAME, O_WRONLY);
    
    time_t current_time;
    time(&current_time);
    
    // формируем строку с временем и PID
    snprintf(buffer, BUFFER_SIZE, 
            "Время от Process1: %sPID Process1: %d", 
            ctime(&current_time), getpid());
    
    // пишем в FIFO
    write(fd, buffer, strlen(buffer) + 1);
    close(fd);
    
    printf("Process 1: Данные отправлены в FIFO\n");
    
    // удаляем FIFO
    unlink(FIFO_NAME);
    
    return 0;
}
