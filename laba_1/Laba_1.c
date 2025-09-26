#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <locale.h>


// Библиотеки:
// windows.h - основной заголовочный файл Windows API
// tchar.h - для поддержки Unicode и ANSI символов
// stdio.h - для стандартного ввода/вывода
// <locale.h> для поддержки руского языка

// Функция для определения типа накопителя
const TCHAR* GetDriveTypeString(UINT type) {
    switch (type) {
        case DRIVE_UNKNOWN:     return _T("Неизвестный тип");
        case DRIVE_NO_ROOT_DIR: return _T("Несуществующий диск");
        case DRIVE_REMOVABLE:   return _T("Съемный диск (USB, флешка)");
        case DRIVE_FIXED:       return _T("Жесткий диск (HDD/SSD)");
        case DRIVE_REMOTE:      return _T("Сетевой диск");
        case DRIVE_CDROM:       return _T("CD/DVD-ROM");
        case DRIVE_RAMDISK:     return _T("RAM-диск");
        default:                return _T("Неопределенный тип");
    }
}

// Функция для получения информации о файловой системе
void GetFileSystemInfo(const TCHAR* drivePath, TCHAR* fsName, DWORD fsNameSize) {
    // GetVolumeInformation - получение информации о томе
    if (!GetVolumeInformation(
        drivePath,          // Путь к диску
        NULL,               // Имя тома (не требуется)
        0,                  // Размер буфера для имени тома
        NULL,               // Серийный номер (не требуется)
        NULL,               // Макс. длина имени файла
        NULL,               // Флаги файловой системы
        fsName,             // Буфер для имени ФС
        fsNameSize          // Размер буфера
    )) {
        _tcscpy_s(fsName, fsNameSize, _T("Недоступно"));
    }
}

// Функция для получения информации о дисковом пространстве
void GetDiskSpaceInfo(const TCHAR* drivePath, ULONGLONG* total, ULONGLONG* free) {
    ULARGE_INTEGER totalBytes, freeBytes;
    
    // GetDiskFreeSpaceEx - получение информации о свободном месте
    if (GetDiskFreeSpaceEx(
        drivePath,          // Путь к диску
        NULL,               // Байты доступные текущему пользователю
        &totalBytes,        // Общий размер тома
        &freeBytes          // Свободное место
    )) {
        *total = totalBytes.QuadPart;
        *free = freeBytes.QuadPart;
    } else {
        *total = 0;
        *free = 0;
    }
}

// Функция для форматирования размера в читаемый вид
void FormatSize(ULONGLONG bytes, TCHAR* buffer, size_t bufferSize) {
    const TCHAR* units[] = { _T("Б"), _T("КБ"), _T("МБ"), _T("ГБ"), _T("ТБ") };
    int unitIndex = 0;
    double size = (double)bytes;
    
    while (size >= 1024 && unitIndex < 4) {
        size /= 1024;
        unitIndex++;
    }
    
    _stprintf_s(buffer, bufferSize, _T("%.2f %s"), size, units[unitIndex]);
}

// Основная функция анализа накопителей
void AnalyzeStorageDevices() {
    _tprintf(_T("=== АНАЛИЗ НАКОПИТЕЛЕЙ ИНФОРМАЦИИ ===\n"));
    
    
    TCHAR logicalDrives[MAX_PATH];
    TCHAR drivePath[] = _T("A:\\");
    
    // GetLogicalDriveStrings - получение строки всех логических дисков
    DWORD result = GetLogicalDriveStrings(MAX_PATH, logicalDrives);
    
    if (result == 0 || result > MAX_PATH) {
        _tprintf(_T("Ошибка: Не удалось получить список дисков.\n"));
        return;
    }
    
    _tprintf(_T("Обнаруженные накопители:\n"));
    _tprintf(_T("==============================================\n"));
    
    TCHAR* drivePtr = logicalDrives;
    int driveCount = 0;
    
    while (*drivePtr != _T('\0')) {
        _tprintf(_T("\n%d. Диск: %s\n"), ++driveCount, drivePtr);
        
        // GetDriveType - определение типа накопителя
        UINT driveType = GetDriveType(drivePtr);
        _tprintf(_T("   Тип: %s\n"), GetDriveTypeString(driveType));
        
        // Пропускаем несуществующие и сетевые диски для детальной информации
        if (driveType != DRIVE_NO_ROOT_DIR && driveType != DRIVE_UNKNOWN) {
            TCHAR fsName[MAX_PATH];
            ULONGLONG totalSpace, freeSpace;
            TCHAR totalStr[64], freeStr[64];
            
            // Получаем информацию о файловой системе
            GetFileSystemInfo(drivePtr, fsName, MAX_PATH);
            _tprintf(_T("   Файловая система: %s\n"), fsName);
            
            // Получаем информацию о дисковом пространстве
            GetDiskSpaceInfo(drivePtr, &totalSpace, &freeSpace);
            
            if (totalSpace > 0) {
                FormatSize(totalSpace, totalStr, 64);
                FormatSize(freeSpace, freeStr, 64);
                
                _tprintf(_T("   Общий размер: %s\n"), totalStr);
                _tprintf(_T("   Свободно: %s\n"), freeStr);
                
                // Вычисляем процент использования
                double usagePercent = 100.0 - ((double)freeSpace / totalSpace * 100.0);
                _tprintf(_T("   Использовано: %.1f%%\n"), usagePercent);
            }
        }
        
        // Переходим к следующему диску в строке
        drivePtr += _tcslen(drivePtr) + 1;
    }
    
    _tprintf(_T("\n==============================================\n"));
    _tprintf(_T("Всего обнаружено накопителей: %d\n"), driveCount);
}

// Дополнительная функция для получения расширенной информации через WMI (альтернативный способ)
void GetExtendedStorageInfo() {
    _tprintf(_T("\n=== ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ ===\n"));
    
    // Альтернативный способ через GetLogicalDrives (битовая маска)
    DWORD driveMask = GetLogicalDrives();
    
    if (driveMask == 0) {
        _tprintf(_T("Ошибка получения битовой маски дисков.\n"));
        return;
    }
    
    _tprintf(_T("Доступные буквы дисков: "));
    
    for (TCHAR drive = _T('A'); drive <= _T('Z'); drive++) {
        if (driveMask & (1 << (drive - _T('A')))) {
            _tprintf(_T("%c "), drive);
        }
    }
    _tprintf(_T("\n"));
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    _tprintf(_T("Программа анализа накопителей информации\n"));
    _tprintf(_T("Цель: Определение характеристик устройств-накопителей\n"));
    _tprintf(_T("Язык: C, API: Windows Native API\n\n"));
    
    // Основной анализ
    AnalyzeStorageDevices();
    
    // Дополнительная информация
    //GetExtendedStorageInfo();
    
    _tprintf(_T("\nАнализ завершен. Нажмите Enter для выхода...\n"));
    _gettchar();
    
    return 0;
}