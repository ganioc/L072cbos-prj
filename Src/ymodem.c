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

extern UartTermStr termThread;

uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint8_t aFileName[FILE_NAME_LENGTH];
uint8_t file_size[FILE_SIZE_LENGTH];

/**
 * @brief  Convert a string to an integer
 * @param  p_inputstr: The string to be converted
 * @param  p_intnum: The integer value
 * @retval 1: Correct
 *         0: Error
 */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum) {
	uint32_t i = 0U, res = 0U;
	uint32_t val = 0U;

	if ((p_inputstr[0] == '0')
			&& ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X'))) {
		i = 2U;
		while ((i < 11U) && (p_inputstr[i] != '\0')) {
			if (ISVALIDHEX(p_inputstr[i])) {
				val = (val << 4U) + CONVERTHEX(p_inputstr[i]);
			} else {
				/* Return 0, Invalid input */
				res = 0U;
				break;
			}
			i++;
		}

		/* valid result */
		if (p_inputstr[i] == '\0') {
			*p_intnum = val;
			res = 1U;
		}
	} else /* max 10-digit decimal input */
	{
		while ((i < 11U) && (res != 1U)) {
			if (p_inputstr[i] == '\0') {
				*p_intnum = val;
				/* return 1 */
				res = 1U;
			} else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K'))
					&& (i > 0U)) {
				val = val << 10U;
				*p_intnum = val;
				res = 1U;
			} else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M'))
					&& (i > 0U)) {
				val = val << 20U;
				*p_intnum = val;
				res = 1U;
			} else if (ISVALIDDEC(p_inputstr[i])) {
				val = val * 10U + CONVERTDEC(p_inputstr[i]);
			} else {
				/* return 0, Invalid input */
				res = 0U;
				break;
			}
			i++;
		}
	}

	return res;
}

