#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "socket_communication.h"

#define RESP_MAX_SIZE 512

#define SA struct sockaddr

result_t SendPacket(uint8_t *payload, uint16_t size, device_t *device) {
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	uint8_t response[RESP_MAX_SIZE];

	printf("REACHED HERE\n");

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n");
	memset(&servaddr, 0, sizeof(servaddr));

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr((char*)(device->ipAddress));
	printf("%d\n", device->port);
	servaddr.sin_port = htons(device->port);

	char debug[32];

	printf("%s\n", inet_ntop(AF_INET, &(servaddr.sin_addr.s_addr), debug, 16));
	printf("%d\n", htons(servaddr.sin_port));

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("CONNECTION FAIL\n");
		return CONNECTION_ERROR;
	}

	printf("SENDING\n");
	printf("size: %d\n", size);


	uint16_t sentBytes = write(sockfd, payload, size);
	printf("sent: %d bytes\n", sentBytes);
	/*
	printf("RECEIVING\n");
	read(sockfd, response, RESP_MAX_SIZE - 1);
	printf("From Server : %s", response);
	*/

	// close the socket
	close(sockfd);

	return OK;
}