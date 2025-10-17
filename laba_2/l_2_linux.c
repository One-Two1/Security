#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include <stdio.h>
//     Для ввода/вывода - printf(), scanf(), работа с файлами
//     Нужна для вывода информации пользователю

//#include <stdlib.h>
//    Стандартные функции
//    Нужна для вызова системных команд

//#include <unistd.h>
//    Нужна для работы с процессами и системными функциями

#define BUFFER_SIZE 4096

void print_usage(const char *program_name) {
    fprintf(stderr, "Использование: %s <искомая_строка> <замена>\n", program_name);
    fprintf(stderr, "Пример: echo 'hello world' | %s 'hello' 'goodbye'\n", program_name);
    fprintf(stderr, "Результат: 'goodbye world'\n");
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

    // Буфер для чтения данных
    char buffer[BUFFER_SIZE];

    // Обрабатываем стандартный ввод построчно
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Удаляем символ новой строки, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Выполняем замену
        char *result = replace_all(buffer, search, replace);
        if (result != NULL) {
            // Выводим результат
            printf("%s\n", result);
            free(result);
        } else {
            // Если замена не удалась, выводим оригинальную строку
            printf("%s\n", buffer);
        }
    }

    // Проверяем наличие ошибок при чтении
    if (ferror(stdin)) {
        fprintf(stderr, "Ошибка при чтении из стандартного ввода\n");
        return 1;
    }

    return 0;
}
