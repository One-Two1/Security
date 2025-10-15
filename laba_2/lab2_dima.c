#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <wctype.h>

void print_help(const char *prog)
{
    printf("LAB2 - подсчет символов, слов и строк\n");
    printf("Автор: Дмитрий\n");
    printf("Использование:\n");
    printf("  %s [/?: справка] [-f файл]\n", prog);
}

void wc_count_file(FILE *f)
{
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    unsigned char buf[4];
    size_t n;
    int c;
    wchar_t wc;
    unsigned long long characters = 0;
    unsigned long long words = 0;
    unsigned long long lines = 0;
    int in_word = 0;

    while ((c = fgetc(f)) != EOF)
    {
        buf[0] = (unsigned char)c;
        n = 1;

        if ((buf[0] & 0x80) != 0)
        
        {
            int count = 0;
            if ((buf[0] & 0xE0) == 0xC0)
            {
                count = 1;
            }
            else if ((buf[0] & 0xF0) == 0xE0)
            {
                count = 2;
            }
            else if ((buf[0] & 0xF8) == 0xF0)
            {
                count = 3;
            }
            for (int i = 0; i < count; i++)
            {
                int next = fgetc(f);
                if (next == EOF)
                {
                    break;
                }
                buf[n++] = (unsigned char)next;
            }
        }

        size_t ret = mbrtowc(&wc, (char*)buf, n, &state);
        if (ret == (size_t)-1 || ret == (size_t)-2)
        {
            continue;
        }

        if (!iswspace(wc))
        {
            characters++;
        }

        if (wc == L'\n')
        {
            lines++;
        }

        if (iswspace(wc))
        {
            if (in_word)
            {
                words++;
                in_word = 0;
            }
        }
        else
        {
            in_word = 1;
        }
    }

    if (in_word)
    {
        words++;
    }

    printf("Символов: %llu\nСлов: %llu\nСтрок: %llu\n", characters, words, lines);
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "-?") == 0)
        {
            print_help(argv[0]);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            FILE *f = fopen(argv[i+1], "rb");
            if (!f)
            {
                perror("Не удалось открыть файл");
                return 1;
            }
            wc_count_file(f);
            fclose(f);
            return 0;
        }
    }

    printf("Введите текст:\n");
    wc_count_file(stdin);

    return 0;
}
