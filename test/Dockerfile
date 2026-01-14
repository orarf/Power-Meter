
# ใช้ Image Ubuntu (ขนาดใหญ่กว่า Alpine แต่นิยมใช้ทั่วไปและมี Tools ครบครัน)
FROM ubuntu:latest

# ปิดการถาม Interactive ของ apt (ป้องกัน Build ค้างตรง ZoneInfo)
ENV DEBIAN_FRONTEND=noninteractive

# ติดตั้งเครื่องมือสำหรับ Build และ Debug ครบชุดสำหรับ C++ และ CMake
# Ubuntu ใช้ apt-get แทน apk
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    clang \
    lldb \
    clang-format \
    clang-tidy \
    valgrind \
    cppcheck \
    pkg-config \
    git \
    bash \
    curl \
    wget \
    zip \
    unzip \
    vim \
    nano \
    && rm -rf /var/lib/apt/lists/*

# กำหนดโฟลเดอร์ทำงานภายใน Container (เหมือนเวลาเราเปิด Terminal แล้วสั่ง cd เข้าไปรอไว้)
WORKDIR /workspace

# ก๊อปปี้ไฟล์โปรแกรมที่ Build เสร็จแล้ว (จากเครื่องเรา) เข้าไปไว้ใน Image
# ตัวอย่าง: ถ้า Build เสร็จแล้วไฟล์อยู่ในโฟลเดอร์ output/app โดยที่นับจากตำแหน่งของ Dockerfile
COPY build/main /usr/local/bin/power-tag



# คำสั่งรันเริ่มต้น (รันสคริปต์หลักของโปรเจกต์)
CMD ["/workspace/run.sh"]


# ==========================================
# วิธีสร้าง Docker Image (Build):
# docker build -t ucmake-dev-env .
# (อย่าลืมจุด . ข้างหลังสุดด้วยนะ)
#
# วิธีรัน Container (Run):
# docker run -d \
#   --name container_name \
#   -v C:\code\HelloCpp:/workspace \
#   -w /workspace \
#   ucmake-dev-env
#
# หมายเหตุ: ตรง -v ให้แก้ Path C:\code\HelloCpp เป็นโฟลเดอร์โค้ดของคุณ
# ==========================================


# ==========================================
# วิธีเอาขึ้น Docker Hub (Publish):
# 1. ตั้งชื่อ Tag ใหม่ให้ตรงกับ Username ของเรา
# docker tag ucmake-dev-env your_username/ucmake-dev-env:latest
#
# 2. ล็อกอิน
# docker login
#
# 3. อัปโหลด
# docker push your_username/ucmake-dev-env:latest
# ==========================================
