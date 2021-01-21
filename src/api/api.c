#include "tools.h"
#include "api.h"
#include "socket_communication.h"

uint16_t sendCommand(device_t *device, uint32_t time, command_t command) {
	uint8_t payload[256];
	uint16_t payloadSize;
	result_t com_res;

	payloadSize = GeneratePayload(device, command, time, payload);

	com_res = SendPacket(payload, payloadSize, device);

	return OK;
}