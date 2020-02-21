/*
 * dual_bank.c
 *
 *  Created on: 2 Feb 2020
 *      Author: yangjun
 */
#include <stdio.h>
#include "dual_bank.h"
#include "stm32l0xx_hal.h"
#include "main.h"

uint8_t bBFB2;

extern const uint32_t mMAGIC;
extern const uint32_t mMAGIC;
extern const char mPROG[PROG_NAME_MAX_LEN];
extern const uint32_t mVERSION;
extern const uint32_t mDATE;

extern CRC_HandleTypeDef hcrc;

uint8_t bCheckEEPROM(uint32_t offset);
uint8_t fillEEPROM(uint32_t offset);
void menu(void);

/**
 * @brief  Convert an Integer to a string
 * @param  p_str: The string output pointer
 * @param  intnum: The integer to be converted
 * @retval None
 */
/*void Int2Str(uint8_t *p_str, uint32_t intnum) {
 uint32_t i, divider = 1000000000U, pos = 0U, status = 0U;

 for (i = 0U; i < 10U; i++) {
 p_str[pos++] = (uint8_t) ((intnum / divider) + 48U);

 intnum = intnum % divider;
 divider /= 10U;
 if ((p_str[pos - 1U] == '0') && (status == 0U)) {
 pos = 0U;
 } else {
 status++;
 }
 }
 }*/

/**
 * @brief  Convert a string to an integer
 * @param  p_inputstr: The string to be converted
 * @param  p_intnum: The integer value
 * @retval 1: Correct
 *         0: Error
 */

void printBank(uint32_t offset) {
	int i, j;
	uint8_t datax;

	for (i = offset; i < offset + EEPROM_PART_LEN; i = i + 16) {
		printf("%x :", i);
		for (j = i; j < i + 16; j++) {
			datax = *((uint8_t *) j);
			printf("%x ", datax);
		}
		printf("\r\n");
	}
}

void printOtherBank(void) {
	uint32_t i, j;

	for (i = 0; i < 0x10000; i += 16) {
		printf("0x%05x: ", i);
		for (j = 0; j < 4; j++) {
			printf("%08x ", *((uint32_t*)(FLASH_START_BANK2 + i + j*4)));
		}
		printf("\r\n");
	}

}

void printHelp() {
	printf("\r\n");
	printf("===========================================\r\n");
	printf("  Flash binary to the other bank ------- 1\r\n");
	printf("  Erase the other bank ----------------- 2\r\n");
	printf("  Rewrite the other bank --------------- 3\r\n");
	printf("  Check the other bank integrity ------- 4\r\n");
	printf("  Switch bank -------------------------- 5\r\n");
	printf("  Toggle the system bank selection ----- 6\r\n");
	printf("  Quit-----------------------------------q\r\n");
	printf("===========================================\r\n");
}

void testCRC32() {
	// uint8_t buf[9] = { '1', '2', '3', '4', '5', '6', '7', '8' };
	uint8_t buf[9] = { 0x1a, 0x2b, 0x3c, 0x4d };
	uint32_t uCRC;

	uCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *) buf, 1);

	printf("testCRC32 0x%x\r\n", uCRC);
	printf("test xor 0x%x\r\n", 0xFFFFFFFF ^ uCRC);  // This is right!
}

void dualBankOps(void) {
	FLASH_AdvOBProgramInitTypeDef adv_config;
	uint8_t rtn;
	int counter = 0;

	printf("Begin dual_bank ops\r\n");
	// read BFB2 bits
	//	printf("BFB2 bit: %d\r\n", 0x01);
	/* Get the current configuration */
	HAL_FLASHEx_AdvOBGetConfig(&adv_config);
	printf("BootConfig: %d\r\n", adv_config.BootConfig);

	if ((FLASH->OPTR & FLASH_OPTR_BFB2) == FLASH_OPTR_BFB2) {
		printf("Boot from Bank 2\r\n");
		bBFB2 = 1;
	} else {
		printf("Boot from Bank 1\r\n");
		bBFB2 = 0;
	}

	// Read EEPROM out
//	printf("EEPROM base: %x\r\n", DATA_EEPROM_BASE);
//	printf("EEPROM 1 end: %x\r\n", DATA_EEPROM_BANK1_END);
//	printf("EEPROM bank2 : %x\r\n", DATA_EEPROM_BANK2_BASE);
//	printf("EEPROM bank2 end: %x\r\n", DATA_EEPROM_BANK2_END);
	// EEPROM bank 1,
	printf("\r\nBank1:\r\n");
	printBank(EEPROM_BANK1_BASE);
	printf("\r\nBank2:\r\n");
	printBank(EEPROM_BANK2_BASE);

//	printf("Write to EEPROM1:\r\n");
//	datax = 0x00000011U;
//	HAL_FLASHEx_DATAEEPROM_Unlock();
//	HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, DATA_EEPROM_BASE + 0, datax);
//
//	HAL_FLASHEx_DATAEEPROM_Lock();

	// Check current EEPROM state
	while (1) {
		rtn = bCheckEEPROM(EEPROM_BANK1_BASE);
		if (rtn != 0) {
			printf("Error: Current EEPROM invalid! %d\r\n", rtn);
			counter++;
			fillEEPROM(EEPROM_BANK1_BASE);
		} else {
			printf("Current EEPROM is valid.\r\n");
			break;
		}
		if (counter > MAX_EEPROM_CHECK_TIMES) {
			printf("Error: Over EEPROM check max times %d\r\n",
			MAX_EEPROM_CHECK_TIMES);
			while (1) {
			}
		}
	}
	printf("\r\nAfter Check\r\n");
	printf("\r\nBank1:\r\n");
	printBank(EEPROM_BANK1_BASE);
	printf("\r\nBank2:\r\n");
	printBank(EEPROM_BANK2_BASE);

	menu();

}

