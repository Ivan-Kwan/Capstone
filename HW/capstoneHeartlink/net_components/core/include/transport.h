#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" { 
#endif

bool transportInit(void);                          // 联网+对时+HTTP初始化
bool transportSend(const uint8_t *buf, size_t len);// 发送一帧/批JSON
void transportDeinit(void);

#ifdef __cplusplus
}
#endif