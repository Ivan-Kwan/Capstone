#include "max30102_hal/hal_max30102_sensor.h"
#include "hal_reg.h"
#include "esp_log.h"

static const char *TAG = "HAL_MAX30102_SENSOR";

/**
 * @brief Initialize MAX30102 sensor
 */
esp_err_t max30102SensorInit(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing MAX30102...");

    // Reset FIFO
    ret = max30102ResetFifo();
    if (ret != ESP_OK) return ret;

        // 0xC0 = A_FULL + PPG_RDY
    ret = regWrite8(REG_INT_ENABLE_1, 0xC0);
    if (ret != ESP_OK) return ret;

    ret = regWrite8(REG_INT_ENABLE_2, 0x00);
    if (ret != ESP_OK) return ret;

    // Configure FIFO (sample averaging = 4, rollover enabled, FIFO almost full = 17)
    ret = regWrite8(REG_FIFO_CONFIG, 0x4F);
    if (ret != ESP_OK) return ret;

    // Set SpO2 configuration: 100Hz sample rate, 411uA LED current, 18-bit ADC
    ret = regWrite8(REG_SPO2_CONFIG, 0x27);
    if (ret != ESP_OK) return ret;

    // Default LED current: 7mA for both
    ret = max30102SetLedCurrent(0x24, 0x24);
    if (ret != ESP_OK) return ret;

    // Set default mode to SpO2
    ret = max30102SetMode(HAL_MODE_SPO2);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "MAX30102 initialized successfully");
    return ESP_OK;
}

/**
 * @brief Set measurement mode
 */
esp_err_t max30102SetMode(sensorMode mode)
{
    halMax30102Mode halSensorMode = HAL_MODE_HEART_RATE;
    switch (mode) {
    case MODE_HEART_RATE:
        halSensorMode = HAL_MODE_HEART_RATE;
        break;
    case MODE_SPO2:
        halSensorMode = HAL_MODE_SPO2;
        break;
    case MODE_MULTI_LED:
        halSensorMode = HAL_MODE_MULTI_LED;
        break;
    default:
        break;
    }
    esp_err_t ret = regWrite8(REG_MODE_CONFIG, halSensorMode);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "Mode set to 0x%02X", halSensorMode);
    return ret;
}

/**
 * @brief Set LED current (0x00–0xFF, 0=off, 0xFF=max)
 */
esp_err_t max30102SetLedCurrent(uint8_t redCurrent, uint8_t irCurrent)
{
    esp_err_t ret;
    ret  = regWrite8(REG_LED1_PA, redCurrent);
    ret |= regWrite8(REG_LED2_PA, irCurrent);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "LED current set - RED: 0x%02X, IR: 0x%02X", redCurrent, irCurrent);
    return ret;
}

/**
 * @brief Configure FIFO registers
 */
esp_err_t max30102FifoInit(void)
{
    esp_err_t ret;
    ret = regWrite8(REG_FIFO_CONFIG, 0x4F); // sample avg=4, rollover=1, almost full=17
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "FIFO initialized");
    return ret;
}

/**
 * @brief Reset FIFO pointers
 */
esp_err_t max30102ResetFifo(void)
{
    esp_err_t ret;
    ret  = regWrite8(REG_FIFO_WR_PTR, 0x00);
    ret |= regWrite8(REG_OVF_COUNTER, 0x00);
    ret |= regWrite8(REG_FIFO_RD_PTR, 0x00);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "FIFO reset complete");
    return ret;
}

/**
 * @brief Read one sample (RED + IR) from FIFO
 */
esp_err_t max30102ReadIfReady(max30102Sample *out)
{
    if (out == NULL) return ESP_ERR_INVALID_ARG;

    uint8_t int1 = 0, int2 = 0;
    esp_err_t ret = regRead8(REG_INT_STATUS_1, &int1);
    if (ret != ESP_OK) return ret;

    // Read INT_STATUS_2 to clear potential flags like temperature ready
    (void)regRead8(REG_INT_STATUS_2, &int2);

    // Check mask: is there new data?
    if ((int1 & MAX30102_INT_PPG_RDY_MASK) == 0) {
        return ESP_ERR_NOT_FOUND; // No new sample
    }

    // FIFO burst read 6 bytes: RED[18:0] + IR[18:0]
    uint8_t buf[6];
    ret = regBurstRead(REG_FIFO_DATA, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    out->red =  ((uint32_t)(buf[0] & 0x03) << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    out->ir  =  ((uint32_t)(buf[3] & 0x03) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    return ESP_OK;
}

/**
 * @brief Read interrupt status registers (clear on read)
 */
esp_err_t max30102ReadInterrupt(uint8_t *intStatus1, uint8_t *intStatus2)
{
    esp_err_t ret;
    ret  = regRead8(REG_INT_STATUS_1, intStatus1);
    ret |= regRead8(REG_INT_STATUS_2, intStatus2);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "Interrupt Status - INT1: 0x%02X, INT2: 0x%02X", *intStatus1, *intStatus2);
    return ret;
}