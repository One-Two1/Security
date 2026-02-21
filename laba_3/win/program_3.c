#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <locale.h>

void print_error(const char *msg) {
    DWORD error = GetLastError();
    printf("%s. Код ошибки: %lu\n", msg, error);
    
    // Получаем текстовое описание ошибки
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    
    printf("Описание: %s\n", (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

void show_usage(char *program_name) {
    SetConsoleOutputCP(CP_UTF8);
    printf("Использование: %s <программа> [аргументы...]\n", program_name);
    printf("\nПараметры:\n");
    printf("  /wait      - ждать завершения процесса\n");
    printf("  /nowait    - не ждать (по умолчанию)\n");
    printf("\nПримеры:\n");
    printf("  %s notepad.exe test.txt\n", program_name);
    printf("  %s cmd.exe /c dir C:\\ /wait\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    // Параметры командной строки
    int wait_for_child = 0;  // по умолчанию не ждем
    
    // Пропускаем параметры нашей программы
    int first_arg = 1;
    if (argc > 2) {
        if (strcmp(argv[1], "/wait") == 0) {
            wait_for_child = 1;
            first_arg = 2;
        } else if (strcmp(argv[1], "/nowait") == 0) {
            wait_for_child = 0;
            first_arg = 2;
        }
    }
    
    if (first_arg >= argc) {
        show_usage(argv[0]);
        return 1;
    }
    
    char *app_name = argv[first_arg];
    
    // Формируем командную строку для CreateProcess
    // Пропускаем первые first_arg аргументов
    int cmdline_len = 0;
    for (int i = first_arg; i < argc; i++) {
        cmdline_len += strlen(argv[i]) + 3; // + кавычки и пробел
    }
    
    char *cmdline = (char*)malloc(cmdline_len + 1);
    if (cmdline == NULL) {
        printf("Ошибка выделения памяти\n");
        return 1;
    }
    
    cmdline[0] = '\0';
    for (int i = first_arg; i < argc; i++) {
        if (i > first_arg) strcat(cmdline, " ");
        
        // Добавляем кавычки, если есть пробелы в аргументе
        if (strchr(argv[i], ' ') != NULL) {
            char quoted_arg[1024];
            snprintf(quoted_arg, sizeof(quoted_arg), "\"%s\"", argv[i]);
            strcat(cmdline, quoted_arg);
        } else {
            strcat(cmdline, argv[i]);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
    printf("[Родитель PID: %d] Создание процесса: %s\n", 
           GetCurrentProcessId(), app_name);
    printf("Командная строка: %s\n", cmdline);
    printf("Режим: %s\n", wait_for_child ? "ожидание" : "без ожидания");
    
    // Структуры для CreateProcess
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;  // можно изменить
    
    ZeroMemory(&pi, sizeof(pi));
    
    // Создаем процесс
    BOOL success = CreateProcess(
        NULL,                    // имя исполняемого файла (используем командную строку)
        cmdline,                 // командная строка
        NULL,                    // атрибуты защиты процесса
        NULL,                    // атрибуты защиты потока
        FALSE,                   // наследование дескрипторов
        CREATE_NEW_CONSOLE,      // флаги создания
        NULL,                    // блок окружения (NULL = родительское)
        NULL,                    // рабочий каталог (NULL = текущий)
        &si,                     // STARTUPINFO
        &pi                      // PROCESS_INFORMATION
    );
    
    if (!success) {
        print_error("Ошибка создания процесса");
        free(cmdline);
        return 1;
    }
    SetConsoleOutputCP(CP_UTF8);
    printf("Процесс успешно создан!\n");
    printf("  PID процесса: %lu\n", pi.dwProcessId);
    printf("  ID потока: %lu\n", pi.dwThreadId);
    printf("  Дескриптор процесса: 0x%p\n", pi.hProcess);
    printf("  Дескриптор потока: 0x%p\n", pi.hThread);
    
    if (wait_for_child) {
        SetConsoleOutputCP(CP_UTF8);
        printf("Ожидание завершения процесса...\n");
        
        // Ждем завершения процесса
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        // Получаем код завершения
        DWORD exit_code;
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            printf("Процесс завершен с кодом: %lu\n", exit_code);
        }
    } else {
        SetConsoleOutputCP(CP_UTF8);
        printf("Продолжаем работу (асинхронный режим)...\n");
        
        // Даем немного времени дочернему процессу запуститься
        Sleep(100);
        printf("Родитель завершает работу, дочерний процесс продолжается\n");
    }
    
    // Закрываем дескрипторы
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    free(cmdline);
    return 0;
}