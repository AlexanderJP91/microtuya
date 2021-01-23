#include "tools.h"
#include "api.h"
#include "socket_communication.h"

uint16_t sendCommand(device_t *device, uint32_t time, command_t command) {
	uint8_t payload[256], response[MAX_RESP_PAYLOAD_SIZE];
	uint16_t payloadSize, responseSize = 0;
	result_t com_res, payloadCheck;

	payloadSize = generatePayload(device, command, time, payload);

	com_res = SendPacket(payload, payloadSize, device, response, &responseSize);

	if (com_res != OK) {
#ifdef DEBUG
		printf("ERROR: failed to send the packet\n");
#endif
	}

	payloadCheck = checkPayload(response, responseSize, command);

#ifdef DEBUG
	printf(payloadCheck == OK 	? "SUCCESS: response check successful\n"
								: "ERROR: response check failed\n");
#endif

	return OK;
}