
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <locale.h>

#define BUFFER_SIZE 4096

void print_usage(const char *program_name) {

    // SetConsoleOutputCP - установка кодовой страницы для корректного отображения русских символов
     SetConsoleOutputCP(CP_UTF8);

    fprintf(stderr, "Использование: %s <искомая_строка> <замена>\n", program_name);
    fprintf(stderr, "Пример: echo 'hello world' | %s 'hello' 'goodbye'\n", program_name);
    fprintf(stderr, "Результат: 'goodbye world'\n");
}

// Функция для установки режима работы с консолью (бинарный режим)
int set_binary_mode(void) {
    int result = _setmode(_fileno(stdin), _O_BINARY);
    if (result == -1) {
        fprintf(stderr, "Ошибка: не удалось установить бинарный режим для stdin\n");
        return -1;
    }
    
    result = _setmode(_fileno(stdout), _O_BINARY);
    if (result == -1) {
        fprintf(stderr, "Ошибка: не удалось установить бинарный режим для stdout\n");
        return -1;
    }
    
    return 0;
}

// Функция для чтения строки из stdin с поддержкой Windows и Unix концов строк
char* read_line(void) {
    static char buffer[BUFFER_SIZE];
    static size_t pos = 0;
    static size_t size = 0;
    
    if (pos >= size) {
        // Читаем новые данные
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, stdin);
        if (bytes_read == 0) {
            return NULL; // Конец ввода
        }
        pos = 0;
        size = bytes_read;
    }
    
    // Ищем конец строки (поддерживаем \n, \r\n)
    size_t line_start = pos;
    while (pos < size) {
        if (buffer[pos] == '\n') {
            // Найден конец строки Unix
            size_t line_length = pos - line_start;
            char *line = malloc(line_length + 1);
            if (line == NULL) return NULL;
            
            memcpy(line, buffer + line_start, line_length);
            line[line_length] = '\0';
            
            pos++; // Пропускаем \n
            return line;
        }
        else if (buffer[pos] == '\r') {
            if (pos + 1 < size && buffer[pos + 1] == '\n') {
                // Найден конец строки Windows \r\n
                size_t line_length = pos - line_start;
                char *line = malloc(line_length + 1);
                if (line == NULL) return NULL;
                
                memcpy(line, buffer + line_start, line_length);
                line[line_length] = '\0';
                
                pos += 2; // Пропускаем \r\n
                return line;
            }
            else {
                // Одиночный \r - обрабатываем как конец строки
                size_t line_length = pos - line_start;
                char *line = malloc(line_length + 1);
                if (line == NULL) return NULL;
                
                memcpy(line, buffer + line_start, line_length);
                line[line_length] = '\0';
                
                pos++; // Пропускаем \r
                return line;
            }
        }
        pos++;
    }
    
    // Достигнут конец буфера без найденного конца строки
    size_t line_length = size - line_start;
    char *line = malloc(line_length + 1);
    if (line == NULL) return NULL;
    
    memcpy(line, buffer + line_start, line_length);
    line[line_length] = '\0';
    
    pos = size; // Весь буфер обработан
    return line;
}

// Функция для замены всех вхождений подстроки
char* replace_all(const char *str, const char *search, const char *replace) {
    if (str == NULL || search == NULL || replace == NULL) {
        return NULL;
    }
    
    // Если искомая строка пустая, возвращаем копию исходной строки
    if (strlen(search) == 0) {
        return strdup(str);
    }
    
    // Подсчитываем количество вхождений
    size_t count = 0;
    const char *pos = str;
    while ((pos = strstr(pos, search)) != NULL) {
        count++;
        pos += strlen(search);
    }
    
    // Выделяем память для результата
    size_t result_len = strlen(str) + count * (strlen(replace) - strlen(search)) + 1;
    char *result = malloc(result_len);
    if (result == NULL) {
        return NULL;
    }
    
    // Выполняем замену
    char *current_pos = result;
    const char *start = str;
    const char *found;
    
    while ((found = strstr(start, search)) != NULL) {
        // Копируем часть до найденной подстроки
        size_t segment_len = found - start;
        memcpy(current_pos, start, segment_len);
        current_pos += segment_len;
        
        // Копируем строку замены
        memcpy(current_pos, replace, strlen(replace));
        current_pos += strlen(replace);
        
        // Перемещаем указатель за найденную подстроку
        start = found + strlen(search);
    }
    
    // Копируем оставшуюся часть строки
    strcpy(current_pos, start);
    
    return result;
}

int main(int argc, char *argv[]) {
    // Устанавливаем кодовую страницу для поддержки русского языка
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    // Проверяем количество аргументов
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    char *search = argv[1];
    char *replace = argv[2];
    
    // Проверяем, что аргументы не NULL
    if (search == NULL || replace == NULL) {
        fprintf(stderr, "Ошибка: неверные аргументы\n");
        return 1;
    }
    
    // Устанавливаем бинарный режим для корректной работы с конвейерами
    if (set_binary_mode() == -1) {
        return 1;
    }
    
    // Обрабатываем стандартный ввод построчно
    char *line;
    while ((line = read_line()) != NULL) {
        // Выполняем замену
        char *result = replace_all(line, search, replace);
        if (result != NULL) {
            // Выводим результат с Windows-форматом конца строки
            printf("%s\r\n", result);
            free(result);
        } else {
            // Если замена не удалась, выводим оригинальную строку
            printf("%s\r\n", line);
        }
        free(line);
    }
    
    // Проверяем наличие ошибок при чтении
    if (ferror(stdin)) {
        fprintf(stderr, "Ошибка при чтении из стандартного ввода\n");
        return 1;
    }
    
    return 0;
}