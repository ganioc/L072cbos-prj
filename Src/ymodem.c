/*
 * ymodem.c
 *
 *  Created on: 10 Feb 2020
 *      Author: yangjun
 */
#include "ymodem.h"
#include "main.h"
#include "cust_hal_uart.h"
#include <stdio.h>
#include "dual_bank.h"

uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];

uint8_t aFileName[FILE_NAME_LENGTH];

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Receive a packet from sender
 * @param  p_data
 * @param  p_length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @retval HAL_OK: normally return
 *         HAL_BUSY: abort by user
 */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length,
		uint32_t timeout) {

	return HAL_OK;
}

COM_StatusTypeDef Ymodem_Receive(uint8_t *buf) {
	uint8_t*file_ptr, *buf_ptr;
	int32_t i, j, packet_length, session_done, file_done, packets_received,
			errors, session_begin, size = 0;


	return COM_OK;
}

HAL_StatusTypeDef SerialDownload() {
	COM_StatusTypeDef status;
	printf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	status = Ymodem_Receive(&aPacketData[0]);
	return status;
}

