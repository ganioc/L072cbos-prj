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

//uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum) {
//	uint32_t i = 0U, res = 0U;
//	uint32_t val = 0U;
//
//	if ((p_inputstr[0] == '0')
//			&& ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X'))) {
//		i = 2U;
//		while ((i < 11U) && (p_inputstr[i] != '\0')) {
//			if (ISVALIDHEX(p_inputstr[i])) {
//				val = (val << 4U) + CONVERTHEX(p_inputstr[i]);
//			} else {
//				/* Return 0, Invalid input */
//				res = 0U;
//				break;
//			}
//			i++;
//		}
//
//		/* valid result */
//		if (p_inputstr[i] == '\0') {
//			*p_intnum = val;
//			res = 1U;
//		}
//	} else /* max 10-digit decimal input */
//	{
//		while ((i < 11U) && (res != 1U)) {
//			if (p_inputstr[i] == '\0') {
//				*p_intnum = val;
//				/* return 1 */
//				res = 1U;
//			} else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K'))
//					&& (i > 0U)) {
//				val = val << 10U;
//				*p_intnum = val;
//				res = 1U;
//			} else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M'))
//					&& (i > 0U)) {
//				val = val << 20U;
//				*p_intnum = val;
//				res = 1U;
//			} else if (ISVALIDDEC(p_inputstr[i])) {
//				val = val * 10U + CONVERTDEC(p_inputstr[i]);
//			} else {
//				/* return 0, Invalid input */
//				res = 0U;
//				break;
//			}
//			i++;
//		}
//	}
//
//	return res;
//}

void printBank(uint32_t offset){
	int i,j;
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

void dualBankOps(void) {
	FLASH_AdvOBProgramInitTypeDef adv_config;
	uint8_t rtn;
	int counter=0;

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
	uCRC = HAL_CRC_Calculate(
			&hcrc,
			(uint32_t *)offset,
			EEPROM_CRC_PART_LEN);

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
	for(i=0; i< EEPROM_PART_LEN; i+=16){
		HAL_FLASHEx_DATAEEPROM_Erase(offset+i);
	}


	// write Magic
	HAL_FLASHEx_DATAEEPROM_Program(
			FLASH_TYPEPROGRAMDATA_WORD,
			FETCH_MAGIC_POS(offset),
			mMAGIC);

	// write program name
	for(i=0; i< PROG_NAME_MAX_LEN; i++){
		if(mPROG[i] != 0){
			HAL_FLASHEx_DATAEEPROM_Program(
					FLASH_TYPEPROGRAMDATA_BYTE,
					FETCH_PROG_POS(offset) + i,
					mPROG[i]);
		}
	}

	// write version
	HAL_FLASHEx_DATAEEPROM_Program(
			FLASH_TYPEPROGRAMDATA_WORD,
			FETCH_VERSION_POS(offset),
			mVERSION);

	// write date
	HAL_FLASHEx_DATAEEPROM_Program(
			FLASH_TYPEPROGRAMDATA_WORD,
			FETCH_DATE_POS(offset),
			mDATE);


	// calculate CRC
	uCRC = HAL_CRC_Calculate(
			&hcrc,
			offset,
			EEPROM_CRC_PART_LEN);

	// Write CRC
	HAL_FLASHEx_DATAEEPROM_Program(
			FLASH_TYPEPROGRAMDATA_WORD,
			FETCH_CRC_POS(offset),
			uCRC);

	HAL_FLASHEx_DATAEEPROM_Lock();

	return 0;
}

void menu(){
	printHelp();

}
