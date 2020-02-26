/*
 * nbmodule.c
 *
 *  Created on: 22 Feb 2020
 *      Author: yangjun
 */

#include "nbmodule.h"
#include <string.h>
#include "main.h"
#include <stdio.h>
#include "cust_hal_uart.h"

extern UartTermStr moduleThread;

enum ModuleState modulestate = 0;
/**
 * return 0 , success
 */
int Module_Put(char* param) {
	osEvent event;
	uint16_t length = strlen((char*) param);

	printf("\r\n2:Module_Put %d\r\n", length);
	sprintf(moduleThread.tmpBuffer, "%s", param);
	moduleThread.bInRx = 0;

	if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *) moduleThread.tmpBuffer,
			length) != HAL_OK) {
		printf("2:Error:transmit_DMA\r\n");
		/* Transfer error in transmission process */
		Error_Handler();
		return -1;
	}
	event = osMessageGet(moduleThread.txQ, osWaitForever);
	if (event.status == osEventMessage) {
//		sprintf(moduleThread.tmpBuffer, "2:tx event %lu", event.value.v);
//		safePrintf(moduleThread.tmpBuffer);
		return 0;
	}
	return -2;
}
void printBufFormat(uint8_t *buf, uint16_t len) {
	uint16_t i, j, upper;
	for (i = 0; i < len; i += 16) {
		if ((i + 16) > len) {
			upper = len;
		} else {
			upper = i + 16;
		}
		printf("0x%02x: ", i);
		for (j = i; j < upper; j++) {
			printf("%02x ", buf[j]);
		}
		printf("\r\n");
	}

	printf("readable:\r\n");
	for (i = 0; i < len; i += 16) {
		if ((i + 16) > len) {
			upper = len;
		} else {
			upper = i + 16;
		}
		for (j = i; j < upper; j++) {
			printf("%c", buf[j]);
		}
		printf("\r\n");
	}

}
ModulePacketType parsePacket(uint8_t * buf, uint16_t len) {
	uint16_t i;
	printf("Parse:\r\n");
//	for(i=0; i<len; i++){
//		printf("%02x ", buf[i]);
//	}
//	printf("\r\n");
	printBufFormat(buf, len);

	if (len >= 5&& buf[0] == AT_0D
	&& buf[1] == AT_0A
	&& buf[len-1] == AT_0A
	&&buf[len-2] == AT_0D) {

		return PACKET_VALID;
	} else {
		return PACKET_INVALID;
	}

}
ModulePacketType Module_GetAPacket(uint8_t * buf, uint16_t timeout) {
	HAL_StatusTypeDef result;
	uint16_t len;

	len = 64;
	result = custHAL_UART_Receive(&huart2, buf, &len, timeout);

	if (result == HAL_OK || (result == HAL_TIMEOUT && len > 0)) {
		printf("GetPacket OK %d\r\n", len);
		return parsePacket(buf, len);
	} else {
		printf("GetPacket failed\r\n");
		return PACKET_NONE;
	}
}
int isPacketOK(uint8_t *buf) {
	if (buf[2] == 'O' && buf[3] == 'K') {
		return 0;
	} else {
		return -1;
	}
}
int isPacketRegistered(uint8_t *buf) {
	if (buf[10] == '0' && buf[12] == '1') {
		return 0;
	} else {
		return -1;
	}
}

void initATEnv() {

	printf("Set default AT env\r\n");
	Module_Put("ATE0\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 3000);
	osDelay(1000);

	while (1) {
		osDelay(1000);
		Module_Put("AT\r\n");
		if (Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000)
				== PACKET_VALID
				&& isPacketOK((uint8_t*) moduleThread.rxBuffer) == 0) {
			break;
		}
	}
	printf("\r\n-------- Exit AT env --------\r\n");
}
void testATCmd() {
	printf("Test AT+CPIN?\r\n");
	Module_Put("AT+CPIN?\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("Firmware version AT+CGMR\r\n");
	Module_Put("AT+CGMR\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("Signal quality AT+CSQ\r\n");
	Module_Put("AT+CSQ\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("SIM number AT+ICCID\r\n");
	Module_Put("AT+ICCID\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("IMSI number AT+CIMI\r\n");
	Module_Put("AT+CIMI\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	while (1) {

		printf("Network attachment checking AT+CEREG?\r\n");
		Module_Put("AT+CEREG?\r\n");
		Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
		if (isPacketRegistered((uint8_t*) moduleThread.rxBuffer) == 0) {
			break;
		}
		osDelay(1000);
	}

	printf("Get operator info AT+COPS?\r\n");
	Module_Put("AT+COPS?\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("Get PDP context AT+CGDCONT?\r\n");
	Module_Put("AT+CGDCONT?\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("Get PDP conn info AT+CGACT?\r\n");
	Module_Put("AT+CGACT?\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	while (1) {
		printf("Get PDP address AT+CGPADDR=1\r\n");
		Module_Put("AT+CGPADDR=1\r\n");
		if (Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000) == 0) {
			break;
		}
		osDelay(1000);
	}



	printf("Get network clock AT+CCLK?\r\n");
	Module_Put("AT+CCLK?\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);


	printf("Get IMEI AT+CGSN=1\r\n");
	Module_Put("AT+CGSN=1\r\n");
	Module_GetAPacket((uint8_t*) moduleThread.rxBuffer, 1000);
	osDelay(1000);

	printf("\r\n-------- Exit test AT --------\r\n");
}
