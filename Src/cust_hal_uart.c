/*
 * cust_hal_uart.c
 *
 *  Created on: 30 Jan 2020
 *      Author: yangjun
 */

#include "cust_hal_uart.h"
#include "main.h"
#include <stdio.h>

extern UartTermStr termThread;

/**
 * @brief  End ongoing Rx transfer on UART peripheral (following error detection or Reception completion).
 * @param  huart UART handle.
 * @retval None
 */
static void UART_EndRxTransfer(UART_HandleTypeDef *huart) {
	/* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
	CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
	CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

	/* At end of Rx process, restore huart->RxState to Ready */
	huart->RxState = HAL_UART_STATE_READY;

	/* Reset RxIsr function pointer */
	huart->RxISR = NULL;
}
/**
 * @brief  End ongoing Tx transfer on UART peripheral (following error detection or Transmit completion).
 * @param  huart UART handle.
 * @retval None
 */
static void UART_EndTxTransfer(UART_HandleTypeDef *huart) {
	/* Disable TXEIE and TCIE interrupts */
	CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));

	/* At end of Tx process, restore huart->gState to Ready */
	huart->gState = HAL_UART_STATE_READY;
}

/**
 * @brief DMA UART receive process complete callback.
 * @param hdma DMA handle.
 * @retval None
 */
static void UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma) {
	UART_HandleTypeDef *huart = (UART_HandleTypeDef *) (hdma->Parent);

	/* DMA Normal mode */
	if (HAL_IS_BIT_CLR(hdma->Instance->CCR, DMA_CCR_CIRC)) {
		huart->RxXferCount = 0U;

		/* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
		CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
		CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

		/* Disable the DMA transfer for the receiver request by resetting the DMAR bit
		 in the UART CR3 register */
		CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

		/* At end of Rx process, restore huart->RxState to Ready */
		huart->RxState = HAL_UART_STATE_READY;
	}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
	/*Call registered Rx complete callback*/
	huart->RxCpltCallback(huart);
#else
	/*Call legacy weak Rx complete callback*/
	HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}
/**
 * @brief DMA UART receive process half complete callback.
 * @param hdma DMA handle.
 * @retval None
 */
static void UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma) {
	UART_HandleTypeDef *huart = (UART_HandleTypeDef *) (hdma->Parent);

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
	/*Call registered Rx Half complete callback*/
	huart->RxHalfCpltCallback(huart);
#else
	/*Call legacy weak Rx Half complete callback*/
	HAL_UART_RxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}
/**
 * @brief DMA UART communication error callback.
 * @param hdma DMA handle.
 * @retval None
 */
static void UART_DMAError(DMA_HandleTypeDef *hdma) {
	UART_HandleTypeDef *huart = (UART_HandleTypeDef *) (hdma->Parent);

	const HAL_UART_StateTypeDef gstate = huart->gState;
	const HAL_UART_StateTypeDef rxstate = huart->RxState;

	/* Stop UART DMA Tx request if ongoing */
	if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
			&& (gstate == HAL_UART_STATE_BUSY_TX)) {
		huart->TxXferCount = 0U;
		UART_EndTxTransfer(huart);
	}

	/* Stop UART DMA Rx request if ongoing */
	if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
			&& (rxstate == HAL_UART_STATE_BUSY_RX)) {
		huart->RxXferCount = 0U;
		UART_EndRxTransfer(huart);
	}

	huart->ErrorCode |= HAL_UART_ERROR_DMA;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
	/*Call registered error callback*/
	huart->ErrorCallback(huart);
#else
	/*Call legacy weak error callback*/
	HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

HAL_StatusTypeDef custHAL_UART_Receive_DMA(UART_HandleTypeDef *huart,
		uint8_t *pData, uint16_t Size) {
	/* Check that a Rx process is not already ongoing */
	if (huart->RxState == HAL_UART_STATE_READY) {
		if ((pData == NULL) || (Size == 0U)) {
			return HAL_ERROR;
		}

		/* In case of 9bits/No Parity transfer, pData buffer provided as input parameter
		 should be aligned on a u16 frontier, as data copy from RDR will be
		 handled by DMA from a u16 frontier. */
		if ((huart->Init.WordLength == UART_WORDLENGTH_9B)
				&& (huart->Init.Parity == UART_PARITY_NONE)) {
			if ((((uint32_t) pData) & 1) != 0) {
				return HAL_ERROR;
			}
		}

		/* Process Locked */
		__HAL_LOCK(huart);

		huart->pRxBuffPtr = pData;
		huart->RxXferSize = Size;

		huart->ErrorCode = HAL_UART_ERROR_NONE;
		huart->RxState = HAL_UART_STATE_BUSY_RX;

		if (huart->hdmarx != NULL) {
			/* Set the UART DMA transfer complete callback */
			huart->hdmarx->XferCpltCallback = UART_DMAReceiveCplt;

			/* Set the UART DMA Half transfer complete callback */
			huart->hdmarx->XferHalfCpltCallback = UART_DMARxHalfCplt;

			/* Set the DMA error callback */
			huart->hdmarx->XferErrorCallback = UART_DMAError;

			/* Set the DMA abort callback */
			huart->hdmarx->XferAbortCallback = NULL;

			/* Enable the DMA channel */
			if (HAL_DMA_Start_IT(huart->hdmarx,
					(uint32_t) &huart->Instance->RDR,
					(uint32_t) huart->pRxBuffPtr, Size) != HAL_OK) {
				/* Set error code to DMA */
				huart->ErrorCode = HAL_UART_ERROR_DMA;

				/* Process Unlocked */
				__HAL_UNLOCK(huart);

				/* Restore huart->gState to ready */
				huart->gState = HAL_UART_STATE_READY;

				return HAL_ERROR;
			}
		}
		/* Process Unlocked */
		__HAL_UNLOCK(huart);

		/* Enable the UART Parity Error Interrupt */
		SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);
		SET_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);

		/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
		SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

		/* Enable the DMA transfer for the receiver request by setting the DMAR bit
		 in the UART CR3 register */
		SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

		return HAL_OK;
	} else {
		return HAL_BUSY;
	}
}

