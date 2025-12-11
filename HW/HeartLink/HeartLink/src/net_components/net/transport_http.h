#ifdef __cplusplus
extern "C" { 
#endif

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool transportHttpInit(void);
bool transportHttpSend(const uint8_t *buf, size_t len);
bool transportHttpCheckCommand(char *cmd_buffer, int max_len);
void transportHttpDeinit(void);

#ifdef __cplusplus
}
#endif