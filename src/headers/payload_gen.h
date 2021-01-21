#ifndef _PAYLOAD_GEN_H_
#define _PAYLOAD_GEN_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "device.h"

uint16_t GeneratePayload(device_t *device, command_t command, uint32_t time, uint8_t *resultPayload);

#endif //_PAYLOAD_GEN_H_