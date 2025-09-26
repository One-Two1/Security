#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <locale.h>


// Библиотеки:
// windows.h - основной заголовочный файл Windows API
// tchar.h - для поддержки Unicode и ANSI символов
// stdio.h - для стандартного ввода/вывода
// <locale.h> для поддержки руского языка

// Функция для выполнения команды и вывода результата
void ExecuteCommand(const TCHAR* command, const TCHAR* description) {
    _tprintf(_T("\n%s:\n"), description);
    _tprintf(_T("--------------------------------------------------\n"));
    _tsystem(command);
    _tprintf(_T("--------------------------------------------------\n"));
}

// Функция для получения детальной информации об устройстве через WMIC
void GetDetailedDeviceInfoWMI(const TCHAR* driveLetter) {
    _tprintf(_T("\n=== ДЕТАЛЬНАЯ ИНФОРМАЦИЯ О ДИСКЕ %c: ===\n"), driveLetter[0]);
    
    TCHAR command[512];
    
    // 1. Информация о логическом диске
    _stprintf_s(command, 512, 
        _T("wmic logicaldisk where \"DeviceID='%c:'\" get /value"), 
        driveLetter[0]);
    ExecuteCommand(command, _T("Информация о логическом диске"));
    
    // 2. Информация о разделе
    _stprintf_s(command, 512,
        _T("wmic partition where \"Name like '%%%c:%%'\" get /value"),
        driveLetter[0]);
    ExecuteCommand(command, _T("Информация о разделе"));
    
    // 3. Информация о физическом диске
    _stprintf_s(command, 512,
        _T("wmic diskdrive get /value | findstr /i /c:\"Model\" /c:\"Size\" /c:\"Interface\" /c:\"Media\""));
    ExecuteCommand(command, _T("Информация о физических дисках"));
    
    // 4. Информация о файловой системе через fsutil
    _stprintf_s(command, 512,
        _T("fsutil fsinfo volumeinfo %c:"), 
        driveLetter[0]);
    ExecuteCommand(command, _T("Информация о файловой системе (fsutil)"));
    
    // 5. Статистика использования
    _stprintf_s(command, 512,
        _T("fsutil fsinfo statistics %c:"), 
        driveLetter[0]);
    ExecuteCommand(command, _T("Статистика файловой системы"));
}

// Функция для получения информации через PowerShell
void GetDeviceInfoPowerShell(const TCHAR* driveLetter) {
    _tprintf(_T("\n=== ИНФОРМАЦИЯ ЧЕРЕЗ POWERSHELL ===\n"));
    
    TCHAR command[1024];
    
    // Комплексная информация через PowerShell
    _stprintf_s(command, 1024,
        _T("powershell -command \"")
        _T("Get-WmiObject -Class Win32_LogicalDisk -Filter \\\"DeviceID='%c:'\\\" | Format-List *;")
        _T("Write-Host '--- Физические диски ---';")
        _T("Get-Disk | Format-Table Number, FriendlyName, Size, BusType, PartitionStyle -AutoSize;")
        _T("Write-Host '--- Тома ---';")
        _T("Get-Volume | Where-Object { $_.DriveLetter -eq '%c' } | Format-List *\"")
        , driveLetter[0], driveLetter[0]);
    
    ExecuteCommand(command, _T("Комплексная информация (PowerShell)"));
}

