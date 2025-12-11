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

static const char *TAG = "TEST_FULLFLOW";

// global queue and state
static QueueHandle_t sensorDataQueue = nullptr;
static systemState currentState = SYS_STATE_IDLE;

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
    if (uploadIntfInit(sensorDataQueue) != ESP_OK) {
        Serial.println("Upload Init Failed");
    }
    esp_err_t sensorRet = max30102IntfInit(sensorDataQueue);
    if (sensorRet != ESP_OK) {
        Serial.printf("Sensor Init Failed: 0x%x. System continues without sensor.\n", sensorRet);
    } else {
        Serial.println("Sensor Init Success");
    }

    Serial.println("\n*** MANUAL DATA ACQUISITION STARTING ***");
    
    // START: 启动采集和上传
    max30102IntfStart();
    uploadIntfStart();
    
    // 延时 4000 毫秒 (4 秒) 进行数据采集
    Serial.println("Collecting data...");
    delay(20000); 
    
    // STOP: 停止采集和上传
    Serial.println("4 seconds complete. Stopping acquisition...");
    max30102IntfStop();
    uploadIntfStop();
    
    // 清空队列 (可选，但推荐)
    if (sensorDataQueue != nullptr) {
        xQueueReset(sensorDataQueue);
    }
    
    Serial.println("*** MANUAL DATA ACQUISITION COMPLETE ***\n");
}

void loop()
{
    delay(1000);
}