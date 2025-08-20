#include "sensor.h"
#include "esp_log.h"

static const char *TAG = "SENSOR";

// ADC handles
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle = NULL;
static bool do_calibration1 = false;

// Queue for sensor data
QueueHandle_t sensor_queue = NULL;

esp_err_t sensor_init(void)
{
    esp_err_t ret = ESP_OK;
    
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));

    //-------------ADC1 Calibration Init---------------//
    do_calibration1 = example_adc_calibration_init(ADC_UNIT, ADC_ATTEN, &adc1_cali_handle);
    
    // Create queue for sensor data
    sensor_queue = xQueueCreate(10, sizeof(sensor_data_t));
    if (sensor_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create sensor queue");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Sensor initialized successfully");
    return ret;
}

bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = ADC_CHANNEL,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

esp_err_t sensor_read(sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int adc_raw = 0;
    int voltage = 0;
    
    // Take multiple samples and average them
    for (int i = 0; i < ADC_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));
        adc_raw += adc_raw;
    }
    adc_raw /= ADC_SAMPLES;
    
    data->adc_value = adc_raw;
    
    if (do_calibration1) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage));
        data->voltage = (float)voltage / 1000.0; // Convert to volts
    } else {
        data->voltage = (float)adc_raw * 3.3 / 4095.0; // Simple conversion
    }
    
    // Convert voltage to temperature (example for LM35 sensor)
    // LM35: 10mV/°C, so temperature = voltage * 100
    data->temperature = data->voltage * 100.0;
    
    ESP_LOGI(TAG, "ADC: %d, Voltage: %.2fV, Temperature: %.2f°C", 
             data->adc_value, data->voltage, data->temperature);
    
    return ESP_OK;
}



void sensor_task(void *pvParameters)
{
    sensor_data_t sensor_data;
    
    while (1) {
        if (sensor_read(&sensor_data) == ESP_OK) {
            // Send data to queue
            if (xQueueSend(sensor_queue, &sensor_data, pdMS_TO_TICKS(100)) != pdTRUE) {
                ESP_LOGW(TAG, "Failed to send data to queue");
            }
        }
        
        // Wait for 1 second before next reading
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
