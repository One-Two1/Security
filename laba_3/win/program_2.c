#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <locale.h>

#pragma comment(lib, "shell32.lib")

void show_usage(char *program_name) {
    SetConsoleOutputCP(CP_UTF8);
    printf("Использование: %s <файл/документ/URL> [режим]\n", program_name);
    printf("Режимы:\n");
    printf("  normal    - обычный режим (по умолчанию)\n");
    printf("  minimized - свернутое окно\n");
    printf("  maximized - развернутое окно\n");
    printf("  hidden    - скрытое окно\n");
    printf("\nПримеры:\n");
    printf("  %s notepad.exe\n", program_name);
    printf("  %s document.pdf maximized\n", program_name);
    printf("  %s https://google.com\n", program_name);
    printf("  %s C:\\Windows\\System32\\calc.exe minimized\n", program_name);
}

int get_show_command(char *mode) {
    if (mode == NULL) return SW_SHOWNORMAL;
    
    if (strcmp(mode, "minimized") == 0) return SW_SHOWMINIMIZED;
    if (strcmp(mode, "maximized") == 0) return SW_SHOWMAXIMIZED;
    if (strcmp(mode, "hidden") == 0) return SW_HIDE;
    if (strcmp(mode, "normal") == 0) return SW_SHOWNORMAL;
    
    return SW_SHOWNORMAL;  // по умолчанию
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    char *target = argv[1];
    char *mode = (argc > 2) ? argv[2] : "normal";
    
    int nShowCmd = get_show_command(mode);
    SetConsoleOutputCP(CP_UTF8);
    printf("[PID: %lu] Открытие: %s\n", GetCurrentProcessId(), target);
    printf("Режим окна: %s (%d)\n", mode, nShowCmd);
    
    // Определяем тип действия
    char *operation = NULL;
    
    // Если это исполняемый файл или URL
    if (strstr(target, ".exe") != NULL || 
        strstr(target, ".com") != NULL ||
        strstr(target, ".bat") != NULL ||
        strstr(target, ".cmd") != NULL) {
        operation = "open";
    } else if (strstr(target, "http://") == target || 
               strstr(target, "https://") == target) {
        operation = "open";  // открыть URL в браузере
    } else {
        operation = "open";  // открыть документ с ассоциированной программой
    }
    
    // Используем ShellExecute
    HINSTANCE result = ShellExecute(
        NULL,           // родительское окно (NULL = текущее)
        operation,      // операция ("open", "edit", "print")
        target,         // файл/документ/URL
        NULL,           // параметры
        NULL,           // рабочий каталог
        nShowCmd        // режим отображения окна
    );
    
    // Преобразуем HINSTANCE в целочисленный код ошибки
    // ShellExecute возвращает значение >32 при успехе
    INT_PTR result_code = (INT_PTR)result;
    
    if (result_code <= 32) {
        SetConsoleOutputCP(CP_UTF8);
        printf("Ошибка открытия. Код ошибки: %I64d\n", (long long)result_code);
        
        // Интерпретация ошибок ShellExecute
        switch (result_code) {
            case 0:
                printf("Не хватает памяти или ресурсов\n");
                break;
            case ERROR_FILE_NOT_FOUND:
                printf("Файл не найден\n");
                break;
            case ERROR_PATH_NOT_FOUND:
                printf("Путь не найден\n");
                break;
            case ERROR_BAD_FORMAT:
                printf("Неправильный формат исполняемого файла\n");
                break;
            case SE_ERR_ACCESSDENIED:
                printf("Доступ запрещен\n");
                break;
            case SE_ERR_ASSOCINCOMPLETE:
                printf("Неполная ассоциация файлов\n");
                break;
            case SE_ERR_DDEBUSY:
                printf("DDE занят\n");
                break;
            case SE_ERR_DDEFAIL:
                printf("Ошибка DDE\n");
                break;
            case SE_ERR_DDETIMEOUT:
                printf("Таймаут DDE\n");
                break;
            case SE_ERR_DLLNOTFOUND:
                printf("DLL не найдена\n");
                break;
            case SE_ERR_NOASSOC:
                printf("Нет ассоциированной программы\n");
                break;
            case SE_ERR_OOM:
                printf("Не хватает памяти\n");
                break;
            case SE_ERR_SHARE:
                printf("Ошибка совместного доступа\n");
                break;
            default:
                printf("Неизвестная ошибка\n");
        }
        
        // Получаем текстовое описание ошибки
        DWORD error = GetLastError();
        if (error != 0) {
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
            
            printf("Дополнительная информация: %s\n", (char*)lpMsgBuf);
            LocalFree(lpMsgBuf);
        }
        
        return 1;
    }
    SetConsoleOutputCP(CP_UTF8);
    printf("Процесс успешно запущен! Код возврата: %I64d\n", (long long)result_code);
    return 0;
}