/* Receive a packet */
static HAL_StatusTypeDef ReceivePacketEx(uint8_t *p_data, uint32_t *p_length,
		uint32_t timeout) {
	uint32_t crc, crctmp;
	uint32_t packet_size = 0U;
	HAL_StatusTypeDef status;
	// COM_StatusTypeDef status1;
	uint8_t char1,i,j;

	*p_length = 0U;

	for(i=0; i< 134;i++){
		p_data[i] = 0;
	}

	printf("\r\n%lu ReceivePacket\r\n", HAL_GetTick());
	status = custHAL_UART_ReceiveEx(&huart1, &p_data[1], 128 + 5, 2000);
	printf("\tstatus1:%d\r\n", status);
	for(i=0; i<133;i+=16){
		printf("%03d: ",i);
		for(j=i;j<i+16;j++){
			printf("%02x ",p_data[j]);
		}
		printf("\r\n");
	}

	if (status == COM_OK) {
		// check length
		char1 = p_data[1];
		switch (char1) {
		case SOH:
			packet_size = PACKET_SIZE;
			break;
		case STX:
			packet_size = PACKET_1K_SIZE;
			break;
		default:
			status = HAL_ERROR;
			break;
		}

		if (packet_size >= PACKET_SIZE) {
			if (p_data[PACKET_NUMBER_INDEX]
					!= ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
				packet_size = 0U;
				status = HAL_ERROR;
			} else {
				crc = (uint32_t) p_data[packet_size + PACKET_DATA_INDEX] << 8U;
				crc += p_data[packet_size + PACKET_DATA_INDEX + 1U];
				//					crctmp = HAL_CRC_Calculate(&hcrc,
				//							(uint32_t *) &p_data[PACKET_DATA_INDEX],
				//							packet_size);
				crctmp = Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size);
				if (crctmp != crc) {
					packet_size = 0U;
					status = HAL_ERROR;
				}
			}
		}

	} else if (status == COM_DATA) { // less than size;
		char1 = p_data[1];
		switch (char1) {
		case EOT:  // Packet size = 0
			status = HAL_OK;
			break;
		case CA:
			if (p_data[2] == CA) {
				packet_size = 2U; // continuous CA CA, abort the transmission
			} else {
				status = HAL_ERROR;
			}
			break;
		case ABORT1:
		case ABORT2:
			status = HAL_BUSY;
			break;
		default:
			status = HAL_ERROR;
		}

	} else if (status == COM_TIMEOUT) {
		status = HAL_ERROR;
	} else if (status == COM_ERROR) {
		status = HAL_ERROR;
	}
	*p_length = packet_size;
	return status;
}
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
	uint32_t crc, crctmp;
	uint32_t packet_size = 0U;
	HAL_StatusTypeDef status;
	uint8_t char1;

	*p_length = 0U;

	printf("\r\nReceivePacket\r\n");
	status = custHAL_UART_Receive(&huart1, &char1, 1U, timeout);
	// printf("\tstatus:%d\r\n",status);
	if (status == HAL_OK) {

		switch (char1) {
		case SOH:
			packet_size = PACKET_SIZE;
			break;
		case STX:
			packet_size = PACKET_1K_SIZE;
			break;
		case EOT:  // Packet size = 0
			break;
		case CA:
			if ((custHAL_UART_Receive(&huart1, &char1, 1U, timeout) == HAL_OK)
					&& (char1 == CA)) {
				packet_size = 2U; // continuous CA CA, abort the transmission
			} else {
				status = HAL_ERROR;
			}
			break;
		case ABORT1:
		case ABORT2:
			status = HAL_BUSY;
			break;
		default:
			status = HAL_ERROR;
			break;
		}
		*p_data = char1;

		// 1024 bytes, or 128 bytes
		if (packet_size >= PACKET_SIZE) {
			status = custHAL_UART_Receive(&huart1, &p_data[PACKET_NUMBER_INDEX],
					(uint16_t) (packet_size + PACKET_OVERHEAD_SIZE), timeout);
			printf("\t2 status:%d\r\n", status);
			/* Simple packet sanity check */
			if (status == HAL_OK) {
				if (p_data[PACKET_NUMBER_INDEX]
						!= ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
					packet_size = 0U;
					status = HAL_ERROR;
				} else {
					/* Check packet CRC */
					crc = (uint32_t) p_data[packet_size + PACKET_DATA_INDEX]
							<< 8U;
					crc += p_data[packet_size + PACKET_DATA_INDEX + 1U];
					//					crctmp = HAL_CRC_Calculate(&hcrc,
					//							(uint32_t *) &p_data[PACKET_DATA_INDEX],
					//							packet_size);
					crctmp = Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size);
					if (crctmp != crc) {
						packet_size = 0U;
						status = HAL_ERROR;
					}
				}
			} else {
				packet_size = 0U;
			}
		}
	} // HAL_OK if(  )
	*p_length = packet_size;
	return status;
}
COM_StatusTypeDef Ymodem_ReceiveEx(uint32_t *p_size) {
	uint32_t i, packet_length, session_done = 0U, file_done, errors = 0U,
			session_begin = 0U;
	uint32_t flashdestination;
	uint32_t ramsource; // ,
	uint32_t filesizeTmp;
	uint8_t *file_ptr, mByte;
	uint8_t tmp, packets_received;
	COM_StatusTypeDef result = COM_OK;

	/* Initialize flashdestination variable */
	flashdestination = FLASH_START_BANK2;

	while ((session_done == 0U) && (result == COM_OK)) {
		packets_received = 0U;
		file_done = 0U;
		while ((file_done == 0U) && (result == COM_OK)) {
			switch (ReceivePacketEx(aPacketData, &packet_length,
			DOWNLOAD_TIMEOUT)) {
			case HAL_OK:
				printf("Rx valid block\r\n");
				errors = 0U;

				switch (packet_length) {
				case 2:
					/* Abort by sender */
					mByte = ACK;
					Serial_PutByte(mByte);
					result = COM_ABORT;
					break;
				case 0:
					/* End of transmission */
					mByte = ACK;
					Serial_PutByte(mByte);
					file_done = 1U;
					break;
				default:
					/* Normal packet */
					if (aPacketData[PACKET_NUMBER_INDEX] != packets_received) {
						Serial_PutByte(NAK);
					} else {
						// First block, block 0
						if (packets_received == 0U) {
							if (aPacketData[PACKET_DATA_INDEX] != 0U) {
								/* File name extraction */
								i = 0U;
								file_ptr = aPacketData + PACKET_DATA_INDEX;
								while ((*file_ptr != 0U)
										&& (i < FILE_NAME_LENGTH)) {
									aFileName[i++] = *file_ptr++;
								}

								/* File size extraction */
								aFileName[i++] = '\0';
								i = 0U;
								file_ptr++;
								while ((*file_ptr != ' ')
										&& (i < FILE_SIZE_LENGTH)) {
									file_size[i++] = *file_ptr++;
								}
								file_size[i++] = '\0';
								Str2Int(file_size, &filesizeTmp);

								/* Test the size of the image to be sent */
								/* Image size is greater than Flash size */
								*p_size = filesizeTmp;
								if (*p_size
										> (FLASH_START_BANK2 - FLASH_START_BANK1)) {
									/* End session */
									tmp = CA;
									HAL_UART_Transmit(&huart1, &tmp, 1U,
									NAK_TIMEOUT);
									HAL_UART_Transmit(&huart1, &tmp, 1U,
									NAK_TIMEOUT);
									result = COM_LIMIT;
								} else {
									/* erase destination area -
									 * always the other bank mapped on 0x08018000*/
									// FLASH_If_Erase();
									Serial_PutByte(ACK);
									Serial_PutByte(CRC16);
								}
							} else {
								/* File header packet is empty, end session */
								// Finished download
								Serial_PutByte(ACK);
								file_done = 1U;
								session_done = 1U;
								break;
							}
						} else { // other blocks, Data packet
							ramsource =
									(uint32_t) &aPacketData[PACKET_DATA_INDEX];
							Serial_PutByte(ACK);
						}
						packets_received++;
						printf("packet_received:%d\r\n", packets_received);
						session_begin = 1U;
					}
					break;
				}
				break;
			case HAL_BUSY: /* Abort actually */
				Serial_PutByte(CA);
				Serial_PutByte(CA);
				result = COM_ABORT;
				break;
			default:
				if (session_begin > 0U) {
					errors++;
				}
				if (errors > MAX_ERRORS) {
					/* Abort communication */
					Serial_PutByte(CA);
					Serial_PutByte(CA);
					result = COM_ABORT;
				} else {
#ifdef USE_DEBUG_YMODEM
					printf("errors:%d\r\n", errors);
#endif
					Serial_PutByte(CRC16); /* Ask for a packet */
				}
				break;
			}
		}
	}
	return result;
}
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size) {
	uint32_t i, packet_length, session_done = 0U, file_done, errors = 0U,
			session_begin = 0U;
	uint32_t flashdestination;
	uint32_t ramsource; // ,
	uint32_t filesizeTmp;
	uint8_t *file_ptr, mByte;
	uint8_t tmp, packets_received;
	COM_StatusTypeDef result = COM_OK;

	/* Initialize flashdestination variable */
	flashdestination = FLASH_START_BANK2;

	while ((session_done == 0U) && (result == COM_OK)) {
		packets_received = 0U;
		file_done = 0U;
		while ((file_done == 0U) && (result == COM_OK)) {
			switch (ReceivePacket(aPacketData, &packet_length,
			DOWNLOAD_TIMEOUT)) {
			case HAL_OK:
#ifdef USE_DEBUG_YMODEM
				printf("Rx valid packet %d %d\r\n", packets_received,
						packet_length);
#endif
				errors = 0U;
				switch (packet_length) {
				case 2:
					/* Abort by sender */
					mByte = ACK;
					Serial_PutByte(mByte);
					result = COM_ABORT;
					break;
				case 0:
					/* End of transmission */
					mByte = ACK;
					Serial_PutByte(mByte);
					file_done = 1U;
					break;
				default:
					/* Normal packet */
					if (aPacketData[PACKET_NUMBER_INDEX] != packets_received) {
						Serial_PutByte(NAK);
					} else {
						if (packets_received == 0U) {
							/* File name packet, 1st block */
							if (aPacketData[PACKET_DATA_INDEX] != 0U) {
								/* File name extraction */
								i = 0U;
								file_ptr = aPacketData + PACKET_DATA_INDEX;
								while ((*file_ptr != 0U)
										&& (i < FILE_NAME_LENGTH)) {
									aFileName[i++] = *file_ptr++;
								}

								/* File size extraction */
								aFileName[i++] = '\0';
								i = 0U;
								file_ptr++;
								while ((*file_ptr != ' ')
										&& (i < FILE_SIZE_LENGTH)) {
									file_size[i++] = *file_ptr++;
								}
								file_size[i++] = '\0';
								Str2Int(file_size, &filesizeTmp);

								/* Test the size of the image to be sent */
								/* Image size is greater than Flash size */
								*p_size = filesizeTmp;
								if (*p_size
										> (FLASH_START_BANK2 - FLASH_START_BANK1)) {
									/* End session */
									tmp = CA;
									HAL_UART_Transmit(&huart1, &tmp, 1U,
									NAK_TIMEOUT);
									HAL_UART_Transmit(&huart1, &tmp, 1U,
									NAK_TIMEOUT);
									result = COM_LIMIT;
								} else {
									/* erase destination area - always the other bank mapped on 0x08018000*/
									// Erase the destination area
									// FLASH_If_Erase();
									Serial_PutByte(ACK);
									Serial_PutByte(CRC16);

									printf("Block 0 is valid\r\n");
								}
							}
							/* File header packet is empty, end session */
							// Finished download
							else {
								Serial_PutByte(ACK);
								file_done = 1U;
								session_done = 1U;
								printf("Block 0 is Invalid\r\n");
								break;
							}
						} else /* Other Data packet */
						{
#ifdef ENCRYPT
							if (HAL_CRYP_AESCTR_Decrypt( &DecHandle, &aPacketData[PACKET_DATA_INDEX], packet_length, &aDecryptData[0],
											NAK_TIMEOUT) != HAL_OK)
							{
								/* End session */
								Serial_PutByte(CA);
								Serial_PutByte(CA);
								result = COM_DATA;
								break;
							}
							ramsource = (uint32_t) & aDecryptData;
#else
							ramsource =
									(uint32_t) &aPacketData[PACKET_DATA_INDEX];
#endif

							/* Write received data in Flash */
							// Write to the FLASH
							//							if (FLASH_If_Write(flashdestination,
							//									(uint32_t *) ramsource, packet_length / 4U)
							//									== FLASHIF_OK) {
							//								flashdestination += packet_length;
							Serial_PutByte(ACK);
							//							} else /* An error occurred while writing to Flash memory */
							//							{
							//								/* End session */
							//								Serial_PutByte(CA);
							//								Serial_PutByte(CA);
							//								result = COM_DATA;
							//							}
						}
						packets_received++;
						printf("received  %d pockets\r\n", packets_received);
						session_begin = 1U;
					}
					break;
				}
				break;
			case HAL_BUSY: /* Abort actually, quit to upper frame */
				Serial_PutByte(CA);
				Serial_PutByte(CA);
				result = COM_ABORT;
				break;
			default: // Receive Errors , HAL_ERROR
				if (session_begin > 0U) {
					errors++;
				}
				if (errors > MAX_ERRORS) {
					/* Abort communication */
					Serial_PutByte(CA);
					Serial_PutByte(CA);
					result = COM_ABORT; // quit to upper frame
				} else {
#ifdef USE_DEBUG_YMODEM
					printf("errors:%d\r\n", errors);
#endif
					Serial_PutByte(CRC16); /* Ask for a packet */
				}
				break;
			}
		} // While receivePacket
	} // While

	return result;
}

