#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum {
    SYS_STATE_IDLE,
    SYS_STATE_RUNNING,
    SYS_STATE_BUTT
} systemState;

#endif
#ifdef __cplusplus
}
#endif