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

#define SA struct sockaddr

result_t SendPacket(uint8_t *payload, uint16_t size, device_t *device, uint8_t *response, uint16_t *responseSize) {
	int sockfd, connfd;
	uint16_t sentBytes = 0, receivedBytes = 0, rxRetries = 0;
	struct sockaddr_in servaddr, cli;

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) {

#ifdef DEBUG
		printf("socket creation failed...\n");
#endif

		*responseSize = 0xffff;
		response[0] = '\0';
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

		*responseSize = 0xffff;
		response[0] = '\0';
		return CONNECTION_ERROR;
	}

	sentBytes = write(sockfd, payload, size);

	if (sentBytes != size) {

#ifdef DEBUG
		printf("Mismatch between quantity sent bytes and payload size\n");
		printf("Sent: %d bytes\n", sentBytes);
		printf("Payload size: %d bytes\n", size);
#endif

		//close the socket
		close(sockfd);
		*responseSize = 0xffff;
		response[0] = '\0';
		return TX_ERROR;
	}

#ifdef DEBUG
	printf("PACKET sent, with size: %d bytes\n", size);
	printf("total number of data sent: %d bytes\n", sentBytes);
#endif

	while (receivedBytes < RX_MIN_SIZE && rxRetries < MAX_RX_RETRIES) {
		receivedBytes += read(	sockfd,
								&(response[receivedBytes]),
								RESP_MAX_SIZE - 1);
		usleep(100000);
		rxRetries++;
	}

	if (rxRetries == MAX_RX_RETRIES) {

#ifdef DEBUG
		printf("Maximum number of RX retries reached\n");
#endif

		//close the socket
		close(sockfd);
		*responseSize = 0xffff;
		response[0] = '\0';
		return MAX_RX_ERROR;
	}

#ifdef DEBUG
	printf("Response received, size: %d bytes\n", receivedBytes);
	printf("HEX response:\n");
	for (uint16_t i = 0; i < receivedBytes; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", response[i]);
	}
	printf("\n");
#endif

	*responseSize = receivedBytes;

	// close the socket
	close(sockfd);

	return OK;
}