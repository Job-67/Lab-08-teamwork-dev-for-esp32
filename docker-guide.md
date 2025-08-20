# Docker guide
1. สร้าง docker-compose.yml

```yml
services:
  esp32-dev:
    image: espressif/idf:latest
    container_name: esp32-lab6-1
    volumes:
      - .:/project
    working_dir: /project
    tty: true
    stdin_open: true
    environment:
      - IDF_PATH=/opt/esp/idf
    command: /bin/bash
    networks:
      - esp32-network

networks:
  esp32-network:
    driver: bridge
```
2. เริ่ม docker image

```bash
  docker-compose up -d
```

3. ตรวจสอบชื่อของ docker

```bash
  dosker-compose ps -a 
```
จะเห็นชื่อที่ต้องเอาไปใช้ในคำสั่ง `docker-compose exec` โดยจะใช้ชื่อตรงคอลัมน์ SERVICE


4. เข้าใปใน bash ของ docker

```
docker-compose exec -it [ชื่อที่เห็นจากในข้อ 3]  bash
```
5. export environment variables ของ ESP32

```
. $IDF_PATH/export.sh
```
