#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/statvfs.h>
#include <mntent.h>
#include <string.h>
#endif

int main() {
    printf("Список устройств-накопителей:\n\n");

#ifdef _WIN32
    unsigned int mask = GetLogicalDrives();
    char letter;
    for (letter = 'A'; letter <= 'Z'; letter++) {
        if (mask & 1) {
            char root[4] = {letter, ':', '\\', '\0'};
            unsigned int type = GetDriveTypeA(root);

            char fsName[MAX_PATH] = {0};
            char volName[MAX_PATH] = {0};
           DWORD serial, maxLen, flags;

            unsigned long long freeBytes, totalBytes;
            unsigned long long freeUserBytes;

            if (GetVolumeInformationA(root, volName, MAX_PATH,
                                      &serial, &maxLen, &flags,
                                      fsName, MAX_PATH)) {

                ULARGE_INTEGER freeB, totalB, freeUserB;
                if (GetDiskFreeSpaceExA(root, &freeB, &totalB, &freeUserB)) {
                    totalBytes = totalB.QuadPart;
                    freeBytes  = freeB.QuadPart;

                    printf("Диск %s\n", root);
                    printf("  Файловая система: %s\n", fsName);
                    printf("  Размер: %llu MB\n", totalBytes / (1024*1024));
                    printf("  Свободно: %llu MB\n", freeBytes / (1024*1024));

                    if (type == DRIVE_REMOVABLE)
                        printf("  Тип: съемный (USB или CD/DVD)\n");
                    else if (type == DRIVE_FIXED)
                        printf("  Тип: фиксированный (HDD/SSD)\n");
                    else
                        printf("  Тип: другой\n");

                    printf("\n");
                }
            }
        }
        mask >>= 1;
    }

#else
    FILE *mtab = setmntent("/etc/mtab", "r");
    if (!mtab) {
        perror("setmntent");
        return 1;
    }

    struct mntent *ent;
    while ((ent = getmntent(mtab)) != NULL) {
        if (strncmp(ent->mnt_fsname, "/dev/sd", 7) == 0 ||
            strncmp(ent->mnt_fsname, "/dev/hd", 7) == 0 ||
            strncmp(ent->mnt_fsname, "/dev/mmc", 8) == 0 ||
            strstr(ent->mnt_dir, "/media") || strstr(ent->mnt_dir, "/run/media")) {

            struct statvfs stat;
            if (statvfs(ent->mnt_dir, &stat) == 0) {
                unsigned long long total = stat.f_blocks * stat.f_frsize;
                unsigned long long free  = stat.f_bfree * stat.f_frsize;

                printf("Раздел %s (%s)\n", ent->mnt_fsname, ent->mnt_dir);
                printf("  Файловая система: %s\n", ent->mnt_type);
                printf("  Размер: %llu MB\n", total / (1024*1024));
                printf("  Свободно: %llu MB\n", free / (1024*1024));

                if (strstr(ent->mnt_dir, "/media") || strstr(ent->mnt_dir, "/run/media"))
                    printf("  Тип: съемный (USB)\n");
                else
                    printf("  Тип: фиксированный (HDD/SSD)\n");

                printf("\n");
            }
        }
    }
    endmntent(mtab);
#endif

    return 0;
}
