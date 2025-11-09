#pragma once
#include <stdbool.h>

bool transportHttpInit(void);
bool transportHttpSend(const uint8_t *buf, size_t len);
void transportHttpDeinit(void);