uint8_t bCheckEEPROM(uint32_t offset) {
	uint32_t uTmp;
	uint32_t uCRC;
	// get magic number
	uTmp = FETCH_MAGIC_VAL(offset);
	if (uTmp != mMAGIC) {
		return -1;
	}

	// calculate CRC
	uCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *) offset,
	EEPROM_CRC_WORDS_LEN);

	// check CRC
	uTmp = FETCH_CRC_VAL(offset);
	if (uCRC != uTmp) {
		return -2;
	}

	return 0;

}
uint8_t fillEEPROM(uint32_t offset) {
	uint32_t uCRC;
	int i;

	HAL_FLASHEx_DATAEEPROM_Unlock();

	// erase page 0
	for (i = 0; i < EEPROM_PART_LEN; i += 16) {
		HAL_FLASHEx_DATAEEPROM_Erase(offset + i);
	}

	// write Magic
	HAL_FLASHEx_DATAEEPROM_Program(
	FLASH_TYPEPROGRAMDATA_WORD, FETCH_MAGIC_POS(offset), mMAGIC);

	// write program name
	for (i = 0; i < PROG_NAME_MAX_LEN; i++) {
		if (mPROG[i] != 0) {
			HAL_FLASHEx_DATAEEPROM_Program(
			FLASH_TYPEPROGRAMDATA_BYTE,
			FETCH_PROG_POS(offset) + i, mPROG[i]);
		}
	}

	// write version
	HAL_FLASHEx_DATAEEPROM_Program(
	FLASH_TYPEPROGRAMDATA_WORD, FETCH_VERSION_POS(offset), mVERSION);

	// write date
	HAL_FLASHEx_DATAEEPROM_Program(
	FLASH_TYPEPROGRAMDATA_WORD, FETCH_DATE_POS(offset), mDATE);

	// calculate CRC
	uCRC = HAL_CRC_Calculate(&hcrc, offset,
	EEPROM_CRC_WORDS_LEN);

	// Write CRC
	HAL_FLASHEx_DATAEEPROM_Program(
	FLASH_TYPEPROGRAMDATA_WORD, FETCH_CRC_POS(offset), uCRC);

	HAL_FLASHEx_DATAEEPROM_Lock();

	return 0;
}

void menu() {
	printHelp();

}
/**
 * @brief  Unlocks Flash for write access
 * @param  None
 * @retval HAL_StatusTypeDef HAL_OK if no problem occurred
 */
static HAL_StatusTypeDef FLASH_If_Init(void) {
	HAL_StatusTypeDef status;

	/* Unlock the Program memory */
	status = HAL_FLASH_Unlock();

	/* Clear all FLASH flags */
	__HAL_FLASH_CLEAR_FLAG(
			FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_RDERR | FLASH_FLAG_FWWERR | FLASH_FLAG_NOTZEROERR);

	return status;
}
/**
 * @brief  This function does an erase of all flash bank area - always the other bank.
 * @param  None
 * @retval FLASHIF_OK: user flash area successfully erased
 *         other: error occurred
 */
FLASHIF_StatusTypeDef FLASH_If_Erase(void) {
	FLASH_EraseInitTypeDef desc;
	FLASHIF_StatusTypeDef result = FLASHIF_OK;
	uint32_t status;

	FLASH_If_Init();

	desc.TypeErase = FLASH_TYPEERASE_PAGES;
	desc.PageAddress = FLASH_START_BANK2;
	desc.NbPages = (USER_FLASH_END_ADDRESS - FLASH_START_BANK2)
			/ FLASH_PAGE_SIZE;
	if (HAL_FLASHEx_Erase(&desc, &status) != HAL_OK) {
		result = FLASHIF_ERASEKO;
	}

	if (status != 0xFFFFFFFFU) {
		result = FLASHIF_ERASEKO;
	}

	HAL_FLASH_Lock();

	return result;

}

