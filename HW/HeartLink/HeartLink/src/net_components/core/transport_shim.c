#include "transport.h"
#include "net_components/net/transport_http.h"

bool transportInit(void) {
    return transportHttpInit();
}

bool transportSend(const uint8_t*b,size_t l) {
    return transportHttpSend(b,l);
}

void transportDeinit(void) {
    transportHttpDeinit();
}