// Функция для безопасного извлечения через стандартные утилиты
BOOL EjectDeviceStandard(const TCHAR* driveLetter) {
    _tprintf(_T("\n=== БЕЗОПАСНОЕ ИЗВЛЕЧЕНИЕ ДИСКА %c: ===\n"), driveLetter[0]);
    
    TCHAR command[256];
    int result;
    
    // Способ 1: Через WMIC (самый надежный)
    _tprintf(_T("Способ 1: Через WMIC...\n"));
    _stprintf_s(command, 256, 
        _T("wmic volume where \"DriveLetter='%c:'\" call eject"), 
        driveLetter[0]);
    
    result = _tsystem(command);
    if (result == 0) {
        _tprintf(_T("✓ Устройство успешно подготовлено к извлечению через WMIC\n"));
        return TRUE;
    }
    
    // Способ 2: Через PowerShell
    _tprintf(_T("Способ 2: Через PowerShell...\n"));
    _stprintf_s(command, 256,
        _T("powershell -command \"$driveEject = New-Object -comObject Shell.Application;")
        _T("$driveEject.Namespace(17).ParseName('%c:').InvokeVerb('Eject')\""),
        driveLetter[0]);
    
    result = _tsystem(command);
    if (result == 0) {
        _tprintf(_T("✓ Устройство успешно извлечено через PowerShell\n"));
        return TRUE;
    }
    
    // Способ 3: Через mountvol (для размонтирования)
    _tprintf(_T("Способ 3: Через mountvol...\n"));
    _stprintf_s(command, 256,
        _T("mountvol %c: /D"),
        driveLetter[0]);
    
    result = _tsystem(command);
    if (result == 0) {
        _tprintf(_T("✓ Точка монтирования удалена через mountvol\n"));
        return TRUE;
    }
    
    _tprintf(_T("✗ Не удалось извлечь устройство стандартными методами\n"));
    return FALSE;
}

// Функция для проверки типа устройства
BOOL IsRemovableDevice(const TCHAR* driveLetter) {
    TCHAR command[256];
    TCHAR tempFile[] = _T("temp_drive_check.txt");
    
    // Проверяем через WMIC является ли устройство съемным
    _stprintf_s(command, 256,
        _T("wmic logicaldisk where \"DeviceID='%c:'\" get DriveType > %s"),
        driveLetter[0], tempFile);
    
    _tsystem(command);
    
    // Читаем результат из временного файла
    FILE* file = _tfopen(tempFile, _T("r"));
    if (file) {
        TCHAR line[256];
        while (_fgetts(line, 256, file)) {
            if (_tcsstr(line, _T("2"))) { // DriveType 2 = Removable
                fclose(file);
                _tremove(tempFile);
                return TRUE;
            }
        }
        fclose(file);
    }
    
    _tremove(tempFile);
    return FALSE;
}

// Функция для получения информации через diskpart
void GetDeviceInfoDiskpart(const TCHAR* driveLetter) {
    _tprintf(_T("\n=== ИНФОРМАЦИЯ ЧЕРЕЗ DISKPART ===\n"));
    
    // Создаем скрипт для diskpart
    FILE* script = _tfopen(_T("diskpart_script.txt"), _T("w"));
    if (script) {
        _ftprintf(script, _T("select volume %c\n"), driveLetter[0]);
        _ftprintf(script, _T("detail volume\n"));
        _ftprintf(script, _T("list disk\n"));
        _ftprintf(script, _T("exit\n"));
        fclose(script);
        
        TCHAR command[256];
        _stprintf_s(command, 256,
            _T("diskpart /s diskpart_script.txt"));
        ExecuteCommand(command, _T("DiskPart информация"));
        
        _tremove(_T("diskpart_script.txt"));
    }
}

// Функция для вывода общей системной информации
void GetSystemStorageInfo() {
    _tprintf(_T("\n=== ОБЩАЯ СИСТЕМНАЯ ИНФОРМАЦИЯ ===\n"));
    
    ExecuteCommand(_T("wmic diskdrive list brief"), _T("Физические диски (кратко)"));
    ExecuteCommand(_T("wmic logicaldisk list brief"), _T("Логические диски (кратко)"));
    ExecuteCommand(_T("wmic volume list brief"), _T("Тома (кратко)"));
    ExecuteCommand(_T("mountvol"), _T("Точки монтирования"));
}

