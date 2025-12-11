#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAL_REG_MAP_H
#define HAL_REG_MAP_H

#include <stdint.h>

/* ===========================
 * MAX30102 I2C DEVICE ADDRESS
 * =========================== */
#define MAX30102_ADDR 0x57

/* ===========================
 * REGISTER MAP (from datasheet)
 * =========================== */
#define REG_INT_STATUS_1      0x00
#define REG_INT_STATUS_2      0x01
#define REG_INT_ENABLE_1      0x02
#define REG_INT_ENABLE_2      0x03

#define REG_FIFO_WR_PTR       0x04
#define REG_OVF_COUNTER       0x05
#define REG_FIFO_RD_PTR       0x06
#define REG_FIFO_DATA         0x07

#define REG_FIFO_CONFIG       0x08
#define REG_MODE_CONFIG       0x09
#define REG_SPO2_CONFIG       0x0A

#define REG_LED1_PA           0x0C  // IR LED current
#define REG_LED2_PA           0x0D  // Red LED current

#define REG_MULTI_LED_CTRL1   0x11
#define REG_MULTI_LED_CTRL2   0x12

#define REG_TEMP_INTEGER      0x1F
#define REG_TEMP_FRACTION     0x20

#define REG_REV_ID            0xFE
#define REG_PART_ID           0xFF

// MAX30102 Interrupt Status Register Masks
#define MAX30102_INT_A_FULL_MASK        0x80  // FIFO almost full
#define MAX30102_INT_PPG_RDY_MASK       0x40  // New FIFO data ready
#define MAX30102_INT_ALC_OVF_MASK       0x20  // Ambient light overflow
#define MAX30102_INT_PROX_INT_MASK      0x10  // Proximity threshold triggered

// INT_STATUS_2
#define MAX30102_INT_DIE_TEMP_RDY_MASK  0x02  // Temperature conversion ready

/* ===========================
 * BITFIELD DEFINITIONS
 * =========================== */

/* Interrupt Status 1 Register (0x00) */
typedef union {
    uint8_t all;
    struct {
        uint8_t ppgRdy    : 1;  // Bit 0: New FIFO data ready
        uint8_t alcOvf    : 1;  // Bit 1: Ambient light cancellation overflow
        uint8_t pwrRdy    : 1;  // Bit 2: Power ready
        uint8_t proxInt   : 1;  // Bit 3: Proximity interrupt
        uint8_t dieTempRdy: 1;  // Bit 4: Temperature ready
        uint8_t reserved  : 3;
    } bits;
} max30102IntStatus1Reg;

/* Mode Configuration Register (0x09) */
typedef union {
    uint8_t all;
    struct {
        uint8_t mode     : 3;  // Bits [2:0]: mode selection
        uint8_t reserved : 3;
        uint8_t reset    : 1;  // Bit 6: reset FIFO/data
        uint8_t shutdown : 1;  // Bit 7: shutdown control
    } bits;
} max30102ModeConfigReg;

/* SpO2 Configuration Register (0x0A) */
typedef union {
    uint8_t all;
    struct {
        uint8_t ledPulseWidth : 2; // Bits [1:0]: LED pulse width
        uint8_t sampleRate    : 3; // Bits [4:2]: sample rate
        uint8_t adcRange      : 2; // Bits [6:5]: ADC range
        uint8_t reserved      : 1;
    } bits;
} max30102SpO2ConfigReg;

/* FIFO Configuration Register (0x08) */
typedef union {
    uint8_t all;
    struct {
        uint8_t fifoAFull : 4; // Bits [3:0]: FIFO almost full threshold
        uint8_t sampleAvg : 3; // Bits [6:4]: FIFO averaging samples
        uint8_t rollOver  : 1; // Bit 7: FIFO rollover enable
    } bits;
} max30102FifoConfigReg;

/* ===========================
 * ENUM DEFINITIONS
 * =========================== */

/* Mode selection */
typedef enum {
    HAL_MODE_HEART_RATE = 0x02,
    HAL_MODE_SPO2       = 0x03,
    HAL_MODE_MULTI_LED  = 0x07
} halMax30102Mode;

/* ADC range selection */
typedef enum {
    ADC_RANGE_2048  = 0,
    ADC_RANGE_4096  = 1,
    ADC_RANGE_8192  = 2,
    ADC_RANGE_16384 = 3
} max30102AdcRange;

/* Sample rate selection */
typedef enum {
    SAMPLE_RATE_50   = 0,
    SAMPLE_RATE_100  = 1,
    SAMPLE_RATE_200  = 2,
    SAMPLE_RATE_400  = 3,
    SAMPLE_RATE_800  = 4,
    SAMPLE_RATE_1000 = 5,
    SAMPLE_RATE_1600 = 6,
    SAMPLE_RATE_3200 = 7
} max30102SampleRate;

/* Pulse width selection */
typedef enum {
    PULSE_WIDTH_69US  = 0,
    PULSE_WIDTH_118US = 1,
    PULSE_WIDTH_215US = 2,
    PULSE_WIDTH_411US = 3
} max30102PulseWidth;

#endif

#ifdef __cplusplus
}
#endif