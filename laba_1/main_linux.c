#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <string.h>
#include <mntent.h>
#include <sys/stat.h>

void print_header(const char *title) {
    printf("\n\033[1;34m=== %s ===\033[0m\n", title);
    printf("--------------------------------------------------\n");
}

void print_disk_usage(const char *path) {
    struct statvfs buf;

    if (statvfs(path, &buf) == 0) {
        unsigned long long total = buf.f_blocks * buf.f_frsize;
        unsigned long long free = buf.f_bfree * buf.f_frsize;
        unsigned long long available = buf.f_bavail * buf.f_frsize;
        double used_percent = 100.0 - (100.0 * buf.f_bavail / buf.f_blocks);

        printf("Путь: %s\n", path);
        printf("Общий размер: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
        printf("Свободно: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));
        printf("Доступно: %.2f GB\n", available / (1024.0 * 1024.0 * 1024.0));
        printf("Использовано: %.1f%%\n", used_percent);
        printf("Размер блока: %lu bytes\n", buf.f_frsize);
        printf("---\n");
    }
}

// Функция для получения типа файловой системы через /proc/mounts
const char* get_filesystem_type(const char *mountpoint) {
    FILE *fp = setmntent("/proc/mounts", "r");
    if (fp == NULL) {
        return "unknown";
    }

    struct mntent *entry;
    while ((entry = getmntent(fp)) != NULL) {
        if (strcmp(entry->mnt_dir, mountpoint) == 0) {
            endmntent(fp);
            return entry->mnt_type;
        }
    }

    endmntent(fp);
    return "unknown";
}

void show_mounted_filesystems() {
    print_header("СМОНТИРОВАННЫЕ ФАЙЛОВЫЕ СИСТЕМЫ");
    system("df -h --output=source,fstype,size,used,avail,pcent,target | grep -E '^/dev/' | head -n 20");
}

void show_block_devices() {
    print_header("БЛОЧНЫЕ УСТРОЙСТВА");
    system("lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v 'loop' | head -n 15");
}

//void show_disk_info() {
//   print_header("ИНФОРМАЦИЯ О ДИСКАХ");
//    system("fdisk -l 2>/dev/null | grep 'Disk /dev/' | grep -v 'identifier' | head -10");
//}

void show_usb_devices() {
    print_header("USB УСТРОЙСТВА");
    system("lsusb | grep -i 'storage\\|disk\\|flash\\|usb' | head -10");
}

//void show_smart_info() {
//    print_header("SMART ИНФОРМАЦИЯ");
//    system("which smartctl >/dev/null 2>&1 && echo 'Доступные SMART устройства:' && smartctl --scan | head -5 || echo 'smartctl не установлен'");
//}

void show_disk_usage_detailed() {
    print_header("ДЕТАЛЬНАЯ ИНФОРМАЦИЯ О РАЗДЕЛАХ");

    FILE *fp = setmntent("/proc/mounts", "r");
    if (fp == NULL) {
        printf("Ошибка открытия /proc/mounts\n");
        return;
    }

    struct mntent *entry;
    int count = 0;

    while ((entry = getmntent(fp)) != NULL && count < 15) {
        // Показываем только реальные устройства (не виртуальные файловые системы)
        if (strncmp(entry->mnt_fsname, "/dev/", 5) == 0 &&
            strcmp(entry->mnt_type, "proc") != 0 &&
            strcmp(entry->mnt_type, "sysfs") != 0 &&
            strcmp(entry->mnt_type, "devtmpfs") != 0 &&
            strcmp(entry->mnt_type, "tmpfs") != 0 &&
            strcmp(entry->mnt_type, "cgroup") != 0) {

            printf("Устройство: %s\n", entry->mnt_fsname);
            printf("Точка монтирования: %s\n", entry->mnt_dir);
            printf("Тип ФС: %s\n", entry->mnt_type);
            print_disk_usage(entry->mnt_dir);
            count++;
        }
    }

    endmntent(fp);
}

void show_all_info() {
    printf("\033[1;35m\nАНАЛИЗ НАКОПИТЕЛЕЙ ИНФОРМАЦИИ LINUX\033[0m\n");
    printf("==================================================\n");

    show_block_devices();
    show_mounted_filesystems();
   // show_disk_info();
    show_disk_usage_detailed();
    show_usb_devices();
    //show_smart_info();
}

int main() {
    printf("Программа анализа накопителей информации для Linux\n");

    int choice;

    do {
        printf("\n\033[1;36mМЕНЮ ВЫБОРА:\033[0m\n");
        printf("1. Блочные устройства (lsblk)\n");
        printf("2. Смонтированные файловые системы (df)\n");
       // printf("3. Информация о дисках (fdisk)\n");
        printf("3. Детальная информация о разделах\n");
        printf("4. USB устройства (lsusb)\n");
       // printf("6. SMART информация\n");
        printf("5. Вся информация\n");
        printf("0. Выход\n");
        printf("Выберите опцию: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Очистка буфера
            printf("Неверный ввод!\n");
            continue;
        }

        switch (choice) {
            case 1:
                show_block_devices();
                break;
            case 2:
                show_mounted_filesystems();
                break;
         //   case 3:
            //    show_disk_info();
              //  break;
            case 3:
                show_disk_usage_detailed();
                break;
            case 4:
                show_usb_devices();
                break;
           // case 6:
              //  show_smart_info();
             //   break;
            case 5:
                show_all_info();
                break;
            case 0:
                printf("Выход...\n");
                break;
            default:
                printf("Неверный выбор!\n");
        }

    } while (choice != 0);

    return 0;
}
