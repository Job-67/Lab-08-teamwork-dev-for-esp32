# Sensor Component

This component provides ADC sensor reading functionality for ESP32.

## Features

- ADC reading with calibration
- Multiple sample averaging for accuracy
- Voltage conversion
- Temperature calculation (for LM35 sensor)
- FreeRTOS queue for data communication
- Task-based operation

## Usage

### Initialization
```c
esp_err_t sensor_init(void);
```

### Reading sensor data
```c
sensor_data_t data;
esp_err_t sensor_read(&data);
```

### Using the sensor task
```c
xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
```

### Receiving data from queue
```c
sensor_data_t sensor_data;
if (xQueueReceive(sensor_queue, &sensor_data, pdMS_TO_TICKS(5000)) == pdTRUE) {
    // Process sensor data
}
```

## Configuration

- ADC Channel: ADC1_CHANNEL_0 (GPIO36)
- ADC Attenuation: 11dB (0-3.3V range)
- ADC Width: 12-bit
- Sample averaging: 64 samples

## Hardware Connection

Connect your sensor (e.g., LM35 temperature sensor) to:
- VCC: 3.3V
- GND: GND
- Signal: GPIO36 (ADC1_CHANNEL_0)

## Data Structure

```c
typedef struct {
    uint32_t adc_value;    // Raw ADC value
    float voltage;         // Voltage in volts
    float temperature;     // Temperature in Celsius
} sensor_data_t;
```
