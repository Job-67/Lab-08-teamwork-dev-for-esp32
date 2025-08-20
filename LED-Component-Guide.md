# LED Component Setup Guide - Lab-08 Teamwork

## 📋 Overview
คู่มือการสร้าง LED component สำหรับ Lab-08 teamwork development โดยใช้ ESP-IDF และเชื่อมต่อกับ GitHub repository แบบออนไลน์

## 🎯 Prerequisites
- ESP-IDF v6.0 หรือใหม่กว่า
- Docker (สำหรับ ESP-IDF environment)
- Git
- GitHub account

## 📁 Project Structure
```
Lab-08-teamwork-dev-for-esp32/
├── docker-compose.yml
├── led/                          # LED component
│   ├── CMakeLists.txt
│   ├── include/led.h
│   ├── led.c
│   └── README.md
├── Lab08-6703xxxx/              # Main project
│   ├── CMakeLists.txt
│   ├── main/
│   │   ├── CMakeLists.txt
│   │   ├── main.c
│   │   └── idf_component.yml
│   └── README.md
└── README.md
```

## 🚀 Step-by-Step Instructions

### Step 1: Setup Docker Environment
```bash
# 1. สร้าง docker-compose.yml
cat > docker-compose.yml << 'EOF'
services:
  esp32-dev:
    image: espressif/idf:latest
    container_name: esp32-lab8-led
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
EOF

# 2. เริ่ม Docker container
docker-compose up -d

# 3. เข้าไปใน container
docker-compose exec -it esp32-dev bash

# 4. Export ESP-IDF environment
. $IDF_PATH/export.sh
```

### Step 2: Create LED Component Structure
```bash
# สร้างโฟลเดอร์ LED component
mkdir -p led/include
```

### Step 3: Create LED Component Files

#### 3.1 LED Component CMakeLists.txt
```bash
cat > led/CMakeLists.txt << 'EOF'
idf_component_register(
    SRCS "led.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
)
EOF
```

#### 3.2 LED Header File
```bash
cat > led/include/led.h << 'EOF'
#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// LED Configuration
#define LED_GPIO GPIO_NUM_2  // Built-in LED on most ESP32 boards
#define LED_ON  0            // Active low
#define LED_OFF 1

// LED states
typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON,
    LED_STATE_BLINK,
    LED_STATE_PULSE
} led_state_t;

// Function declarations
esp_err_t led_init(void);
esp_err_t led_set_state(led_state_t state);
esp_err_t led_toggle(void);
esp_err_t led_on(void);
esp_err_t led_off(void);
void led_blink_task(void *pvParameters);
void led_pulse_task(void *pvParameters);

#endif // LED_H
EOF
```

