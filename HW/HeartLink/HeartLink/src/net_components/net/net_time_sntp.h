#ifdef __cplusplus
extern "C" { 
#endif

#pragma once
#include <stdbool.h>
#include <stdint.h>

bool sntpSyncOnce(const char *server, uint32_t timeoutMs);

#ifdef __cplusplus
}
#endif