void safePrintf(char*str) {
	// osSemaphoreWait(uart4Semid, osWaitForever);
	/*
	 sprintf(cBuffer,"%lu : %s\r\n", HAL_GetTick(), str);
	 HAL_UART_Transmit(&huart4, (uint8_t *) cBuffer, strlen(cBuffer),HAL_MAX_DELAY);
	 */
	printf("%lu : %s\r\n", HAL_GetTick(), str);
	// osSemaphoreRelease(uart4Semid);
}

/* Custom uart receive with time out  */
HAL_StatusTypeDef custHAL_UART_Receive(UART_HandleTypeDef *huart,
		uint8_t *pData, uint16_t size, uint32_t timeout) {

	osEvent event;
	HAL_StatusTypeDef result;
	int counter = 0, i;
	int max_counter =
			((timeout / UART_CHECK_INTERVAL) > 0) ?
					(timeout / UART_CHECK_INTERVAL) : 5;
	// uint32_t oldTick = HAL_GetTick();
	uint32_t pos = 0, old_pos = 0, pIndex = 0;

	termThread.bInRx = 1;
//	termThread.newPos = 0;
//	termThread.oldPos = 0;

	result = custHAL_UART_Receive_DMA(&huart1, pData, size);
#ifdef USE_DEBUG_PRINT
	printf("max_counter:%d\r\n", max_counter);
#endif

	if (result != HAL_OK) {
		/* Transfer error in reception process */
#ifdef USE_DEBUG_PRINT
		printf("Error: start uart1Thread rx DMA error %d\r\n", result);
#endif
		Error_Handler();
		HAL_UART_DMAStop(&huart1);
		return result;
	} else {
#ifdef USE_DEBUG_PRINT
		printf("Start uart1Thread rx DMA OK\r\n", result);
#endif
	}

	while (1) {
		counter++;
		// printf("counter:%d\r\n", counter);
		event = osMessageGet(termThread.rxQ, UART_CHECK_INTERVAL);

		if (event.status == osEventMessage) {
#ifdef USE_DEBUG_PRINT
			sprintf(termThread.tmpBuffer, "rx event %lu", event.value.v);

			safePrintf(termThread.tmpBuffer);
#endif
			pos = size - huart1.hdmarx->Instance->CNDTR;
#ifdef USE_DEBUG_PRINT
			sprintf(termThread.tmpBuffer, "-----pos:%lu", pos);

			safePrintf(termThread.tmpBuffer);
#endif
			if (pos > old_pos) {
				for (i = old_pos; i < pos; i++) {
					pIndex++;
				}
				old_pos = pos;
			}
		}
		if (counter > max_counter) {
			result = HAL_TIMEOUT;
			break;
		}
		if (pIndex >= size) {
			result = HAL_OK;
			break;
		}
	}
	HAL_UART_DMAStop(&huart1);
	return result;
}
COM_StatusTypeDef custHAL_UART_ReceiveEx(UART_HandleTypeDef *huart,
		uint8_t *pData, uint16_t size, uint32_t timeout) {

	osEvent event;
	HAL_StatusTypeDef result;
	int counter = 0, i;
	int max_counter =
			((timeout / UART_CHECK_INTERVAL) > 0) ?
					(timeout / UART_CHECK_INTERVAL) : 10;
	// uint32_t oldTick = HAL_GetTick();
	uint32_t pos = 0, old_pos = 0, pIndex = 0;
	int expected_size = 0;

	termThread.bInRx = 1;

	result = custHAL_UART_Receive_DMA(&huart1, pData, size);
#ifdef USE_DEBUG_PRINT
	printf("max_counter:%d\r\n", max_counter);
#endif

	if (result != HAL_OK) {
		/* Transfer error in reception process */
#ifdef USE_DEBUG_PRINT
		printf("Error: start uart1Thread rx DMA error %d\r\n", result);
#endif
		Error_Handler();
		HAL_UART_DMAStop(&huart1);
		return COM_ERROR;
	} else {
#ifdef USE_DEBUG_PRINT
		printf("Start uart1Thread rx DMA OK\r\n", result);
#endif
	}

	while (1) {
		counter++;
		// printf("counter:%d\r\n", counter);
		event = osMessageGet(termThread.rxQ, UART_CHECK_INTERVAL);

		if (event.status == osEventMessage) {
#ifdef USE_DEBUG_PRINT
			sprintf(termThread.tmpBuffer, "rx event %lu", event.value.v);

			safePrintf(termThread.tmpBuffer);
#endif
			pos = size - huart1.hdmarx->Instance->CNDTR;
#ifdef USE_DEBUG_PRINT
			sprintf(termThread.tmpBuffer, "-----pos:%lu", pos);

			safePrintf(termThread.tmpBuffer);
#endif
			if (pos > old_pos) {
				if(expected_size == 0){
					expected_size = 128+5;
				}
				for (i = old_pos; i < pos; i++) {
					pIndex++;
				}
				old_pos = pos;
			}
		}
		if (counter > max_counter && pIndex == 0) {
			result = COM_TIMEOUT;
			break;
		}else if(counter > max_counter){
			result = COM_DATA;
			break;
		}
		if (expected_size > 0 && pIndex >= expected_size) {
			result = COM_OK;
			break;
		}
	}
	HAL_UART_DMAStop(&huart1);
	return result;
}
