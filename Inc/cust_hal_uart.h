/*
 * cust_hal_uart.h
 *
 *  Created on: 30 Jan 2020
 *      Author: yangjun
 */

#ifndef CUST_HAL_UART_H_
#define CUST_HAL_UART_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

#define UART_CHECK_INTERVAL 50

HAL_StatusTypeDef custHAL_UART_Receive_DMA(UART_HandleTypeDef *huart,
		uint8_t *pData,
		uint16_t Size);

void safePrintf(char*str);

HAL_StatusTypeDef custHAL_UART_Receive(
		UART_HandleTypeDef *huart,
		uint8_t *pData,
		uint16_t Size,
		uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* CUST_HAL_UART_H_ */
