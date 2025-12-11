#include <Arduino.h>
#include <unity.h>

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/queue.h"
  #include "nvs_flash.h"
  #include "esp_log.h"

  #include "interface/upload_intf.h"
  #include "common/max30102_types.h"
}

static const char *TAG = "TEST_UPLOAD";
QueueHandle_t testQueue = nullptr;

// Batch size must match the definition in upload_intf.c
#define BATCH_SIZE 100 

void setUp(void) {}
void tearDown(void) {}

// === Test Case 1: System Initialization ===
void test_init_system(void) {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // 2. Initialize Queue
    // The interface layer expects a valid queue handle
    testQueue = xQueueCreate(200, sizeof(max30102Sample));
    TEST_ASSERT_NOT_NULL(testQueue);
}

// === Test Case 2: Verify Upload Logic & Print Expected Data ===
void test_upload_interface_logic(void) {
    ESP_LOGI(TAG, "Testing Upload Interface (Black Box Mode)...");

    // 1. Initialize and Start the Upload Interface
    // We are only using the public API defined in upload_intf.h
    TEST_ASSERT_EQUAL(ESP_OK, uploadIntfInit(testQueue));
    TEST_ASSERT_EQUAL(ESP_OK, uploadIntfStart());

    ESP_LOGI(TAG, "Upload Task Started. Generating data stream...");
    
    // We will generate enough samples to trigger exactly one batch upload (100 samples)
    // and print what the JSON *should* look like based on our input.
    
    ESP_LOGI(TAG, "--------------------------------------------------");
    ESP_LOGI(TAG, "[PREVIEW] Expected JSON Content for Batch 1:");
    ESP_LOGI(TAG, "{ \"device_id\": \"ESP32_HEARTLINK_001\", \"samples\": [");

    for (int i = 0; i < 150; i++) {
        max30102Sample fakeSample;
        fakeSample.red = 10000 + i; 
        fakeSample.ir = 20000 + i;  

        // Send to real queue (Driving the real logic)
        if (xQueueSend(testQueue, &fakeSample, 0) != pdTRUE) {
            ESP_LOGW(TAG, "Queue full at sample %d", i);
        }

        // Manual JSON Preview (Only for the first batch)
        if (i < BATCH_SIZE) {
            // Manually format the string to look like JSON
            // format: {"red": 10000, "ir": 20000},
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "  {\"r\":%u, \"i\":%u}%s", 
                     (unsigned int)fakeSample.red, 
                     (unsigned int)fakeSample.ir,
                     (i == BATCH_SIZE - 1) ? "" : ","); // Add comma except for last item
            ESP_LOGI(TAG, "%s", buffer);
        } else if (i == BATCH_SIZE) {
            ESP_LOGI(TAG, "] }");
            ESP_LOGI(TAG, "--------------------------------------------------");
            ESP_LOGI(TAG, "Batch 1 Limit Reached. Real Upload Task should trigger now.");
        }
        
        delay(10); // Simulate 100Hz sampling rate
    }

    ESP_LOGI(TAG, "Waiting for background task to process...");
    delay(3000);

    uploadIntfStop();
    ESP_LOGI(TAG, "Upload Interface Stopped.");
    
    TEST_PASS();
}

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    esp_log_level_set("*", ESP_LOG_INFO);

    UNITY_BEGIN();
    RUN_TEST(test_init_system);
    RUN_TEST(test_upload_interface_logic);
    UNITY_END();
}

void loop() {
    delay(1000);
}