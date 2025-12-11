#ifdef __cplusplus
extern "C" { 
#endif

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool transportInit(void);                           // Init Networking + Time Sync + HTTP
bool transportSend(const uint8_t *buf, size_t len); // Send a frame/batch of JSON
void transportDeinit(void);

#ifdef __cplusplus
}
#endif