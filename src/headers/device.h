#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <stdlib.h>

#define IP_ADDRESS_SIZE 15
#define PORT_SIZE       4
#define KEY_SIZE        16
#define DEVICE_ID_SIZE  20
#define UUID_SIZE       20
#define COMMAND_SIZE    1

typedef enum command {  SWITCH_ON      = 0,
						SWITCH_OFF     = 1,
						SET_BRIGHTNESS = 2,
						SET_WHITE      = 3,
						SET_COLOR      = 4
			} command_t;

typedef struct deviceStatus {
	uint8_t  brightness;
	uint8_t  colorTemp;
	uint8_t  r;
	uint8_t  g;
	uint8_t  b;
	uint16_t h;
	uint8_t  s;
	uint8_t  v;
} deviceStatus_t;

typedef struct device {
	uint8_t        ipAddress[IP_ADDRESS_SIZE + 1];
	uint16_t       port;
	uint8_t        key      [KEY_SIZE + 1];
	uint8_t        devId    [DEVICE_ID_SIZE + 1];
	uint8_t        uuid     [UUID_SIZE + 1];
	uint32_t       commandTime;
	deviceStatus_t status;
} device_t;

#endif // _DEVICE_H_