#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 1024
#define SLEEP_TIME 10

int main() {
    int fd;
    char buffer[BUFFER_SIZE];

    time_t current_time;
    time(&current_time);
    
    printf("Process 2: Жду 10 секунд...\n");
    sleep(SLEEP_TIME);
    
    printf("Process 2: Открываю FIFO для чтения...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    
    // читаем из FIFO
    read(fd, buffer, BUFFER_SIZE);
    close(fd);
    
   
    // выводим информацию
    printf("=== Process 2 ===\n");
    printf("Время Process2: %s", ctime(&current_time));
    printf("Полученные данные: %s\n", buffer);
    
    return 0;
}
