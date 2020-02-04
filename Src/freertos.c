/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
UartTermStr termThread;

osSemaphoreId uart4Semid;
char cBuffer[RXBUFFERSIZE + 24] = { 0 };
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void uart1Thread(void const *argument);
void safePrintf(char * str);
void processRx1Data(char * str, int start, int end);
HAL_StatusTypeDef custHAL_UART_Receive_DMA(UART_HandleTypeDef *huart,
		uint8_t *pData, uint16_t Size);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */
	termThread.bInRx = 0;
	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	osThreadDef(taskUart1, uart1Thread, osPriorityNormal, 0, 192);
	termThread.tId = osThreadCreate(osThread(taskUart1), NULL);

	/*  */
	osSemaphoreDef(semLogOut);
	uart4Semid = osSemaphoreCreate(osSemaphore(semLogOut), 1);

	osMessageQDef(osqueuerx1, 3, uint16_t);
	termThread.rxQ = osMessageCreate(osMessageQ(osqueuerx1), NULL);

	osMessageQDef(osqueuetx1, 3, uint16_t);
	termThread.txQ = osMessageCreate(osMessageQ(osqueuetx1), NULL);

	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument) {
	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(500);

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
	}
	osThreadTerminate(NULL);
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void safePrintf(char*str) {
	// osSemaphoreWait(uart4Semid, osWaitForever);
	/*
	 sprintf(cBuffer,"%lu : %s\r\n", HAL_GetTick(), str);
	 HAL_UART_Transmit(&huart4, (uint8_t *) cBuffer, strlen(cBuffer),HAL_MAX_DELAY);
	 */
	printf("%lu : %s\r\n", HAL_GetTick(), str);
	// osSemaphoreRelease(uart4Semid);
}

void uart1Thread(void const *argument) {
	osEvent event;
	HAL_StatusTypeDef result;
	// uint8_t opt = 0;
	safePrintf("uart1 ,Hello world");
	osDelay(1000);

	while (1) {
		termThread.bInRx = 1;
		result = custHAL_UART_Receive_DMA(&huart1, (uint8_t *) termThread.rxBuffer,
				RXBUFFERSIZE);
		if ( result != HAL_OK) {
			/* Transfer error in reception process */
			printf("Error: uart1Thread rx DMA error %d\r\n", result);
			Error_Handler();
		}
		event = osMessageGet(termThread.rxQ, osWaitForever);
		if (event.status == osEventMessage) {
			//sprintf(termThread.tmpBuffer,
			//		"flag:%d\r\n",__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE));
			// safePrintf(termThread.tmpBuffer);
			sprintf(termThread.tmpBuffer, "-----rxheight:%lu",
					huart1.hdmarx->Instance->CNDTR);
			safePrintf(termThread.tmpBuffer);

			sprintf(termThread.tmpBuffer, "rx event %lu", event.value.v);
			safePrintf(termThread.tmpBuffer);

			if (event.value.v == 0x20) {
//				 sprintf(termThread.tmpBuffer, "half %s", termThread.rxBuffer);
//				sprintf(termThread.tmpBuffer,"half");
//				safePrintf(termThread.tmpBuffer);
			} else if (event.value.v == 0x21) {
//				 sprintf(termThread.tmpBuffer, "%s", termThread.rxBuffer);
//				sprintf(termThread.tmpBuffer,"full");
//				safePrintf(termThread.tmpBuffer);
			} else if (event.value.v == 0x22) {
//				sprintf(termThread.tmpBuffer, "idle");
//				safePrintf(termThread.tmpBuffer);
			}
			termThread.newPos = RXBUFFERSIZE - huart1.hdmarx->Instance->CNDTR;
			sprintf(termThread.tmpBuffer, "siz:%d tr:%lu new:%d old:%d",
			RXBUFFERSIZE, huart1.hdmarx->Instance->CNDTR, termThread.newPos,
					termThread.oldPos);
			safePrintf(termThread.tmpBuffer);

			if (termThread.newPos > termThread.oldPos) {
				processRx1Data(termThread.rxBuffer, termThread.oldPos,
						termThread.newPos);
				termThread.oldPos = termThread.newPos;
			} else if (termThread.newPos < termThread.oldPos) {
				processRx1Data(termThread.rxBuffer, termThread.oldPos,
						RXBUFFERSIZE);
				processRx1Data(termThread.rxBuffer, 0, termThread.newPos);
				termThread.oldPos = termThread.newPos;
			} else {

			}
		}
		osDelay(100);
	}
	osThreadTerminate(NULL);
}
void printRx1Data(char * str, int start, int end) {
	char chs[3] = { 0 };
	char *buf = termThread.tmpBuffer;

	int i;

	if (start == 0 && end == 0) {
		return;
	}

	for (i = 0; i < end - start; i++) {
		buf[i] = str[start + i];
	}
	buf[i] = 0;
	safePrintf(buf);
	// print ascii code for every character
	for (i = 0; i < end - start; i++) {
		sprintf(chs, "%d", str[start + i]);
		safePrintf(chs);
	}

	if (buf[i - 1] == 13) {
		buf[i++] = 10;
	}
	buf[i] = 0;

}
void processRx1Data(char * str, int start, int end) {

	osEvent event;
	// char chs[3] = { 0 };
	char *buf = termThread.tmpBuffer;

	printRx1Data(str, start, end);

	// input is in str[], start, end
	// for response on uart1, tx
	termThread.bInRx = 0;

//	if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *) buf, strlen(buf))
//			!= HAL_OK) {
//		printf("Error:transmit_DMA\r\n");
//		/* Transfer error in transmission process */
//		Error_Handler();
//	}
//	event = osMessageGet(termThread.txQ, osWaitForever);
//	if (event.status == osEventMessage) {
//		sprintf(termThread.tmpBuffer, "tx event %lu", event.value.v);
//		safePrintf(termThread.tmpBuffer);
//	}
}
/***********************************************************************************/

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
