#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <программа> [аргументы...]\n", argv[0]);
        return 1;
    }

    // argv[0] - имя нашей программы
    // argv[1] - имя запускаемой программы
    // argv[2:] - аргументы для запускаемой программы
    char **new_argv = &argv[1];
    
    printf("[PID: %d] Запуск: %s\n", getpid(), argv[1]);
    
    // execvp использует переменные окружения родителя автоматически
    execvp(new_argv[0], new_argv);
    
    // Если exec вернул управление - произошла ошибка
    perror("execvp");
    return 1;
}
