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

extern UartTermStr  moduleThread;

void Module_Put(uint8_t* param) {
	osEvent event;
	uint16_t length = strlen(param);
	uint8_t ch = 0x1f;

	printf("2:Module_Put %d\r\n", length);
	sprintf(moduleThread.tmpBuffer, "%s", param);
	moduleThread.bInRx = 0;

	if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *) moduleThread.tmpBuffer, length) != HAL_OK) {
		printf("2:Error:transmit_DMA\r\n");
		/* Transfer error in transmission process */
		Error_Handler();
		return;
	}
	event = osMessageGet(moduleThread.txQ, osWaitForever);
	if (event.status == osEventMessage) {
//		sprintf(moduleThread.tmpBuffer, "2:tx event %lu", event.value.v);
//		safePrintf(moduleThread.tmpBuffer);
	}
}
