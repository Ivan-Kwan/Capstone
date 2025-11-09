#include "hal_interrupt.h"
#include "hal_config.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "HAL_INTERRUPT";

/* Queue handle to send interrupt events to other tasks */
static QueueHandle_t interruptQueue = NULL;

/* Interrupt Service Routine (ISR) */
static void IRAM_ATTR gpioIsrHandler(void *arg)
{
    uint32_t gpioNum = (uint32_t) arg;

    /* Send GPIO number to queue from ISR context */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(interruptQueue, &gpioNum, &xHigherPriorityTaskWoken);

    /* Trigger context switch if needed */
    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

/* Initialize the interrupt GPIO and queue */
esp_err_t interruptInit(void)
{
    esp_err_t ret;

    /* Create queue to handle interrupt events */
    interruptQueue = xQueueCreate(10, sizeof(uint32_t));
    if (interruptQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create interrupt queue");
        return ESP_FAIL;
    }

    /* Configure GPIO as input with pull-up and rising edge interrupt */
    gpio_config_t ioConf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MAX30102_INT_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    ret = gpio_config(&ioConf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        return ret;
    }

    /* Install ISR service if not already installed */
    ret = gpio_install_isr_service(0); // 0 means ESP_INTR_FLAG_DEFAULT
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "ISR service installation failed");
        return ret;
    }

    /* Add ISR handler for MAX30102 interrupt pin */
    ret = gpio_isr_handler_add(MAX30102_INT_PIN, gpioIsrHandler, (void *) MAX30102_INT_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ISR handler add failed");
        return ret;
    }

    ESP_LOGI(TAG, "Interrupt initialized on GPIO %d", MAX30102_INT_PIN);
    return ESP_OK;
}

/* Wait for interrupt event from queue */
bool interruptWaitEvent(uint32_t timeoutMs)
{
    uint32_t ioNum;
    if (xQueueReceive(interruptQueue, &ioNum, pdMS_TO_TICKS(timeoutMs))) {
        ESP_LOGD(TAG, "Interrupt event received from GPIO %ld", ioNum);
        return true;
    }
    return false;
}

/* Deinitialize interrupt (remove handler and delete queue) */
void interruptDeinit(void)
{
    gpio_isr_handler_remove(MAX30102_INT_PIN);
    if (interruptQueue) {
        vQueueDelete(interruptQueue);
        interruptQueue = NULL;
    }
    ESP_LOGI(TAG, "Interrupt deinitialized");
}