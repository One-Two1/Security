#include <stdio.h>
#include <stdlib.h>
#include <process.h>  // для _spawnvp
#include <windows.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage%s <program> [arg...]\n", argv[0]);
        printf("Example: %s notepad.exe test.txt\n", argv[0]);
        return 1;
    }

    // argv[0] - имя нашей программы
    // argv[1] - имя запускаемой программы
    // argv[2:] - аргументы для запускаемой программы
    char **new_argv = &argv[1];
    
    printf("[PID: %d] Start: %s\n", GetCurrentProcessId(), argv[1]);
    
    // _spawnvp - аналог execvp в Windows
    // _P_WAIT - ждать завершения дочернего процесса
    // _P_NOWAIT - не ждать (асинхронный запуск)
    intptr_t result = _spawnvp(_P_WAIT, new_argv[0], (const char * const *)new_argv);
    
    if (result == -1) {
       // printf("Error %d\n", errno);
       // perror("_spawnvp");
        
        // Дополнительная информация об ошибке Windows
        DWORD error = GetLastError();
        printf("Windows error code: %lu\n", error);
        return 1;
    }
    
    printf("Child process has ended: %d\n", result);
    return 0;
}