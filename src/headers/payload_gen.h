#ifndef _PAYLOAD_GEN_H_
#define _PAYLOAD_GEN_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "device.h"
#include "tools.h"

#define MAX_RESP_PAYLOAD_SIZE 1024
#define MIN_RESP_PAYLOAD_SIZE 8

uint16_t generatePayload(device_t *device, command_t command, uint32_t time, uint8_t *resultPayload);
result_t checkPayload(uint8_t *response, uint8_t responseSize, command_t command);

#endif //_PAYLOAD_GEN_H_