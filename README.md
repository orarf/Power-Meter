# PanelServer PAS600 Modbus Monitor

A C++ application to monitor Schneider Electric **iA9MEM15** (Single Phase) and **iPM2xxx** (3-Phase) energy meters via Modbus TCP through a PAS600 Panel Server outside.
It collects electrical measurements, stores them in local SQLite databases, and tracks historical energy trends.

## Features

- **Multi-Device Support**: Monitor multiple devices (iA9MEM15 & iPM2xxx) in a single loop.
- **Dual Database Storage**:
  - `iA9MEM15.db`: Stores single-phase readings.
  - `iPM2xxx.db`: Stores 3-phase readings.
- **Historical Tracking**: Tracks **Total Energy** (Delivered + Received) history:
  - Last 1 Minute
  - Last 5 Minutes
  - Last 30 Minutes
  - Last 1 Hour
  - Last 2 Hours
- **Robustness**:
  - Auto-reconnection logic.
  - Safe handling of `NaN` / invalid sensor values.
  - 60-second polling loop.
- **Configurable**: Uses a simple `.env` file (or environment variables) for network and device settings.

## Prerequisites

- **C++ Compiler**: Supporting C++17 or later.
- **CMake**: Version 3.16+.
- **SQLite3**: Source code is bundled/downloaded automatically by the build script (no manual install required).

## Configuration

Create/Export environment variables (or use a `.env` file loader if implemented):

```bash
export MODBUS_IP=192.168.100.100   # IP Address of the Modbus Server/Gateway
export MODBUS_PORT=502            # Modbus TCP Port (usually 502)
export DEVICE_IDS=100,101,102     # Comma-separated list of Unit IDs to poll
```

## Build and Run

The project includes a helper script to build and execute the application.

1.  **Make the script executable** (first time only):
    ```bash
    chmod +x build.sh
    ```

2.  **Build and Run**:
    ```bash
    ./build.sh
    ```

## Database Schema

### 1. iA9MEM15.db (Table: `readings`)
*   **Metrics**: Voltage (A-N), Current (A), Power (A, Total), PF, Internal Temp.
*   **History**: `total_energy` + `total_energy_last_XM`.

### 2. iPM2xxx.db (Table: `readings_pm2xxx`)
*   **Metrics**:
    *   **Voltage**: Phase A, B, C, Avg (L-N).
    *   **Current**: Phase A, B, C, Avg.
    *   **Power**: Active, Reactive, Apparent (Total).
    *   **Basics**: PF, Frequency.
*   **History**: `total_energy` + `total_energy_last_XM`.

*Data retention policy: Records older than **2 days** are automatically deleted.*

## Project Structure

- `main.cpp`: Entry point. Runs both monitors.
- `include/Read_iA9MEM15.h`: Logic for iA9MEM15 devices.
- `include/Read_iPM2xxx.h`: Logic for iPM2xxx devices.
- `include/iA9MEM15.h`: Modbus map for iA9MEM15.
- `include/iPM2xxx.h`: Modbus map for iPM2xxx.
- `build.sh`: Build automation script.

# PanelServer PAS600 Modbus Monitor

...existing content...

## วิธีนำโปรแกรมไปรันบนเครื่องเซิร์ฟเวอร์ (24 ชั่วโมง)

### 1. สร้างและติดตั้งโปรแกรมบนเซิร์ฟเวอร์

ให้นำโค้ดโปรเจกต์นี้ไปไว้บนเซิร์ฟเวอร์ จากนั้นเข้าไปในโฟลเดอร์โปรเจกต์แล้วรันคำสั่ง:
```bash
chmod +x build.sh
./build.sh
```

### 2. รันโปรแกรมแบบ Background ด้วย systemd (แนะนำ)

สร้างไฟล์ service สำหรับ systemd (เปลี่ยน `<TB_TOKEN>` และ path ให้ตรงกับของคุณ):

เปิดเท็กซ์เอดิเตอร์ เช่น nano เพื่อสร้างไฟล์ใหม่
sudo nano /etc/systemd/system/panelserver.service
[Unit]
Description=PanelServer PAS600 Modbus Monitor

[Service]
WorkingDirectory=/workspace/PanelServerPAS600
ExecStart=/workspace/PanelServerPAS600/build/main <TB_TOKEN>
Restart=always
User=ubuntu

[Install]
WantedBy=multi-user.target
```
กด Ctrl+O เพื่อบันทึกไฟล์ แล้วกด Ctrl+X เพื่อออกจาก nano

จากนั้นสั่งรันและตั้งค่าให้เริ่มอัตโนมัติ:
```bash
sudo systemctl daemon-reload
sudo systemctl enable panelserver
sudo systemctl start panelserver
sudo systemctl status panelserver
```
### 3. การตรวจสอบสถานะ

- ถ้าใช้ systemd:  
  ดู log ด้วยการพิมพ์คำสั่งรันใน terminal  
  journalctl -u panelserver -f

---