#### 3.3 LED Implementation File
```bash
cat > led/led.c << 'EOF'
#include "led.h"

static const char *TAG = "LED";
static led_state_t current_state = LED_STATE_OFF;
static TaskHandle_t led_task_handle = NULL;

esp_err_t led_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LED GPIO");
        return ret;
    }
    
    // Turn off LED initially
    gpio_set_level(LED_GPIO, LED_OFF);
    current_state = LED_STATE_OFF;
    
    ESP_LOGI(TAG, "LED initialized successfully on GPIO %d", LED_GPIO);
    return ESP_OK;
}

esp_err_t led_set_state(led_state_t state)
{
    esp_err_t ret = ESP_OK;
    
    // Stop existing task if any
    if (led_task_handle != NULL) {
        vTaskDelete(led_task_handle);
        led_task_handle = NULL;
    }
    
    switch (state) {
        case LED_STATE_OFF:
            gpio_set_level(LED_GPIO, LED_OFF);
            break;
            
        case LED_STATE_ON:
            gpio_set_level(LED_GPIO, LED_ON);
            break;
            
        case LED_STATE_BLINK:
            xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 5, &led_task_handle);
            break;
            
        case LED_STATE_PULSE:
            xTaskCreate(led_pulse_task, "led_pulse", 2048, NULL, 5, &led_task_handle);
            break;
            
        default:
            ret = ESP_ERR_INVALID_ARG;
            break;
    }
    
    if (ret == ESP_OK) {
        current_state = state;
        ESP_LOGI(TAG, "LED state changed to %d", state);
    }
    
    return ret;
}

esp_err_t led_toggle(void)
{
    static bool led_status = false;
    led_status = !led_status;
    gpio_set_level(LED_GPIO, led_status ? LED_ON : LED_OFF);
    return ESP_OK;
}

esp_err_t led_on(void)
{
    return led_set_state(LED_STATE_ON);
}

esp_err_t led_off(void)
{
    return led_set_state(LED_STATE_OFF);
}

void led_blink_task(void *pvParameters)
{
    while (1) {
        gpio_set_level(LED_GPIO, LED_ON);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, LED_OFF);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void led_pulse_task(void *pvParameters)
{
    while (1) {
        // Fade in
        for (int i = 0; i < 10; i++) {
            gpio_set_level(LED_GPIO, LED_ON);
            vTaskDelay(pdMS_TO_TICKS(50 * i));
            gpio_set_level(LED_GPIO, LED_OFF);
            vTaskDelay(pdMS_TO_TICKS(50 * (10 - i)));
        }
        
        // Fade out
        for (int i = 10; i > 0; i--) {
            gpio_set_level(LED_GPIO, LED_ON);
            vTaskDelay(pdMS_TO_TICKS(50 * i));
            gpio_set_level(LED_GPIO, LED_OFF);
            vTaskDelay(pdMS_TO_TICKS(50 * (10 - i)));
        }
    }
}
EOF
```

#### 3.4 LED Component README
```bash
cat > led/README.md << 'EOF'
# LED Component

ESP32 LED control component with multiple states and effects.

## Features
- LED ON/OFF control
- Blinking mode
- Pulse/fade effect
- GPIO configuration
- Task-based operation

## Usage

### Initialization
```c
esp_err_t led_init(void);
```

### Control Functions
```c
esp_err_t led_set_state(led_state_t state);
esp_err_t led_toggle(void);
esp_err_t led_on(void);
esp_err_t led_off(void);
```

### LED States
- `LED_STATE_OFF` - Turn off LED
- `LED_STATE_ON` - Turn on LED
- `LED_STATE_BLINK` - Blinking mode
- `LED_STATE_PULSE` - Pulse/fade effect

## Hardware Connection
- LED connected to GPIO 2 (built-in LED on most ESP32 boards)
- Active low configuration (GPIO low = LED on)

## Example
```c
// Initialize LED
led_init();

// Turn on LED
led_on();

// Start blinking
led_set_state(LED_STATE_BLINK);

// Turn off LED
led_off();
```
EOF
```

### Step 4: Create Main Project Structure
```bash
# สร้างโฟลเดอร์โปรเจคหลัก (เปลี่ยนรหัสนักศึกษา)
mkdir -p Lab08-6703xxxx/main
```

### Step 5: Create Main Project Files

#### 5.1 Main Project CMakeLists.txt
```bash
cat > Lab08-6703xxxx/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Lab08-6703xxxx)
EOF
```

#### 5.2 Main Component CMakeLists.txt
```bash
cat > Lab08-6703xxxx/main/CMakeLists.txt << 'EOF'
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES led
)
EOF
```

#### 5.3 Main Application File
```bash
cat > Lab08-6703xxxx/main/main.c << 'EOF'
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "led.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Lab08-6703xxxx LED application");
    
    // Initialize LED
    if (led_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED");
        return;
    }
    
    // Demo LED states
    ESP_LOGI(TAG, "LED Demo starting...");
    
    while (1) {
        // Turn on LED
        ESP_LOGI(TAG, "LED ON");
        led_on();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Turn off LED
        ESP_LOGI(TAG, "LED OFF");
        led_off();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Blinking mode
        ESP_LOGI(TAG, "LED BLINKING");
        led_set_state(LED_STATE_BLINK);
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // Pulse mode
        ESP_LOGI(TAG, "LED PULSE");
        led_set_state(LED_STATE_PULSE);
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // Turn off
        led_off();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
EOF
```

