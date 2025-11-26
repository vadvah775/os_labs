#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char* argv[]){
    
    int pipefd[2];
    char buffer[100];
    
    if(pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if(pid == -1){
        perror("fork\n");
        return 1;
    }
    
    if (pid == 0){
        //дочерний процесс, читатель
        close(pipefd[1]);

        sleep(5);

        read(pipefd[0], buffer, sizeof(buffer));
        close(pipefd[0]);
        
        time_t child_time;
        time(&child_time);

        printf("----Дочерний процесс----\n");
        printf("Время дочернего %s", ctime(&child_time));
        printf("%s\n", buffer);
    } else {
        // родительский процесс
        close(pipefd[0]);

        time_t current_time;
        time(&current_time);

        snprintf(buffer, 
                sizeof(buffer), 
                "Время родителя %sPID родителя %d", 
                ctime(&current_time), getpid());        

        write(pipefd[1], buffer, sizeof(buffer));

        close(pipefd[1]);
        wait(NULL);
    }


    return 0; 
}
