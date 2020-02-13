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
#include "dual_bank.h"
#include "cust_hal_uart.h"
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
void handleStateNone(char ch);
void handleStateDownloading(char ch);
void uart1ThreadEx(void const *argument);
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
	termThread.state = STATE_NONE;

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
	osThreadDef(taskUart1, uart1ThreadEx, osPriorityNormal, 0, 256);
	termThread.tId = osThreadCreate(osThread(taskUart1), NULL);

	/*  */
	osSemaphoreDef(semLogOut);
	uart4Semid = osSemaphoreCreate(osSemaphore(semLogOut), 1);

	osMessageQDef(osqueuerx1, 6, uint16_t);
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

void uart1ThreadEx(void const *argument) {

	HAL_StatusTypeDef result;
	FLASHIF_StatusTypeDef resultFlash;
	uint32_t wData = 0x00010001;
	char ch;

	while (1) {
		// osDelay(500);
		printf("\r\nPlease input:\r\n");
		result = custHAL_UART_Receive(&huart1, (uint8_t *) &ch, 1, 5000);
		printf("result:%d\r\n", result);

		if (result != HAL_OK) {
			continue;
		}
		printf("ch:%d\r\n", ch);
		switch (ch) {
		case '0':
			safePrintf("Restart user App\r\n");
			osDelay(2000);
			HAL_NVIC_SystemReset();
			break;
		case '1':
			safePrintf("Download user application into the Flash\r\n");
			result = SerialDownload();
			if (result == HAL_OK) {
				safePrintf("Download succeed\r\n");
			} else {
				printf("Download fail %d\r\n", result);
			}
			break;
		case '2':
			safePrintf("Erase the other bank\r\n");
			resultFlash = FLASH_If_Erase();
			if (resultFlash == FLASHIF_OK) {
				safePrintf("Erase OK\r\n");
			} else {
				safePrintf("Erase fail\r\n");
			}
			break;
		case '3':
			safePrintf("program the other bank\r\n");
			resultFlash = FLASH_If_Write(FLASH_START_BANK2, &wData, 1);
			printf("resultFlash:%d\r\n", resultFlash);
			break;
		case '4':
			safePrintf("Check the other bank\r\n");
			break;
		case '5':
			safePrintf("Check the other bank and exit\r\n");
			break;
		case '6':
			safePrintf("Switch to the other bank\r\n");
			break;
		default:
			break;
		}
	}

}

/***********************************************************************************/

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