/**
 * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
 * @note   After writing data buffer, the flash content is checked.
 * @param  destination: start address for target location
 * @param  p_source: pointer on buffer with data to write
 * @param  length: length of data buffer (unit is 32-bit word)
 * @retval uint32_t 0: Data successfully written to Flash memory
 *         1: Error occurred while writing data in Flash memory
 *         2: Written Data in flash memory is different from expected one
 */
FLASHIF_StatusTypeDef FLASH_If_Write(uint32_t destination, uint32_t *p_source,
		uint32_t length) {
	FLASHIF_StatusTypeDef status = FLASHIF_OK;
	uint32_t *p_actual = p_source; /* Temporary pointer to data that will be written in a half-page space */
	uint32_t i = 0;

	FLASH_If_Init();

	while (p_actual < (uint32_t *) (p_source + length)) {
		/* Write the buffer to the memory */
		if (HAL_FLASHEx_HalfPageProgram(destination, p_actual) == HAL_OK) /* No error occurred while writing data in Flash memory */
		{
			printf("\tp OK\r\n");
			/* Check if flash content matches memBuffer */
			for (i = 0; i < WORDS_IN_HALF_PAGE; i++) {
				if ((*(uint32_t *) (destination + 4U * i)) != p_actual[i]) {
					/* flash content doesn't match memBuffer */
					status = FLASHIF_WRITINGCTRL_ERROR;
					break;
				}
			}

			/* Increment the memory pointers */
			destination += FLASH_HALF_PAGE_SIZE;
			p_actual += WORDS_IN_HALF_PAGE;
		} else {
			printf("\tp Fail\r\n");
			status = FLASHIF_WRITING_ERROR;
		}

		if (status != FLASHIF_OK) {
			break;
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();

	return status;
}

/**
 * @brief  This function does an CRC check of an application loaded in a memory bank.
 * @param  start: start of user flash area
 * @retval FLASHIF_OK: user flash area successfully erased
 *         other: error occurred
 */
FLASHIF_StatusTypeDef FLASH_If_Check(uint32_t start) {
	FLASHIF_StatusTypeDef result = FLASHIF_OK;
	uint32_t size, crc;
	size = *(uint32_t *) (FLASH_END_BANK2 - FLASH_START_BANK2);

	/* checking if size and CRC of the binary is stored in the EEPROM */
//  if ((size == 0) || (size > (FLASH_END_BANK1 - FLASH_START_BANK1)))
//  {
//    result = FLASHIF_RECORD_ERROR;
//  }
	/* checking if the data could be code (first word is stack location) */
	if ((*(uint32_t *) start >> 24U) != 0x20U) {
		printf("Code invalid\r\n");
		result = FLASHIF_EMPTY;
	}

	crc = HAL_CRC_Calculate(&hcrc, (uint32_t *) start, size);

	printf("crc 0x%x\r\n", crc);

//  if (crc != *(uint32_t *)(EEPROM_BANK2_START + EEPROM_CRCC_OFFSET))
//  {
//    result = FLASHIF_CRCKO;
//  }

	return result;
}


/**
  * @brief  Modify the BFB2 status of user flash area.
  * @param  none
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
FLASHIF_StatusTypeDef FLASH_If_BankSwitch(void)
{
  FLASH_AdvOBProgramInitTypeDef adv_config;
  HAL_StatusTypeDef result;
  FLASHIF_StatusTypeDef result1;

  /* Get the current configuration */
  HAL_FLASHEx_AdvOBGetConfig(&adv_config);

  if (adv_config.BootConfig == OB_BOOT_BANK1) /* BANK1 active for boot */
  {
    adv_config.BootConfig = OB_BOOT_BANK2;
  }
  else
  {
    adv_config.BootConfig = OB_BOOT_BANK1;
  }

  /* Initiating the modifications */
  result = HAL_FLASH_OB_Unlock();

  /* program if unlock is successful */
  if (result == HAL_OK)
  {
    result = HAL_FLASHEx_AdvOBProgram(&adv_config);
  }
  if (result == HAL_OK)
  {
    HAL_FLASH_OB_Launch();
  }

  if(result == HAL_OK){
	  result1 = FLASHIF_OK;
  }else{
	  result1 = FLASHIF_WRITINGCTRL_ERROR;
  }

  return result1;
}


