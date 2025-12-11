#include "esp_rom_sys.h"
#include "upload_intf.h"
#include "net_components/net/transport_http.h"
#include "common/max30102_types.h"
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <string.h>
#include <time.h>
#include "interface/wifi_intf.h"

static const char *TAG = "UPLOAD_INTF";

// [CONFIGURATION]
// Sample Rate: 100Hz
// Batch Window: 4 Seconds
// Batch Size: 100 * 4 = 400 samples
#define BATCH_SIZE 400 
#define QUEUE_TIMEOUT_MS 5000

// Device Identity (Should match your Cloud backend configuration)
#define USER_ID   "USER_1"
#define DEVICE_ID "DEV_1"

static QueueHandle_t sourceQueueHandle = NULL;
static TaskHandle_t uploadTaskHandle = NULL;
static volatile bool isUploadRunning = false;

/**
 * @brief Task to batch raw sensor data and upload via HTTP.
 * This implements the "Cloud Stitching" strategy.
 */
static void uploadTask(void *arg) {
    // Buffer to hold raw samples. 
    // Memory usage: 400 * 8 bytes = 3200 bytes.
    // This is allocated on the Task Stack, so we need a large stack size (see uploadIntfStart).
    max30102Sample batchBuffer[BATCH_SIZE];
    int sampleCount = 0;

    // ESP_LOGI(TAG, "Upload task started (Raw Data Mode, 4s Window). Waiting 2s for stabilization...");
    esp_rom_printf("Upload task started (Raw Data Mode, 4s Window). Waiting 2s for stabilization...\n");
    // Wait for the user to place their finger properly and for the signal to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (isUploadRunning) {
        max30102Sample singleSample;

        if (sampleCount % 100 == 0) {
                 esp_rom_printf("INFO: Sample Count: %d\n", sampleCount); 
            }
        // 1. Receive data from the queue (Blocking with timeout)
        if (xQueueReceive(sourceQueueHandle, &singleSample, pdMS_TO_TICKS(QUEUE_TIMEOUT_MS)) == pdTRUE) {
            // Store data in the local batch buffer
            batchBuffer[sampleCount++] = singleSample;

            // 2. Check if the batch is full (400 samples / 4 seconds collected)
            if (sampleCount >= BATCH_SIZE) {
                // ESP_LOGI(TAG, "Batch full (%d samples), packing JSON...", sampleCount);
                esp_rom_printf("Batch full (%d samples), packing JSON...\n", sampleCount);

                // Get current timestamp (Start time of this batch)
                // The http_client_common.c also adds a timestamp header, but putting it in the body
                // helps the cloud sort and stitch multiple packets accurately.
                time_t now;
                time(&now);

                // 3. Construct JSON Object
                // Structure:
                // {
                //   "user_id": "...",
                //   "device_id": "...",
                //   "timestamp": 1234567890,
                //   "samples": [ 
                //      {"ir": 12345, "red": 67890}, 
                //      ... (400 items)
                //   ]
                // }
                cJSON *root = cJSON_CreateObject();
                cJSON_AddStringToObject(root, "user_id", USER_ID);
                cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
                cJSON_AddNumberToObject(root, "timestamp", (double)now);

                cJSON *dataArray = cJSON_CreateArray();
                cJSON_AddItemToObject(root, "samples", dataArray);

                // Loop through buffer and add all raw samples to the array
                for (int i = 0; i < sampleCount; i++) {
                    cJSON *item = cJSON_CreateObject();
                    // IR is critical for HRV; RED is for SpO2. Uploading both allows full analysis.
                    cJSON_AddNumberToObject(item, "ir", batchBuffer[i].ir);
                    cJSON_AddNumberToObject(item, "red", batchBuffer[i].red);
                    cJSON_AddItemToArray(dataArray, item);
                }

                // 4. Serialize JSON to string
                // Note: A 400-sample JSON will be approx 15KB in size.
                char *jsonString = cJSON_PrintUnformatted(root);
                
                // 5. Send Data
                if (jsonString != NULL) {
                    // Debug: Print payload size
                    // ESP_LOGI(TAG, "Payload generated (%d bytes). Uploading...", strlen(jsonString));
                    esp_rom_printf("Payload generated (%d bytes). Uploading...\n", strlen(jsonString));
                    
                    // Uncomment the line below to inspect raw JSON (Warning: It will be huge)
                    // ESP_LOGI(TAG, "Payload content: %s", jsonString);

                    if (wifiIntfIsConnected()) {
                        // transportHttpSend handles the HTTPS POST request
                        // It automatically adds 'x-api-key' and 'timestamp' headers via http_client_common.c
                        if (transportHttpSend((uint8_t *)jsonString, strlen(jsonString))) {
                            // ESP_LOGI(TAG, "Upload Success");
                            esp_rom_printf("Upload Success\n");
                        } else {
                            // ESP_LOGE(TAG, "Upload Failed (Check Network/Server)");
                            esp_rom_printf("Upload Failed (Check Network/Server)\n");
                        }
                    } else {
                        // ESP_LOGW(TAG, "Network NOT ready (Simulation Mode) - Skipping Send");
                        esp_rom_printf("Network NOT ready (Simulation Mode) - Skipping Send\n");
                    }
                    
                    // Critical: Free the memory allocated by cJSON_Print
                    free(jsonString);
                } else {
                    // ESP_LOGE(TAG, "Failed to allocate memory for JSON string");
                    esp_rom_printf("Failed to allocate memory for JSON string\n");
                }

                // 6. Cleanup
                cJSON_Delete(root);
                sampleCount = 0; // Reset counter for the next batch
            }
        }
    }
    
    ESP_LOGI(TAG, "Upload task stopping...");
    
    // Task exits loop, enters infinite block loop, awaiting external deletion
    while(true) vTaskDelay(pdMS_TO_TICKS(100));
}

esp_err_t uploadIntfInit(QueueHandle_t queue) {
    if (queue == NULL) return ESP_ERR_INVALID_ARG;
    sourceQueueHandle = queue;
    return ESP_OK;
}

esp_err_t uploadIntfStart(void) {
    if (uploadTaskHandle != NULL) return ESP_FAIL;

    isUploadRunning = true;

    BaseType_t ret = xTaskCreate(uploadTask, "uploadTask", 12288, NULL, 3, &uploadTaskHandle);
    
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}

void uploadIntfStop(void) {
    isUploadRunning = false;

    // External task deletion and handle clearing
    if (uploadTaskHandle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(50));
        vTaskDelete(uploadTaskHandle);
        uploadTaskHandle = NULL;
    }
}