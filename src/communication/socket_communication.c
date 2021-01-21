#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "socket_communication.h"
#include "crc32.h"
#include "tools.h"

#define RESP_MAX_SIZE 1024

#define IS_BIG_ENDIAN ('\x01\x02\x03\x04' == 0x01020304)

#define SA struct sockaddr

result_t SendPacket(uint8_t *payload, uint16_t size, device_t *device) {
	int sockfd, connfd;
	uint16_t responseSize = 0;
	struct sockaddr_in servaddr, cli;

	uint8_t response[RESP_MAX_SIZE];

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) {

#ifdef DEBUG
		printf("socket creation failed...\n");
#endif

		return CONNECTION_ERROR;
	} 
	else

#ifdef DEBUG
		printf("Socket successfully created..\n");
#endif

	memset(&servaddr, 0, sizeof(servaddr));

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr((char*)(device->ipAddress));
	servaddr.sin_port = htons(device->port);

	char debug[32];

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {

#ifdef DEBUG
		printf("CONNECTION FAIL\n");
#endif

		return CONNECTION_ERROR;
	}

	uint16_t sentBytes = write(sockfd, payload, size);

#ifdef DEBUG
	printf("PACKET sent, with size: %d bytes\n", size);
	printf("total number of data sent: %d bytes\n", sentBytes);
#endif

//TODO: manage this with a loop to append received data
	if (responseSize <= 28) {
		responseSize = read(sockfd, response, RESP_MAX_SIZE - 1);
		usleep(100000);
	}

#ifdef DEBUG
	printf("Response received, size: %d bytes\n", responseSize);
	printf("HEX response:\n");
	for (uint16_t i = 0; i < responseSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", response[i]);
	}
	printf("\n");
#endif

	uint32_t crc = crc32(response, responseSize - 8);
	uint8_t crcCheck = 0;

	for (uint8_t i = 0; i < sizeof(crc); i++) {
#if IS_BIG_ENDIAN == 1
		uint8_t endianessShift = 8 * ((uint8_t)sizeof(crc) - i - 1);
#else
		uint8_t endianessShift = 8 * i;
#endif
		crcCheck += response[responseSize - 8 + i] != (uint8_t)(crc >> endianessShift);
	}

#ifdef DEBUG
	printf("received CRC: 0x%02x%02x%02x%02x\n", 	response[responseSize - 8], 
													response[responseSize - 7],
													response[responseSize - 6],
													response[responseSize - 5]);
	printf("computed CRC: 0x%08x\n", crc);
	if (crcCheck == 0) {
		printf("CRC MATCH!\n");
	} else {
		printf("CRC MISMATCH!\n");
	}
#endif

	// close the socket
	close(sockfd);

	return OK;
}