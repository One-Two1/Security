#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл/URL> [--min|--max]\n", argv[0]);
        return 1;
    }

    char *target = argv[1];
    char *option = (argc > 2) ? argv[2] : "";
    
    printf("[PID: %d] Открытие: %s\n", getpid(), target);
    
    if (strstr(option, "min") || strstr(option, "max")) {
        printf("Заметка: В Linux флаги окон игнорируются (эмуляция ShellExecute)\n");
    }

    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс: запускаем xdg-open
        execlp("xdg-open", "xdg-open", target, NULL);
        perror("execlp xdg-open");
        exit(1);
    } else if (pid > 0) {
        // Родитель: ждем завершения
        int status;
        waitpid(pid, &status, 0);
        printf("Процесс открытия завершен с кодом: %d\n", WEXITSTATUS(status));
    } else {
        perror("fork");
        return 1;
    }
    
    return 0;
}