HAL_StatusTypeDef SerialDownload() {
	COM_StatusTypeDef status;
	uint32_t size;
	printf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
//	status = Ymodem_Receive(&size);
	status = Ymodem_ReceiveEx(&size);
	return status;
}

void Serial_PutByte(uint8_t param) {
	osEvent event;
	uint16_t length = 1U;

	printf("Serial_PutByte %x\r\n", param);

	if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *) &param, length) != HAL_OK) {
		printf("Error:transmit_DMA\r\n");
		/* Transfer error in transmission process */
		Error_Handler();
		return;
	}
	event = osMessageGet(termThread.txQ, osWaitForever);
	if (event.status == osEventMessage) {
		sprintf(termThread.tmpBuffer, "tx event %lu", event.value.v);
		safePrintf(termThread.tmpBuffer);
	}
}

/**
 * @brief  Update CRC16 for input byte
 * @param  CRC input value
 * @param  input byte
 * @retval None
 */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte) {
	uint32_t crc = crcIn;
	uint32_t in = byte | 0x100;
	do {
		crc <<= 1;
		in <<= 1;
		if (in & 0x100)
			++crc;
		if (crc & 0x10000)
			crc ^= 0x1021;
	} while (!(in & 0x10000));
	return crc & 0xffffu;
}
/**
 * @brief  Cal CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t Cal_CRC16(const uint8_t* data, uint32_t size) {
	uint32_t crc = 0;
	const uint8_t* dataEnd = data + size;
	while (data < dataEnd)
		crc = UpdateCRC16(crc, *data++);

	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);
	return crc & 0xffffu;
}
