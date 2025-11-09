#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "driver/gpio.h"
#include "driver/i2c.h"

#define MAX30102_I2C_PORT  I2C_NUM_0    // I2C master or internal communication port
#define MAX30102_INT_PIN   GPIO_NUM_25   // Interrupt pin（GPIO25）
#define MAX30102_SDA_PIN   GPIO_NUM_21   // SDA
#define MAX30102_SCL_PIN   GPIO_NUM_26   // SCL
#define MAX30102_ADDR      0x57          // MAX30102 I2C address
#define I2C_MASTER_FREQ_HZ 100000

#endif