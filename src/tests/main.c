#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "device.h"
#include "tools.h"
#include "api.h"

//#define EMBEDDED

#ifdef EMBEDDED
#include "embedded_communication.h"
#else
#include "socket_communication.h"
#endif

static uint8_t convertStringToUint8(char* inString);

//input order is:
	//IP address
	//PORT
	//key
	//device ID
	//UUID
	//command
	//other data sepending on command

int main(int argc, char *argv[]) {
	device_t device;
	uint8_t apiRes, i, command, inputError = 0;
	uint8_t brightness = 255, colorTemp = 255, r = 255, g = 255, b = 255;
	uint32_t now;
	rgb_t rgbColor;
	hsv_t hsvColor;

	if (argc < 7) {
		printf("INPUT FAIL\n");
		return INPUT_FAIL;
	}

	command = argv[6][0] - '0';

	switch(command) {
		case 0:
			inputError = argc == 7 ? 0 : 1;
			break;
		case 1:
			inputError = argc == 7 ? 0 : 1;
			break;
		case 2:
			inputError = argc == 8 ? 0 : 1;
			break;
		case 3:
			inputError = argc == 9 ? 0 : 1;
			break;
		case 4:
			inputError = argc == 10 ? 0 : 1;
			break;
		default:
			inputError = 1;
			break;
	}

	if (inputError != 0) {
		printf("INPUT FAIL\n");
		return INPUT_FAIL;
	}

	memcpy(device.ipAddress, argv[1], IP_ADDRESS_SIZE);
	device.port = 0;

	for(i = 0; i < strlen(argv[2]); i++) {
		device.port *= 10;
		device.port += (argv[2][i] - '0');
	}
	memcpy(device.key,       argv[3], KEY_SIZE);
	device.key[KEY_SIZE] = '\0';
	memcpy(device.devId,     argv[4], DEVICE_ID_SIZE);
	device.devId[DEVICE_ID_SIZE] = '\0';
	memcpy(device.uuid,      argv[5], UUID_SIZE);
	device.uuid[UUID_SIZE] = '\0';
	//now = (uint32_t)time(0);
	now = 1611100142;

	//set the device status to 0
	memset(&(device.status), 0, sizeof(deviceStatus_t));

	switch(command) {
		case 0:
			device.status.brightness = 0;
			device.status.colorTemp  = 0;
			break;
		case 1:
			device.status.brightness = 0;
			device.status.colorTemp  = 0;
			break;
		case 2:
			device.status.brightness = convertStringToUint8(argv[7]);
			device.status.colorTemp  = 0;
			break;
		case 3:
			device.status.brightness = convertStringToUint8(argv[7]);
			device.status.colorTemp  = convertStringToUint8(argv[8]);
			break;
		case 4:
			device.status.r          = convertStringToUint8(argv[7]);
			device.status.g          = convertStringToUint8(argv[8]);
			device.status.b          = convertStringToUint8(argv[9]);
			rgbColor.r               = (double)(device.status.r) / 255;
			rgbColor.g               = (double)(device.status.g) / 255;
			rgbColor.b               = (double)(device.status.b) / 255;
			hsvColor                 = rgb2hsv(rgbColor);
			device.status.h          = (uint16_t)(hsvColor.h * 360);
			device.status.s          = (uint8_t)(hsvColor.s * 360);
			device.status.v          = (uint8_t)(hsvColor.v * 360);
			break;
		default:
			printf("UNKNOWED COMMAND!!!\n");
			break;
	}

	sendCommand(&device, now, command);

	return OK;
}

static uint8_t convertStringToUint8(char* inString) {
	uint8_t i, tmp = 0;

	for(i = 0; i < strlen(inString); i++) {
		tmp *= 10;
		tmp += inString[i] - '0';
	}

	return tmp;
}