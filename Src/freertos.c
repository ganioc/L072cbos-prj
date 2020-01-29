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

char *charUart1 = "\r\n2\r\n";
char cBuffer[256]={0};
char rxBuffer1[RXBUFFERSIZE] = {0};
int  oldRxBufferPos1 = 0;
int  newRxBufferPos1 = 0;

char tmpChars[32]={0};
char buf[32]={0};
/* USER CODE END Variables */
osThreadId uart1TID;
osThreadId uart2TID;
osThreadId defaultTaskHandle;
osSemaphoreId uart4Semid;
osMessageQId osQueue;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void uart1Thread(void const *argument);
void safePrintf(char * str);
void processRx1Data(char * str, int start, int end);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

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
	osThreadDef(uart1Task, uart1Thread, osPriorityNormal, 0, 128);
	uart1TID = osThreadCreate(osThread(uart1Task), NULL);

	/*  */
	osSemaphoreDef(semLogOut);
	uart4Semid = osSemaphoreCreate(osSemaphore(semLogOut), 1);

	osMessageQDef(osqueue, 1, uint16_t);
	osQueue = osMessageCreate(osMessageQ(osqueue), NULL);

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(2000);
		// safePrintf("default task hello");
//		osSemaphoreWait(uart4Semid, osWaitForever);
//		sprintf(cBuffer,"%d : defaultTask Hello world.\r\n", HAL_GetTick());
//		HAL_UART_Transmit(&huart4, (uint8_t *) cBuffer, strlen(cBuffer),HAL_MAX_DELAY);
//		osSemaphoreRelease(uart4Semid);
		// printf("%d : Go\r\n", HAL_GetTick());
		// HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
	}
	osThreadTerminate(NULL);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void safePrintf(char*str){
	osSemaphoreWait(uart4Semid, osWaitForever);
	sprintf(cBuffer,"%lu : %s\r\n", HAL_GetTick(), str);
	HAL_UART_Transmit(&huart4, (uint8_t *) cBuffer, strlen(cBuffer),HAL_MAX_DELAY);
	osSemaphoreRelease(uart4Semid);
}

void uart1Thread(void const *argument) {
	osEvent event;
	// uint8_t opt = 0;


	safePrintf("uart1 ,Hello world");
	while (1) {
		if (HAL_UART_Receive_DMA(&huart1, (uint8_t *)rxBuffer1, RXBUFFERSIZE) != HAL_OK)
		{
		    /* Transfer error in reception process */
		    Error_Handler();
		}
		event = osMessageGet(osQueue, osWaitForever);
		if (event.status == osEventMessage)
		{
			sprintf(tmpChars,"flag:%d\r\n",__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE));
			safePrintf(tmpChars);
			sprintf(tmpChars,"rxcount:%d s:%lu\r\n",
					huart1.RxXferCount,
					huart1.hdmarx->Instance->CNDTR);
			safePrintf(tmpChars);

			sprintf(tmpChars, "rxevent %lu", event.value.v);
			safePrintf(tmpChars);

			if(event.value.v == 0x20){
				sprintf(tmpChars, "half %s\r\n", rxBuffer1);
				safePrintf(tmpChars);
			}else if(event.value.v == 0x21){
				sprintf(tmpChars, "%s\r\n", rxBuffer1);
				safePrintf(tmpChars);
			}else if(event.value.v == 0x22){
				sprintf(tmpChars, "idle\r\n");
				safePrintf(tmpChars);
			}
			newRxBufferPos1 = RXBUFFERSIZE - huart1.hdmarx->Instance->CNDTR;
			if(newRxBufferPos1 > oldRxBufferPos1){
				processRx1Data(rxBuffer1, oldRxBufferPos1, newRxBufferPos1);
				oldRxBufferPos1 = newRxBufferPos1;
			}else if( newRxBufferPos1 < oldRxBufferPos1){
				processRx1Data(rxBuffer1,oldRxBufferPos1,RXBUFFERSIZE);
				processRx1Data(rxBuffer1,0,newRxBufferPos1);
				oldRxBufferPos1 = newRxBufferPos1;
			}else{

			}
		}

//		while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)
//		{
//		}
//		osSemaphoreWait(uart4Semid, osWaitForever);
//		sprintf(cBuffer,"%d : uart1 Hello world.\r\n", HAL_GetTick());
//		HAL_UART_Transmit(&huart4, (uint8_t *) cBuffer, strlen(cBuffer),HAL_MAX_DELAY);
//		osSemaphoreRelease(uart4Semid);


//		HAL_UART_Transmit(&huart2, (uint8_t *) charUart2, 1,
//		HAL_MAX_DELAY);


//		safePrintf("uart1Thread snd msg\r\n");
//		if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)charUart1, strlen(charUart1)) != HAL_OK)
//		{
//		    /* Transfer error in transmission process */
//		    Error_Handler();
//		}
//		event = osMessageGet(osQueue, osWaitForever);
//		if (event.status == osEventMessage)
//		{
//			safePrintf("uart1Thread recv msg\r\n");
//		      if (event.value.v == QUEUED_VALUE)
//		      {
//		        BSP_LED_On(LED2);
//		        osDelay(LED_TOGGLE_DELAY);
//		        BSP_LED_Off(LED2);
//		      }
//			sprintf(tmpChars, "event %lu", event.value.v);
//			safePrintf(tmpChars);
//			if (event.value.v == 1)
//			{
//
//			}else{
//
//			}
//		}

//		HAL_UART_Transmit(&huart1, (uint8_t *) charUart2, 1,
//		HAL_MAX_DELAY);
		//osDelay(1000);
	}
	osThreadTerminate(NULL);
}
void processRx1Data(char * str, int start, int end){

	osEvent event;
//	uint_8 ch;
	char chs[3] = {0};

	int i=0;
	for(i=0; i< end -start; i++){
		buf[i] = str[start + i];
	}
	buf[i] = 0;
	safePrintf(buf);
	// print ascii code for every character
	for(i=0; i< end - start; i++){
		sprintf(chs,"%d",str[start+i]);
		safePrintf(chs);
	}

	if(buf[i -1 ] == 13){
		buf[i++] = 10;
	}
	buf[i] = 0;



	if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)buf, strlen(buf)) != HAL_OK)
	{
			    /* Transfer error in transmission process */
		Error_Handler();
	}
	event = osMessageGet(osQueue, osWaitForever);
	if (event.status == osEventMessage)
	{
				sprintf(tmpChars, "event %lu", event.value.v);
				safePrintf(tmpChars);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
