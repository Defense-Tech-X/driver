# VirtualDroneController

Виртуальный HID-джойстик для Windows, имитирующий пульт управления дроном. Игры и симуляторы Windows видят обычный джойстик, а внешняя программа (feeder) задаёт его состояние.

## Устройство

| Элемент  | HID usage   | Диапазон   |
|----------|-------------|------------|
| Throttle | Z           | 0..32767   |
| Yaw      | Rx          | 0..32767   |
| Pitch    | Y           | 0..32767   |
| Roll     | X           | 0..32767   |
| Acro     | Button 1    | 0 = Angle, 1 = Acro |

- Hardware ID: `root\VirtualDroneController`, VID/PID: `0xDEED` / `0xFEED`.
- Канал feeder: отдельная HID-коллекция `UsagePage 0xFF00`, `Usage 0x01`, feature report с ID `2`.
- Формат feature report (10 байт, little-endian): `ReportId (u8) | Throttle | Yaw | Pitch | Roll (u16 каждый) | FlightMode (u8)`. Контракт описан в [src/controller_protocol.h](src/controller_protocol.h).

## Требования

- Windows 11 x64 (INF рассчитан на сборку 22000 и новее), PowerShell, права администратора для установки.
- [EWDK](https://learn.microsoft.com/windows-hardware/drivers/download-the-wdk), смонтированный как диск. Путь и версия SDK задаются в [scripts/config.ps1](scripts/config.ps1) (по умолчанию `E:\`, `10.0.28000.0`).

## Использование

```powershell
# Один раз: создать сертификат подписи и сделать его доверенным (от администратора)
.\scripts\setup-machine.ps1

# Сборка и подпись каталога (Debug по умолчанию)
.\scripts\build.ps1 [-Configuration Debug|Release] [-Rebuild]

# Переустановка драйвера (от администратора)
.\scripts\deploy.ps1 [-Configuration Debug|Release]
```

`deploy.ps1` при каждом запуске удаляет прежнее устройство и пакет драйвера, сбрасывает сохранённую калибровку DirectInput и устанавливает драйвер заново. Проверка: `joy.cpl`, устройство "Virtual Drone Controller".

Драйвер работает с включённым Secure Boot благодаря локально доверенному сертификату.

## Структура

```
src/       исходники драйвера, .vcxproj, .inx, .rc
scripts/   настройка машины, сборка, установка
certs/     экспортированный сертификат (создаётся setup-machine.ps1, в git не хранится)
out/       результаты сборки (в git не хранятся)
```
