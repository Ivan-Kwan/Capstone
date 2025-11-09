#pragma once
#include <stdbool.h>

bool sntpSyncOnce(const char *server, uint32_t timeoutMs);