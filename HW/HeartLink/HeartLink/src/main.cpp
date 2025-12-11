#include <Arduino.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/queue.h"

  #include "nvs_flash.h"
  #include "esp_err.h"
  #include "esp_log.h"
}

#include "common/max30102_types.h"
#include "interface/wifi_intf.h"
#include "interface/upload_intf.h"
#include "interface/max30102_intf.h"

#ifndef UNIT_TEST

static const char *TAG = "APP_MAIN";

// global queue and state
static QueueHandle_t sensorDataQueue = nullptr;
static systemState currentState = SYS_STATE_IDLE;

void heartlinkTask(void *pvParameters)
{
    char rx_buffer[128];

    ESP_LOGI(TAG, "System Initialized. Waiting for cloud command...");

    while (true) {
        // get the command from cloud
        if (wifiIntfCheckCommand(rx_buffer, sizeof(rx_buffer))) {

            Serial.print("Cloud Response: ");
            Serial.println(rx_buffer);

            // start
            if (strstr(rx_buffer, "\"start\"") != nullptr) {
                if (currentState == SYS_STATE_IDLE) {
                    ESP_LOGI(TAG, ">>> START command received <<<");
                    max30102IntfStart();
                    uploadIntfStart();
                    currentState = SYS_STATE_RUNNING;
                }
            }
            // stop
            else if (strstr(rx_buffer, "\"stop\"") != nullptr) {
                if (currentState == SYS_STATE_RUNNING) {
                    ESP_LOGI(TAG, ">>> STOP command received <<<");
                    max30102IntfStop();
                    uploadIntfStop();
                    if (sensorDataQueue != nullptr) {
                        xQueueReset(sensorDataQueue);
                    }
                    currentState = SYS_STATE_IDLE;
                }
            }
        }

        // state machine
        switch (currentState) {
            case SYS_STATE_RUNNING:
                vTaskDelay(pdMS_TO_TICKS(3000));  // check stop every 3 seconds
                break;

            case SYS_STATE_IDLE:
            default:
                ESP_LOGI(TAG, "Status: IDLE (Polling cloud...)");
                vTaskDelay(pdMS_TO_TICKS(5000));  // check start every 5 seconds
                break;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_HTTP", ESP_LOG_VERBOSE);
    delay(1000);

    Serial.println();
    Serial.println(F("\n\n*** FIRMWARE RELOADED - VERSION 3.0 (Arduino) ***\n"));

    // Scan WiFi
    Serial.println("Scanning WiFi networks...");
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    Serial.println("Scan done.");
    if (n == 0) {
        Serial.println("No networks found!");
    } else {
        Serial.printf("%d networks found:\n", n);
        for (int i = 0; i < n; ++i) {
            Serial.printf("  %d: [%s] (%d) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), 
                          (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");
        }
    }
    Serial.println("-----------------------------");

    // 1. initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. create queue
    sensorDataQueue = xQueueCreate(200, sizeof(max30102Sample));
    if (sensorDataQueue == nullptr) {
        ESP_LOGE(TAG, "Failed to create data queue");
        Serial.println(F("ERROR: Failed to create data queue, halting."));
        while (true) {
            delay(1000);
        }
    }

    // 3. connect WiFi (retry with blocking)
    Serial.println("Connecting WiFi...");
    if (wifiIntfConnect() != ESP_OK) {
        Serial.println("WiFi Connect Failed! (Checking NVS/Password/Signal)");
        Serial.println("System will continue in OFFLINE mode.");
    } else {
        Serial.println("WiFi Connected!");
    }

    // 4. initialize interface layer
    Serial.println("Waiting for system stability after network sync...");
    delay(500);

    if (uploadIntfInit(sensorDataQueue) != ESP_OK) {
        Serial.println("Upload Init Failed");
    }
    esp_err_t sensorRet = max30102IntfInit(sensorDataQueue);
    if (sensorRet != ESP_OK) {
        Serial.printf("Sensor Init Failed: 0x%x. System continues without sensor.\n", sensorRet);
    } else {
        Serial.println("Sensor Init Success");
    }

    // 5. create task
    xTaskCreate(
        heartlinkTask,
        "heartlinkTask",
        8192,       // stack size
        nullptr,
        1,          // priority
        nullptr
    );

}

void loop()
{
    delay(1000);
}

#endif