### Step 6: Configure Component Dependencies

#### 6.1 Create idf_component.yml
```bash
cat > Lab08-6703xxxx/main/idf_component.yml << 'EOF'
dependencies:
  led:
    git: https://github.com/YOUR_USERNAME/Lab-08-teamwork-dev-for-esp32.git
    path: led
    version: "*"
EOF
```

### Step 7: Create .gitignore Files

#### 7.1 Root .gitignore
```bash
cat > .gitignore << 'EOF'
# ESP-IDF build files
build/
sdkconfig
sdkconfig.old
dependencies.lock

# CMake build files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile

# Compiled files
*.o
*.obj
*.a
*.so
*.elf
*.bin
*.map
EOF
```

#### 7.2 LED Component .gitignore
```bash
cat > led/.gitignore << 'EOF'
# ESP-IDF build files
build/
*.o
*.obj
*.a
*.so
*.elf
*.bin
EOF
```

#### 7.3 Main Project .gitignore
```bash
cat > Lab08-6703xxxx/.gitignore << 'EOF'
# ESP-IDF build files
build/
sdkconfig
sdkconfig.old
dependencies.lock

# CMake build files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile

# Compiled files
*.o
*.obj
*.a
*.so
*.elf
*.bin
*.map
EOF
```

### Step 8: Git Setup and Push to GitHub

```bash
# 1. Initialize Git repository (if not already done)
git init

# 2. Add all files
git add .

# 3. Commit changes
git commit -m "Add LED component and main project structure"

# 4. Add remote repository (replace with your GitHub URL)
git remote add origin https://github.com/YOUR_USERNAME/Lab-08-teamwork-dev-for-esp32.git

# 5. Push to GitHub
git push -u origin main
```

### Step 9: Test the Component

```bash
# 1. Clean build
cd Lab08-6703xxxx
idf.py clean

# 2. Build project
idf.py build

# 3. Flash to ESP32 (optional)
idf.py flash monitor
```

## 🔧 Configuration Notes

### Update GitHub URL
ในไฟล์ `Lab08-6703xxxx/main/idf_component.yml` ให้เปลี่ยน:
```yaml
dependencies:
  led:
    git: https://github.com/YOUR_USERNAME/Lab-08-teamwork-dev-for-esp32.git
    path: led
    version: "*"
```

### Update Project Name
เปลี่ยนชื่อโปรเจคใน `Lab08-6703xxxx/CMakeLists.txt`:
```cmake
project(Lab08-6703xxxx)  # เปลี่ยนเป็นรหัสนักศึกษาของคุณ
```

## 📋 Verification Checklist

- [ ] LED component structure created
- [ ] All source files created and compiled
- [ ] Main project structure created
- [ ] idf_component.yml configured
- [ ] .gitignore files created
- [ ] Git repository initialized
- [ ] Code pushed to GitHub
- [ ] Component can be downloaded from GitHub
- [ ] Project builds successfully
- [ ] LED functions work correctly

## 🚨 Troubleshooting

### Build Errors
- ตรวจสอบ ESP-IDF version (ต้องเป็น v6.0+)
- ตรวจสอบ dependencies ใน CMakeLists.txt
- ตรวจสอบ include paths

### Git Issues
- ตรวจสอบ GitHub URL ใน idf_component.yml
- ตรวจสอบ file permissions
- ตรวจสอบ network connection

### LED Not Working
- ตรวจสอบ GPIO pin configuration
- ตรวจสอบ hardware connection
- ตรวจสอบ power supply

## 📞 Support
หากมีปัญหา ให้ตรวจสอบ:
1. ESP-IDF documentation
2. GitHub repository logs
3. Build output messages
4. Hardware connections
