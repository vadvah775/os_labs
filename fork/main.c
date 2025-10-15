#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

// обработчик для atexit()
void exit_handler() {
    printf("Процесс с PID %d завершает работу через atexit()\n", getpid());
}

// обработчик сигнала для signal() - SIGINT
void sigint_handler(int sig) {
    printf("Процесс %d получил сигнал SIGINT (%d)\n", getpid(), sig);
}

// рбработчик сигнала для sigaction() - SIGTERM
// он расширенный так как содержит контекст сигнала а так же то откуда пришёл сигнал
void sigterm_handler(int sig, siginfo_t *info, void *context) {
    printf("Процесс %d получил сигнал SIGTERM (%d) от процесса %d\n", 
           getpid(), sig, info->si_pid);
}

int main() {
    pid_t pid;
    int status;
    // регистрируем обработчик для atexit()
    if (atexit(exit_handler) != 0) {
        perror("atexit");
        exit(EXIT_FAILURE);
    }
    
    // устанавливаем обработчик для SIGINT с помощью signal()
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    
    // устанавливаем обработчик для SIGTERM с помощью sigaction()
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigterm_handler;
    sa.sa_flags = SA_SIGINFO;// ставим флаг чтобы он использовал расширенную версию
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    printf("Родительский процесс: PID = %d, PPID = %d\n", getpid(), getppid());
    
    pid = fork();
    
    if (pid == -1) {
        // ошибка при вызове fork()
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        // дочерний процесс
        printf("Дочерний процесс: PID = %d, PPID = %d\n", getpid(), getppid());
        
        // дочерний процесс выполняет какую-то работу
        printf("Дочерний процесс выполняет работу...\n");
        sleep(2);
        
        // завершаем дочерний процесс с определенным кодом
        printf("Дочерний процесс завершает работу\n");
        exit(42);
    }
    else {
        // родительский процесс
        printf("Родительский процесс: создан дочерний процесс с PID = %d\n", pid);
        
        printf("Родительский процесс ожидает завершения дочернего...\n");
        pid_t finished_pid = wait(&status);
        
        if (finished_pid == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        
        // анализируем код завершения дочернего процесса
        if (WIFEXITED(status)) {
            printf("Дочерний процесс %d завершился с кодом: %d\n", 
                   finished_pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс %d завершился по сигналу: %d\n", 
                   finished_pid, WTERMSIG(status));
        }
        
        printf("Родительский процесс завершает работу\n");
        
        printf("\nДемонстрация обработчиков сигналов:\n");
        printf("Отправьте SIGINT (Ctrl+C) или SIGTERM (из другого терминала)\n");
        printf("Ожидание 10 секунд для демонстрации...\n");
        
        sleep(10);
        
        printf("Родительский процесс завершен нормально\n");
    }
    
    return 0;
}
