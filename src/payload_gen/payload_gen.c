#include "payload_gen.h"
#include "aes.h"
#include "crc32.h"

#define IS_BIG_ENDIAN ('\x01\x02\x03\x04' == 0x01020304)

#define TIMESTAMP_SIZE 10

static void convertTimeToString(uint32_t inTime, uint8_t *result);
static void convertUint8ToString(uint8_t inValue, uint8_t *result);
static void convertUint8ToHexString(uint8_t inValue, uint8_t *result);

uint8_t commands[][128] = {	"\"dps\":{\"1\":true}",
							"\"dps\":{\"1\":false}",
							"\"dps\":{\"3\":255}",
							"\"dps\":{\"4\":255}",
							"\"dps\":{\"2\":\"white\",\"3\":255,\"4\":255}",
							"\"dps\":{\"2\":\"colour\",\"5\":\"00000000000000\"}"
						};

uint8_t key[]         = "0000000000000000";
uint8_t devId[]       = "\"devId\":\"00000000000000000000\"";
uint8_t uuid[]        = "\"uid\":\"00000000000000000000\"";
uint8_t t[]           = "\"t\":\"0000000000\"";
uint8_t vMeta[]       = { '3',  '.',  '3',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t suffix[]      = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x55 };
uint8_t prefix[]      = { 0x00, 0x00, 0x55, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t commandCode[] = { 0x07, 0x00, 0x00, 0x00 };

uint8_t prefixSize  = sizeof(prefix) / sizeof(prefix[0]);
uint8_t commandCodeSize = sizeof(commandCode) / sizeof(commandCode[0]);

uint16_t generatePayload(device_t *device, command_t command, uint32_t time, uint8_t *resultPayload) {

	uint8_t devIdSize   = strlen((char*)devId);
	uint8_t uuidSize    = strlen((char*)uuid);
	uint8_t commandSize = 0;
	uint8_t tSize       = strlen((char*)t);
	uint8_t vMetaSize   = sizeof(vMeta) / sizeof(vMeta[0]);

	uint8_t suffixSize  = sizeof(suffix) / sizeof(suffix[0]);

	////////////////////////
	//VARIABLES DEFINITION//
	////////////////////////

	//command time
	uint8_t nowString[11], nowStringSize = 10;

	//since the command size is unknown, need a buffer to store it temporarly
	uint8_t *tmpCommand, *tmpCommandPtr, tmpString[4];

	//payload with no padding
	uint8_t *payload, *payloadPtr, payloadSize;

	//payload with padding
	uint8_t *paddedPayload, padding, paddedPayloadSize;

	//encrypted payload
	uint8_t *encPayload, encPayloadSize;

	//encrypted payload
	uint8_t *payloadWithMeta, *payloadWithMetaPtr, payloadWithMetaSize;

	//encryption variables
	struct AES_ctx ctx;

	//CRC
	uint32_t crc;
	uint8_t  crcArray[4];

	//general variables
	uint8_t i;

	//////////////////////////////////
	//VARIABLES DEFINITION COMPLETED//
	//////////////////////////////////

	/////////////////////////////
	//UPDATE PAYLOAD PROPERTIES//
	/////////////////////////////

	tmpCommand = (uint8_t *) malloc(strlen((char*)commands[command]));
	memcpy(tmpCommand, commands[command], strlen((char*)commands[command]));

	printf("DEBUG 1: %s\n", tmpCommand);

	if(command == SET_BRIGHTNESS || command == SET_COLOR_TEMP) {
		if(command == SET_BRIGHTNESS) {
			convertUint8ToString(device->status.brightness, tmpString);
		} else {
			convertUint8ToString(device->status.colorTemp, tmpString);
		}
		memcpy(&(tmpCommand[strlen((char*)tmpCommand) - 4]), tmpString, strlen((char*)tmpString));
		tmpCommand[strlen((char*)tmpCommand) - 4 + strlen((char*)tmpString)] = '}';
		tmpCommand[strlen((char*)tmpCommand) - 4 + strlen((char*)tmpString) + 1] = '\0';
	} else if(command == SET_WHITE) {
		convertUint8ToString(device->status.brightness, tmpString);
		tmpCommandPtr = &(tmpCommand[strlen((char*)tmpCommand) - 12]);
		memcpy(	tmpCommandPtr,
				tmpString,
				strlen((char*)tmpString)
			);
		tmpCommandPtr += strlen((char*)tmpString);
		memcpy(
				tmpCommandPtr,
				",\"4\":",
				5
			);
		tmpCommandPtr += 5;
		convertUint8ToString(device->status.colorTemp, tmpString);
		memcpy(
				tmpCommandPtr,
				tmpString,
				strlen((char*)tmpString)
			);
		tmpCommandPtr += strlen((char*)tmpString);
		memcpy(
				tmpCommandPtr,
				"}\0",
				2
			);
	} else if(command == SET_COLOR) {
		tmpCommandPtr = &(tmpCommand[strlen((char*)tmpCommand) - 16]);
		convertUint8ToHexString(device->status.r, tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString(device->status.g, tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString(device->status.b, tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString((uint8_t)(device->status.h >> 8), tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString((uint8_t)(device->status.h), tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString(device->status.s, tmpCommandPtr);
		tmpCommandPtr += 2;
		convertUint8ToHexString(device->status.v, tmpCommandPtr);
		tmpCommandPtr += 2;
		memcpy(
				tmpCommandPtr,
				"\"}\0",
				3
			);
		tmpCommandPtr += 3;
		*tmpCommandPtr = '\0';
	}


	memcpy(key,        device->key, KEY_SIZE);
	payloadPtr = &devId[devIdSize - DEVICE_ID_SIZE - 1];
	memcpy(payloadPtr, device->devId, DEVICE_ID_SIZE);
	payloadPtr = &uuid[uuidSize - UUID_SIZE - 1];
	memcpy(payloadPtr, device->uuid, UUID_SIZE);
	convertTimeToString(time, nowString);
	payloadPtr = &t[tSize - TIMESTAMP_SIZE - 1];
	memcpy(payloadPtr, nowString, TIMESTAMP_SIZE);
	commandSize = strlen((char*)tmpCommand);

	////////////////////////////////////
	//GEBERATE PAYLOAD WITH NO PADDING//
	////////////////////////////////////

	memcpy(&(t[5]), nowString, nowStringSize);

	payloadSize = devIdSize + uuidSize + tSize + commandSize + 5;

	payload = (uint8_t *) malloc(payloadSize + 1);

	payloadPtr = payload;
	*payloadPtr = '{';
	payloadPtr++;
	memcpy(payloadPtr, devId,             devIdSize   );
	payloadPtr += devIdSize;
	*payloadPtr = ',';
	payloadPtr++;
	memcpy(payloadPtr, uuid,              uuidSize    );
	payloadPtr += uuidSize;
	*payloadPtr = ',';
	payloadPtr++;
	memcpy(payloadPtr, t,                 tSize       );
	payloadPtr += tSize;
	*payloadPtr = ',';
	payloadPtr++;
	memcpy(payloadPtr, tmpCommand, commandSize);
	payloadPtr += commandSize;
	*payloadPtr = '}';

	payload[payloadSize] = '\0';

	/////////////////////////////////////
	//PAYLOAD GENERATED WITH NO PADDING//
	/////////////////////////////////////

#ifdef DEBUG
	printf("payload with no padding size: %d\n", payloadSize);
	printf("ASCII payload with no padding:\n");
	printf("%s\n", payload);
	printf("HEX payload with no padding:\n");
	for (i = 0; i < payloadSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", payload[i]);
	}
	printf("\n");
	printf("payloadSize: %d\n", payloadSize);
	printf("\n\n");
#endif

	//////////////////////////
	//ADD PADDING TO PAYLOAD//
	//////////////////////////

	//compute the wanted padding
	padding = 16 - (payloadSize % 16);
	//payload size with padding
	paddedPayloadSize = payloadSize + padding;
	//allocate the memory for the padded payload
	paddedPayload = (uint8_t *) malloc(paddedPayloadSize + 1);
	//store new padded payload
	memcpy(paddedPayload,                 payload, payloadSize);
	memset(&(paddedPayload[payloadSize]), padding, padding    );
	paddedPayload[paddedPayloadSize] = '\0';

	//free the not padded payload memory location
	free(payload);

	//////////////////////////////////
	//PAYLOAD WITH PADDING GENERATED//
	//////////////////////////////////

#ifdef DEBUG
	printf("ASCII payload with padding:\n");
	printf("%s\n", paddedPayload);
	printf("HEX payload with padding:\n");
	for (i = 0; i < paddedPayloadSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", paddedPayload[i]);
	}
	printf("\n\n");
#endif

	///////////
	//ENCRYPT//
	///////////

	//encrypted payload has the same size of the padded payload
	encPayloadSize = paddedPayloadSize;
	//allocate the memory for the padded payload
	encPayload = (uint8_t *) malloc(encPayloadSize + 1);
	//make a caopy of the padded payload
	memcpy(encPayload, paddedPayload, encPayloadSize);
	encPayload[encPayloadSize] = '\0';

	//free the padded payload memory location
	free(paddedPayload);

	//init AES
	AES_init_ctx(&ctx, device->key);
	//loop encryption on 16 bytes at the time
	for (i = 0; i < encPayloadSize / 16; i++) {
		AES_ECB_encrypt(&ctx, &(encPayload[i * 16]));
	}

	////////////////////////
	//ENCRYPTION COMPLETED//
	////////////////////////

#ifdef DEBUG
	printf("HEX encrypted payload:\n");
	for (i = 0; i < encPayloadSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", encPayload[i]);
	}
	printf("\n\n");
#endif

	//////////////////////////////////////////
	//GENERATE PAYLOAD WITH META INFORMATION//
	//////////////////////////////////////////

	payloadWithMetaSize = encPayloadSize + vMetaSize + suffixSize + prefixSize + commandCodeSize + 1; //extra byte for size info

	//allocate the memory for the payload with meta information
	payloadWithMeta = (uint8_t *) malloc(payloadWithMetaSize + 1);

	//generate the payload with meta information
	payloadWithMetaPtr = payloadWithMeta;
	memcpy(payloadWithMetaPtr, prefix,  prefixSize);
	payloadWithMetaPtr += prefixSize;
	memcpy(payloadWithMetaPtr, commandCode, commandCodeSize);
	payloadWithMetaPtr += commandCodeSize;
	*payloadWithMetaPtr = encPayloadSize +vMetaSize + suffixSize;
	payloadWithMetaPtr++;
	printf("%s\n", payloadWithMeta);
	memcpy(payloadWithMetaPtr, vMeta, vMetaSize);
	payloadWithMetaPtr += vMetaSize;
	memcpy(payloadWithMetaPtr, encPayload, encPayloadSize);
	payloadWithMetaPtr += encPayloadSize;
	memcpy(payloadWithMetaPtr, suffix, suffixSize);
	payloadWithMetaPtr += suffixSize;

	payloadWithMeta[payloadWithMetaSize] = '\0';

	printf("\n\n");

	//free the encrypted payload memory location
	free(encPayload);

	///////////////////////////////////////////
	//PAYLOAD WITH META INFORMATION GENERATED//
	///////////////////////////////////////////

#ifdef DEBUG
	printf("payload with meta information:\n");
	for (i = 0; i < payloadWithMetaSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", payloadWithMeta[i]);
	}
	printf("\n\n");
#endif

	crc = crc32(payloadWithMeta, payloadWithMetaSize - 8);
	crcArray[3] = (uint8_t)(crc & 0x000000ff);
	crcArray[2] = (uint8_t)((crc & 0x0000ff00) >> 8);
	crcArray[1] = (uint8_t)((crc & 0x00ff0000) >> 16);
	crcArray[0] = (uint8_t)((crc & 0xff000000) >> 24);

#ifdef DEBUG
	printf("CRC integer: 0x%08x\n", crc);
	printf("\n\n");
#endif

	memcpy(&(payloadWithMeta[payloadWithMetaSize - 8]), crcArray, 4);

#ifdef DEBUG
	printf("payload with meta information and CRC:\n");
	for (i = 0; i < payloadWithMetaSize; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", payloadWithMeta[i]);
	}
	printf("\n\n");
#endif

	//allocate the memory for the payload to return
	//resultPayload = (uint8_t *) malloc(payloadWithMetaSize + 1);
	//STORE THE RESULT
	memcpy(resultPayload, payloadWithMeta, payloadWithMetaSize + 1);
	
	free(payloadWithMeta);

	return payloadWithMetaSize;
}

result_t checkPayload(uint8_t *response, uint8_t responseSize, command_t command) {
	uint8_t *responsePtr, *tmpString, crcCheck = 0, i, endianessShift, actualPayloadSize, expectedPayloadSize;
	uint32_t responseCrc;

	////////////////
	//PREFIX CHECK//
	////////////////

	responsePtr = response;
	if (memcmp(responsePtr, prefix, prefixSize) != 0) {

#ifdef DEBUG
	printf("Error checking the response prefix\n");
#endif

		return RESPONSE_DATA_ERROR;
	}

	//////////////////////////
	//PREFIX CHECK COMPLETED//
	//////////////////////////

	//////////////////////
	//COMMAND CODE CHECK//
	//////////////////////

	responsePtr += prefixSize;
	if (memcmp(responsePtr, commandCode, commandCodeSize) != 0) {

#ifdef DEBUG
	printf("Error checking the response command code\n");
#endif

		return RESPONSE_DATA_ERROR;
	}

	////////////////////////////////
	//COMMAND CODE CHECK COMPLETED//
	////////////////////////////////

	/////////////////
	//PAYLOAD CHECK//
	/////////////////

	responsePtr += commandCodeSize;
	actualPayloadSize = response - responsePtr + responseSize - 1;
	expectedPayloadSize = *responsePtr;
	if (actualPayloadSize != expectedPayloadSize || actualPayloadSize < MIN_RESP_PAYLOAD_SIZE) {

#ifdef DEBUG
	printf("Error checking the response size\n");
#endif

		return RESPONSE_SIZE_ERROR;
	}
	tmpString = (uint8_t*)malloc(expectedPayloadSize - 8);
	memset(tmpString, '\0', expectedPayloadSize - 8);
	responsePtr++;
	if (memcmp(responsePtr, tmpString, expectedPayloadSize - 8) != 0) {

#ifdef DEBUG
		printf("Error checking the response payload\n");
#endif

		return RESPONSE_DATA_ERROR;
	}

	///////////////////////////
	//PAYLOAD CHECK COMPLETED//
	///////////////////////////

	///////////////
	//CRC32 CHECK//
	///////////////

	responsePtr += expectedPayloadSize - 8;
	responseCrc = crc32(response, responseSize - 8);
	for (i = 0; i < sizeof(responseCrc); i++) {
#if IS_BIG_ENDIAN == 1
		endianessShift = 8 * ((uint8_t)sizeof(responseCrc) - i - 1);
#else
		endianessShift = 8 * i;
#endif
		crcCheck += *(responsePtr + i) != (uint8_t)(responseCrc >> endianessShift);
	}

#ifdef DEBUG
	printf("received CRC: 0x%02x%02x%02x%02x\n", 	*(responsePtr), 
													*(responsePtr + 1),
													*(responsePtr + 2),
													*(responsePtr + 3));
	printf("computed CRC: 0x%08x\n", responseCrc);
	printf(crcCheck == 0 ? "CRC MATCH!\n" : "CRC MISMATCH!\n");
#endif

	if (crcCheck != 0) {
		return CRC_ERROR;
	}

	/////////////////////////
	//CRC32 CHECK COMPLETED//
	/////////////////////////

	////////////////
	//SUFFIX CHECK//
	////////////////

	responsePtr += 4;

	if (memcmp(responsePtr, &(suffix[4]), 4) != 0) {
		return RESPONSE_DATA_ERROR;
	}

	//////////////////////////
	//SUFFIX CHECK COMPLETED//
	//////////////////////////

	return OK;
}

static void convertTimeToString(uint32_t inTime, uint8_t *result) {
	int8_t i;
	uint32_t tmp;

	tmp = inTime;

	for(i = 9; i >= 0; i--) {
		result[i] = (tmp % 10) + '0';
		tmp /= 10;
	}
}

static void convertUint8ToString(uint8_t inValue, uint8_t *result) {

	if(inValue < 10) {
		result[0] = inValue + '0';
		result[1] = '\0';
	} else if(inValue < 100) {
		result[0] = (inValue / 10) + '0';
		result[1] = (inValue % 10) + '0';
		result[2] = '\0';
	} else {
		result[0] = (inValue / 100) + '0';
		result[1] = ((inValue % 100) / 10) + '0';
		result[2] = (inValue % 10) + '0';
		result[3] = '\0';
	}
}

static void convertUint8ToHexString(uint8_t inValue, uint8_t *result) {

	uint8_t tmp;

	tmp = (inValue & 0xf0) >> 4;
	result[0] = tmp > 0x09 ? tmp + 'a' - 10 : tmp + '0';
	tmp = inValue & 0x0f;
	result[1] = tmp > 0x09 ? tmp + 'a' - 10 : tmp + '0';
}