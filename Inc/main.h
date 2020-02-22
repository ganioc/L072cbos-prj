/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define MAX_EEPROM_CHECK_TIMES  10

#define MAGIC_NUMBER       0x01020304U
#define PROG_NAME          {'Y','a','n','g','o'}

/* EEPROM part */
/* EEPROM config part length */
#define EEPROM_BANK1_BASE  DATA_EEPROM_BASE
#define EEPROM_BANK2_BASE  DATA_EEPROM_BANK2_BASE

// sector 0, page 0, 128 bytes
#define EEPROM_PART_LEN    (32 * 4)
#define EEPROM_CRC_PART_LEN  (EEPROM_PART_LEN - 4)
#define EEPROM_CRC_WORDS_LEN (32 -1)

#define EEPROM_MAGIC_POS   0


/* PROG NAME , 16 bytes */
#define EEPROM_PROG_POS    (EEPROM_MAGIC_POS + 1*4)
/* in bytes */
#define PROG_NAME_MAX_LEN  16

#define EEPROM_VERSION_POS (EEPROM_PROG_POS + 4*4)
#define MAJOR_VERSION      1
#define MAJOR_VERSION_POS  (8*3)
#define MINOR_VERSION      0
#define MINOR_VERSION_POS  (8*2)
#define REV_VERSION        3
#define REV_VERSION_POS    (8*1)
#define PATCH_VERSION      0
#define PATCH_VERSION_POS  (8*0)

#define EEPROM_DATE_POS    (EEPROM_VERSION_POS + 4)
#define DATE_YEAR          2020
#define DATE_YEAR_POS      0
#define DATE_DAY           2
#define DATE_DAY_POS       (8*2)
#define DATE_MONTH         4
#define DATE_MONTH_POS     (8*3)

/* FILE CRC result, only for downloaded program file */
#define EEPROM_FILE_CRC_POS    (EEPROM_DATE_POS + 4)

#define EEPROM_STATE_POS       (EEPROM_FILE_CRC_POS + 4)
#define STATE_EMPTY_STATE      0x0
#define STATE_MANUAL_UPDATED   0x01
#define STATE_AUTO_UPDATED     0x10
#define STATE_AUTO_DOWNLOADING_INFO  0x11
#define STATE_AUTO_DOWNLOADED_INFO   0x12
#define STATE_AUTO_DOWNLOADING_FILE  0x13
#define STATE_AUTO_DOWNLOADED_FILE   0x12

#define EEPROM_FILESIZE_TOTAL_POS  (EEPROM_STATE_POS + 4)
#define EEPROM_FIELSIZE_CUR_POS    (EEPROM_FILESIZE_TOTAL_POS + 4)


#define EEPROM_CRC_POS     (EEPROM_PART_LEN - 1*4)

/*  operations on EEPROM  */
#define FETCH_MAGIC_POS(OFFSET)  (OFFSET + EEPROM_MAGIC_POS)
#define FETCH_MAGIC_VAL(OFFSET)  (*((uint32_t *)FETCH_MAGIC_POS(OFFSET)))

#define FETCH_PROG_POS(OFFSET)   (OFFSET + EEPROM_PROG_POS)

#define FETCH_VERSION_POS(OFFSET) (OFFSET + EEPROM_VERSION_POS)

#define FETCH_DATE_POS(OFFSET)    (OFFSET + EEPROM_DATE_POS)

#define FETCH_SIZE_POS(OFFSET)    (OFFSET + EEPROM_SIZE_POS)

#define FETCH_CRC_POS(OFFSET)     (OFFSET + EEPROM_CRC_POS)
#define FETCH_CRC_VAL(OFFSET)     (*((uint32_t *)FETCH_CRC_POS(OFFSET)))


/*  End of EEPROM part  */

#define RXBUFFERSIZE   16

enum UartTermState{
	STATE_NONE=0,
	STATE_DOWNLOADING=1

} ;


typedef struct{
	osThreadId   tId;
	osMessageQId rxQ;
	osMessageQId txQ;
	char rxBuffer[RXBUFFERSIZE];
	char tmpBuffer[RXBUFFERSIZE*2];
	int  oldPos;
	int  newPos;
	uint8_t bInRx;
	enum UartTermState state;
	uint32_t bankActive;
} UartTermStr;


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
