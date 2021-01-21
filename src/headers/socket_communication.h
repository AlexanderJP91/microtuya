#ifndef _SOCKET_COMMUNICATION_H_
#define _SOCKET_COMMUNICATION_H_

#include <stdlib.h>

#include "tools.h"
#include "device.h"

result_t SendPacket(uint8_t *payload, uint16_t size, device_t *device);

#endif //_SOCKET_COMMUNICATION_H_