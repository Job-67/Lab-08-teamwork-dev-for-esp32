#ifndef SENSOR_H
#define SENSOR_H

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ADC Configuration
#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_0  // GPIO36
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12
#define ADC_SAMPLES 64

// Sensor data structure
typedef struct {
    uint32_t adc_value;
    float voltage;
    float temperature;  // If using temperature sensor
} sensor_data_t;

// Function declarations
esp_err_t sensor_init(void);
esp_err_t sensor_read(sensor_data_t *data);
void sensor_task(void *pvParameters);
bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);

// Queue for sensor data
extern QueueHandle_t sensor_queue;

#endif // SENSOR_H
