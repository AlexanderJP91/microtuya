#ifndef _SOCKET_COMMUNICATION_H_
#define _SOCKET_COMMUNICATION_H_

#include <stdlib.h>

#include "tools.h"
#include "device.h"

#define RX_MIN_SIZE    28
#define MAX_RX_RETRIES 5

result_t SendPacket(uint8_t *payload, uint16_t size, device_t *device, uint8_t *response, uint16_t *responseSize);

#endif //_SOCKET_COMMUNICATION_H_