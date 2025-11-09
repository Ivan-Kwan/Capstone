#include "max30102_driver.h"
#include "hal_max30102_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "MAX30102_DRIVER";
static volatile bool g_running = false;
static TimerHandle_t g_stopTimer = NULL;
static SemaphoreHandle_t g_lock;

/* Default LED currents (approx. 7 mA) – adjust if needed */
#define MAX30102_DEFAULT_LED_CURRENT   (0x24)

esp_err_t max30102Init(void) {
    esp_err_t ret;

    // Basic sensor bring-up
    ret = max30102SensorInit();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "SensorInit failed"); return ret; }

    // FIFO config + clean
    ret = max30102FifoInit();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "FifoInit failed"); return ret; }

    ret = max30102ResetFifo();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ResetFifo failed"); return ret; }

    // Default mode & LED currents
    ret = max30102SetMode(MODE_SPO2);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "SetMode failed"); return ret; }

    ret = max30102SetLedCurrent(MAX30102_DEFAULT_LED_CURRENT, MAX30102_DEFAULT_LED_CURRENT);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "SetLedCurrent failed"); return ret; }

    ESP_LOGI(TAG, "MAX30102 initialized with default config");
    return ESP_OK;
}

esp_err_t max30102Configure(sensorMode mode, uint8_t redCurrent, uint8_t irCurrent) {
    esp_err_t ret;
    ret = max30102SetMode(mode);
    if (ret != ESP_OK) return ret;
    return max30102SetLedCurrent(redCurrent, irCurrent);
}

esp_err_t max30102ReadSample(max30102Sample *out) {
    if (!out) return ESP_ERR_INVALID_ARG;
    return max30102ReadIfReady(out);
}

esp_err_t max30102Reset(void) {
    return max30102ResetFifo();
}

esp_err_t max30102ReadInterruptStatus(uint8_t *intStatus1, uint8_t *intStatus2) {
    return max30102ReadInterrupt(intStatus1, intStatus2);
}

esp_err_t max30102HandleInterrupt(max30102Sample *out)
{
    esp_err_t ret = max30102ReadIfReady(out);
    if (ret == ESP_ERR_NOT_FOUND) return ESP_OK;
    return ret;
}

static void stopTimerCb(TimerHandle_t xTimer) {
    (void)xTimer;
    max30102StopSession();
}

esp_err_t max30102StartSession(uint32_t durationMs) {
    if (!g_lock) g_lock = xSemaphoreCreateMutex();
    xSemaphoreTake(g_lock, portMAX_DELAY);

    if (g_running) {
        xSemaphoreGive(g_lock);
        return ESP_OK;
    }

    // 1) Boot the sensor
    ESP_ERROR_CHECK(max30102SensorInit());
    ESP_ERROR_CHECK(max30102FifoInit());
    ESP_ERROR_CHECK(max30102ResetFifo());
    ESP_ERROR_CHECK(max30102SetMode(MODE_SPO2));
    ESP_ERROR_CHECK(max30102SetLedCurrent(0x24, 0x24));

    // 2) Set the running status
    g_running = true;

    // 3) Set a stop timer（To run max30102StopSession）
    if (g_stopTimer) {
        xTimerStop(g_stopTimer, 0);
        xTimerDelete(g_stopTimer, 0);
    }
    g_stopTimer = xTimerCreate("m02Stop", pdMS_TO_TICKS(durationMs), pdFALSE, NULL, stopTimerCb);
    if (g_stopTimer) xTimerStart(g_stopTimer, 0);

    xSemaphoreGive(g_lock);
    ESP_LOGI(TAG, "Session started (%lu ms)", (unsigned long)durationMs);
    return ESP_OK;
}

esp_err_t max30102StopSession(void) {
    if (!g_lock) g_lock = xSemaphoreCreateMutex();
    xSemaphoreTake(g_lock, portMAX_DELAY);

    if (!g_running) {
        xSemaphoreGive(g_lock);
        return ESP_OK;
    }

    if (g_stopTimer) {
        xTimerStop(g_stopTimer, 0);
        xTimerDelete(g_stopTimer, 0);
        g_stopTimer = NULL;
    }
    g_running = false;

    xSemaphoreGive(g_lock);
    ESP_LOGI(TAG, "Session stopped");
    return ESP_OK;
}

bool max30102IsRunning(void) {
    return g_running;
}