#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

typedef struct {
    pid_t pid;
    int exit_code;
} PROCESS_INFORMATION;

int create_process(char *executable, char **args, PROCESS_INFORMATION *pi) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        execvp(executable, args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // Родительский процесс
        pi->pid = pid;
        pi->exit_code = 0; // Пока неизвестно
        return 0; // Успех
    } else {
        return -1; // Ошибка fork
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <программа> [аргументы...]\n", argv[0]);
        return 1;
    }

    PROCESS_INFORMATION pi;
    char **new_args = &argv[1];
    
    printf("[Родитель PID: %d] Создание процесса для: %s\n", getpid(), argv[1]);
    
    if (create_process(argv[1], new_args, &pi) == 0) {
        printf("[Родитель] Процесс создан. PID потомка: %d\n", pi.pid);
        printf("[Родитель] Продолжаем работу (асинхронный запуск)...\n");
        
        // Демонстрация асинхронности
        sleep(2);
        
        // Опционально: ожидание потомка
        int status;
        waitpid(pi.pid, &status, 0);
        pi.exit_code = WEXITSTATUS(status);
        printf("[Родитель] Потомок %d завершился с кодом: %d\n", pi.pid, pi.exit_code);
    } else {
        fprintf(stderr, "[Родитель] Ошибка создания процесса\n");
        return 1;
    }
    
    return 0;
}
