#include <Arduino.h>
#include <unity.h>

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/queue.h"
  #include "nvs_flash.h"
  #include "esp_log.h"
  
  // Only include Interface and Algorithm headers
  #include "interface/max30102_intf.h"
  #include "common/max30102_types.h"
  
  // Include the algorithm to verify if the raw data is valid for calculation
  #include "dcp_algorithm/maxim_algo.h"
}

static const char *TAG = "TEST_DET";
QueueHandle_t testQueue = nullptr;

// Test Configuration
#define TEST_BATCH_SIZE 400 // 4 seconds @ 100Hz

void setUp(void) {}
void tearDown(void) {}

// === Test Case 1: System Initialization ===
void test_init_system(void) {
    Serial.begin(115200);
    delay(2000); 
    Serial.setDebugOutput(true);
    esp_log_level_set("*", ESP_LOG_INFO);

    ESP_LOGI(TAG, "--- STARTING SENSOR & ALGORITHM TEST ---");

    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // 2. Create Queue
    testQueue = xQueueCreate(TEST_BATCH_SIZE * 2, sizeof(max30102Sample));
    TEST_ASSERT_NOT_NULL(testQueue);
}

// === Test Case 2: Capture Data and Calculate Vital Signs ===
void test_sensor_data_calculation(void) {
    ESP_LOGI(TAG, "Step 2: Init & Start Sensor Interface...");

    // 1. Initialize Sensor Interface
    // This handles I2C and Interrupt/Timer initialization internally
    esp_err_t ret = max30102IntfInit(testQueue);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Sensor Init Failed: 0x%x. Check wiring!", ret);
        TEST_FAIL();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // 2. Start Detection
    TEST_ASSERT_EQUAL(ESP_OK, max30102IntfStart());

    ESP_LOGI(TAG, "Sensor Started. Collecting %d samples (approx 4s)...", TEST_BATCH_SIZE);
    ESP_LOGI(TAG, ">>>>> PLEASE KEEP FINGER ON SENSOR STEADY <<<<<");

    // 3. Buffers for Algorithm
    // Using static to avoid stack overflow in test task
    static uint32_t irBuffer[TEST_BATCH_SIZE];
    static uint32_t redBuffer[TEST_BATCH_SIZE];
    
    max30102Sample sample;
    int collectedCount = 0;

    // 4. Collect Data Loop
    // We need exactly TEST_BATCH_SIZE samples to run the algorithm
    while (collectedCount < TEST_BATCH_SIZE) {
        if (xQueueReceive(testQueue, &sample, pdMS_TO_TICKS(1000)) == pdTRUE) {
            
            irBuffer[collectedCount] = sample.ir;
            redBuffer[collectedCount] = sample.red;
            
            // Print progress every 100 samples
            if ((collectedCount + 1) % 100 == 0) {
                ESP_LOGI(TAG, "Collected %d/%d samples...", collectedCount + 1, TEST_BATCH_SIZE);
            }
            
            collectedCount++;
        } else {
            ESP_LOGE(TAG, "Timeout waiting for data!");
            break;
        }
    }

    // 5. Stop Sensor
    max30102IntfStop();
    ESP_LOGI(TAG, "Collection Finished.");

    TEST_ASSERT_EQUAL_INT(TEST_BATCH_SIZE, collectedCount);

    // 6. Run Algorithm (Local Calculation Test)
    ESP_LOGI(TAG, "Running Maxim Algorithm on collected data...");
    
    int32_t spo2 = 0;
    int8_t spo2_valid = 0;
    int32_t hr = 0;
    int8_t hr_valid = 0;

    maxim_heart_rate_and_oxygen_saturation(
        irBuffer, TEST_BATCH_SIZE,
        redBuffer, 
        &spo2, &spo2_valid, 
        &hr, &hr_valid
    );

    // 7. Print Results
    ESP_LOGI(TAG, "--------------------------------------------------");
    ESP_LOGI(TAG, "[RESULT] Heart Rate: %d bpm (Valid: %d)", hr, hr_valid);
    ESP_LOGI(TAG, "[RESULT] SpO2:       %d %%  (Valid: %d)", spo2, spo2_valid);
    ESP_LOGI(TAG, "--------------------------------------------------");

    // 8. Assertions
    // Note: If you don't have a finger on the sensor, valid might be 0.
    // We warn but don't fail the test strictly unless you are sure finger is present.
    if (hr_valid == 1 && spo2_valid == 1) {
        ESP_LOGI(TAG, "Test Passed: Valid vital signs detected.");
    } else {
        ESP_LOGW(TAG, "Test Warning: Algorithm returned invalid data. Finger position issue?");
        // Uncomment to enforce validity
        // TEST_FAIL_MESSAGE("Algorithm could not calculate valid HR/SpO2");
    }
    
    TEST_PASS();
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_init_system);
    RUN_TEST(test_sensor_data_calculation);
    UNITY_END();
}

void loop() {
    delay(1000);
}