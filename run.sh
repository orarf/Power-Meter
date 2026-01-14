#!/bin/bash
set -e

echo "Starting Power-Tag Service..."

# ใช้ Path ที่เรา COPY ไปไว้ใน Dockerfile
EXECUTABLE="/usr/local/bin/power-tag"

# ตรวจสอบว่ามี Token ส่งผ่านมาทาง Environment Variable หรือไม่ 
# ถ้าไม่มีให้ใช้ค่า Default ที่ระบุไว้ในนี้
TOKEN=${TB_TOKEN:-"pEgujBQots9Mq3pQY6lQ"}

if [ -f "$EXECUTABLE" ]; then
    echo "Running with Token: $TOKEN"
    # รันโปรแกรมโดยส่ง Token เป็น argument
    "$EXECUTABLE" "$TOKEN"
else
    echo "Error: Executable not found at $EXECUTABLE"
    exit 1
fi