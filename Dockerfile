FROM ubuntu:latest

# ปิดการถาม Interactive
ENV DEBIAN_FRONTEND=noninteractive

# (เลือกเปิดส่วนนี้หากรันแล้วเจอ Error ว่าหา Library .so ไม่เจอ)
# RUN apt-get update && apt-get install -y libsqlite3-0 libssl3 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# ก๊อปปี้ไฟล์จากเครื่อง Host เข้าไปใน Container
COPY build/main /usr/local/bin/power-tag
COPY run.sh /workspace/run.sh
COPY iPM2xxx.db /workspace/iPM2xxx.db
COPY iA9MEM15.db /workspace/iA9MEM15.db
COPY .env /workspace/.env

# ให้สิทธิ์การรันเฉพาะไฟล์โปรแกรมและสคริปต์
RUN chmod +x /usr/local/bin/power-tag
RUN chmod +x /workspace/run.sh

# สิทธิ์การเขียนไฟล์สำหรับ Database (สำคัญมากเพื่อให้ SQLite Insert ได้)
RUN chmod 666 /workspace/iPM2xxx.db
RUN chmod 666 /workspace/iA9MEM15.db

# ตั้งค่า Token เป็น Environment Variable (สามารถเปลี่ยนตอน docker run ได้)
ENV TB_TOKEN="pEgujBQots9Mq3pQY6lQ"

# สั่งรันผ่าน run.sh
CMD ["/bin/bash", "/workspace/run.sh"]