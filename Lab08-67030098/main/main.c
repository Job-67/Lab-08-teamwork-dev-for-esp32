#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sensor.h"
#include "display.h"
#include "led.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Lab08-67030098 application");
    
    // Initialize sensor
    if (sensor_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return;
    }
    
    // Initialize display
    display_init();
    ESP_LOGI(TAG, "Display initialized");
    
    // Initialize LED
    led_init();
    ESP_LOGI(TAG, "LED initialized");
    
    // Create sensor task
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    
    // Main loop to read sensor data and display
    sensor_data_t sensor_data;
    
    while (1) {
        // Wait for sensor data from queue
        if (xQueueReceive(sensor_queue, &sensor_data, pdMS_TO_TICKS(5000)) == pdTRUE) {
            ESP_LOGI(TAG, "Received sensor data:");
            ESP_LOGI(TAG, "  ADC Value: %lu", sensor_data.adc_value);
            ESP_LOGI(TAG, "  Voltage: %.2fV", sensor_data.voltage);
            ESP_LOGI(TAG, "  Temperature: %.2f°C", sensor_data.temperature);
            
            // Display sensor data on screen
            display_clear_screen();
            
            // Show temperature and voltage as numeric data
            display_show_data(sensor_data.temperature, sensor_data.voltage);
            
            // Show additional info as message
            char info_str[64];
            snprintf(info_str, sizeof(info_str), "ADC: %lu", sensor_data.adc_value);
            display_show_message(info_str);
            
            // Control LED based on temperature
            if (sensor_data.temperature > 30.0) {
                led_on();  // Turn on LED if temperature is high
                ESP_LOGI(TAG, "Temperature high - LED ON");
            } else {
                led_off(); // Turn off LED if temperature is normal
                ESP_LOGI(TAG, "Temperature normal - LED OFF");
            }
        } else {
            ESP_LOGW(TAG, "No sensor data received");
        }
    }
}
