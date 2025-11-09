#ifndef MAX30102_TYPES_H
#define MAX30102_TYPES_H

#include <stdint.h>

// Sensor modes
typedef enum {
    MODE_HEART_RATE = 0,
    MODE_SPO2,
    MODE_MULTI_LED
} sensorMode;

typedef struct {
    uint32_t red;
    uint32_t ir;
} max30102Sample;

#endif