// Интерактивное меню для работы с конкретным устройством
void InteractiveDeviceManagement() {
    _tprintf(_T("\n=== УПРАВЛЕНИЕ УСТРОЙСТВАМИ ЧЕРЕЗ СТАНДАРТНЫЕ УТИЛИТЫ ===\n"));
    
    // Получаем список дисков через WMIC
    ExecuteCommand(_T("wmic logicaldisk where \"DriveType=2 or DriveType=3\" get DeviceID,DriveType,Size,FileSystem /value"), 
                  _T("Доступные диски (съемные и фиксированные)"));
    
    _tprintf(_T("\nВведите букву диска для работы (например, C): "));
    TCHAR driveLetter[10];
    _tscanf(_T("%s"), driveLetter);
    
    if (_tcslen(driveLetter) != 1) {
        _tprintf(_T("Неверный ввод! Должна быть одна буква.\n"));
        return;
    }
    
    TCHAR drivePath[] = _T("X:");
    drivePath[0] = driveLetter[0];
    
    int choice;
    do {
        _tprintf(_T("\n=== МЕНЮ ДЛЯ ДИСКА %s ===\n"), drivePath);
        _tprintf(_T("1. Базовая информация (WMIC)\n"));
        _tprintf(_T("2. Детальная информация (PowerShell)\n"));
        _tprintf(_T("3. Информация через DiskPart\n"));
        _tprintf(_T("4. Информация о файловой системе (fsutil)\n"));
        _tprintf(_T("5. Проверить тип устройства\n"));
        
        if (IsRemovableDevice(driveLetter)) {
            _tprintf(_T("6. ⚡ БЕЗОПАСНОЕ ИЗВЛЕЧЕНИЕ (съемное устройство)\n"));
        } else {
            _tprintf(_T("6. Устройство не является съемным\n"));
        }
        
        _tprintf(_T("7. Общая системная информация\n"));
        _tprintf(_T("0. Назад\n"));
        _tprintf(_T("Выберите: "));
        
        _tscanf(_T("%d"), &choice);
        
        switch (choice) {
            case 1:
                GetDetailedDeviceInfoWMI(driveLetter);
                break;
            case 2:
                GetDeviceInfoPowerShell(driveLetter);
                break;
            case 3:
                GetDeviceInfoDiskpart(driveLetter);
                break;
            case 4: {
                TCHAR command[256];
                _stprintf_s(command, 256, _T("fsutil fsinfo volumeinfo %s"), drivePath);
                ExecuteCommand(command, _T("Информация о файловой системе"));
                break;
            }
            case 5:
                if (IsRemovableDevice(driveLetter)) {
                    _tprintf(_T("✓ Устройство %s является СЪЕМНЫМ\n"), drivePath);
                } else {
                    _tprintf(_T("✗ Устройство %s НЕ является съемным\n"), drivePath);
                }
                break;
            case 6:
                if (IsRemovableDevice(driveLetter)) {
                    _tprintf(_T("Вы уверены, что хотите извлечь устройство %s? (y/n): "), drivePath);
                    TCHAR confirm;
                    _tscanf(_T(" %c"), &confirm);
                    if (confirm == _T('y') || confirm == _T('Y')) {
                        EjectDeviceStandard(driveLetter);
                    }
                }
                break;
            case 7:
                GetSystemStorageInfo();
                break;
            case 0:
                break;
            default:
                _tprintf(_T("Неверный выбор!\n"));
        }
    } while (choice != 0);
}

// Функция для массового вывода информации о всех устройствах
void ShowAllDevicesInfo() {
    _tprintf(_T("\n=== КОМПЛЕКСНАЯ ИНФОРМАЦИЯ О ВСЕХ УСТРОЙСТВАХ ===\n"));
    
    ExecuteCommand(_T("wmic diskdrive list full"), _T("Вся информация о физических дисках"));
    ExecuteCommand(_T("wmic logicaldisk list full"), _T("Вся информация о логических дисках"));
    ExecuteCommand(_T("wmic volume list full"), _T("Вся информация о томах"));
    ExecuteCommand(_T("powershell \"Get-StorageDiagnosticInfo\""), _T("Диагностическая информация о хранилище"));
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    int choice;
    do {
        _tprintf(_T("\n=== ГЛАВНОЕ МЕНЮ ===\n"));
        _tprintf(_T("1. Управление конкретным устройством\n"));
        _tprintf(_T("2. Комплексная информация о всех устройствах\n"));
        _tprintf(_T("3. Общая системная информация\n"));
        _tprintf(_T("0. Выход\n"));
        _tprintf(_T("Выберите: "));
        
        _tscanf(_T("%d"), &choice);
        
        switch (choice) {
            case 1:
                InteractiveDeviceManagement();
                break;
            case 2:
                ShowAllDevicesInfo();
                break;
            case 3:
                GetSystemStorageInfo();
                break;
            case 0:
                _tprintf(_T("Выход...\n"));
                break;
            default:
                _tprintf(_T("Неверный выбор!\n"));
        }
    } while (choice != 0);
    
    return 0;
}