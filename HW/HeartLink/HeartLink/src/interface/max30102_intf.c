#include "esp_rom_sys.h"
#include "max30102_intf.h"
#include "max30102_driver/max30102_driver.h"
#include "max30102_hal/hal_interrupt.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "MAX_INTF";

// Handle for the data queue to send samples to the upload task
static QueueHandle_t dataQueueHandle = NULL;

// Handle for the sensor task
static TaskHandle_t sensorTaskHandle = NULL;

// Flag to control the task loop
static volatile bool isRunning = false;

/**
 * @brief Task to continuously read sensor data triggered by interrupts.
 */
static void sensorTask(void *arg) {
    // Start the sensor session with an infinite duration (or very long)
    max30102StartSession(0xFFFFFFFF); 

    max30102Sample currentSample;

    ESP_LOGI(TAG, "Sensor task started (Polling Mode active)");

    int continuous_read_errors = 0; // Consecutive error counter
    const int MAX_ERRORS = 5;       // Maximum consecutive errors

    while (isRunning) {
        // No longer waiting for hardware interrupt (interruptWaitEvent)
        // Instead, actively delay 10ms (corresponds to 100Hz sample rate)
        vTaskDelay(pdMS_TO_TICKS(10)); 

        // Directly attempt to read data
        esp_err_t ret = max30102ReadSample(&currentSample);
        
        if (ret == ESP_OK) {
            continuous_read_errors = 0; // Successful read, reset error count
            // Data read successfully, send to queue
            if (dataQueueHandle != NULL) {
                if (xQueueSend(dataQueueHandle, &currentSample, 0) != pdTRUE) {
                    // Queue full, drop old data to prevent blocking
                    ESP_LOGW(TAG, "Queue full. Sample R:%u, IR:%u", 
                             (unsigned int)currentSample.red, (unsigned int)currentSample.ir);
                } else {
                    ESP_LOGD(TAG, "Sample sent. R:%u, IR:%u", 
                             (unsigned int)currentSample.red, (unsigned int)currentSample.ir);
                }
            }
        }
        else if (ret == ESP_ERR_NOT_FOUND) {
            // No new sample, ignore, continue loop
        }
        else {
            ESP_LOGE(TAG, "Sensor Read FAILED with error: 0x%x (%s)", ret, esp_err_to_name(ret));
            continuous_read_errors++;
            if (continuous_read_errors >= MAX_ERRORS) {
                ESP_LOGE(TAG, "!!! MAX30102 CRITICAL FAILURE: %d consecutive I2C errors. Stopping task.", MAX_ERRORS);
                isRunning = false; // Task exits itself upon reaching max errors
                break;
            }
            // Delay 50ms before retrying after failure, giving the I2C bus a chance to recover
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    // Stop the sensor hardware session
    // max30102StopSession();
    
    ESP_LOGI(TAG, "Sensor task finished gracefully. Waiting for deletion.");
    
    // Task exits loop, enters infinite block loop, awaiting external deletion
    while(true) vTaskDelay(pdMS_TO_TICKS(100));
}

esp_err_t max30102IntfInit(QueueHandle_t queue) {
    if (queue == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    dataQueueHandle = queue;

    // Initialize the MAX30102 Driver
    return max30102Init();
}

esp_err_t max30102IntfStart(void) {
    if (sensorTaskHandle != NULL) {
        // ESP_LOGW(TAG, "Task already running");
        esp_rom_printf("Task already running\n");
        return ESP_FAIL;
    }
    
    // Re-initialize interrupt system on every start
    if (interruptInit() != ESP_OK) {
        ESP_LOGE(TAG, "Interrupt re-initialization failed!");
        return ESP_FAIL;
    }

    isRunning = true;
    // Create the task with relatively high priority to handle interrupts quickly
    BaseType_t ret = xTaskCreate(sensorTask, "sensorTask", 6144, NULL, configMAX_PRIORITIES - 1, &sensorTaskHandle);
    
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}

void max30102IntfStop(void) {
    isRunning = false;
    // External task deletion and handle clearing
    if (sensorTaskHandle != NULL) {
        // Wait for task to exit while loop and enter the block state
        vTaskDelay(pdMS_TO_TICKS(50)); 
        vTaskDelete(sensorTaskHandle); 
        sensorTaskHandle = NULL;
    }
    interruptDeinit();
}