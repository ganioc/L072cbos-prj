/*
 * dual_bank.c
 *
 *  Created on: 2 Feb 2020
 *      Author: yangjun
 */
#include <stdio.h>
#include "dual_bank.h"
#include "stm32l0xx_hal.h"

void printHelp(){
	printf("===========================================\r\n");
	printf("  Flash binary to the other bank ------- 1\r\n");
	printf("  Erase the other bank ----------------- 2\r\n");
	printf("  Rewrite the other bank --------------- 3\r\n");
	printf("  Check the other bank integrity ------- 4\r\n");
	printf("  Switch bank -------------------------- 5\r\n");
	printf("  Toggle the system bank selection ----- 6\r\n");
	printf("===========================================\r\n");
}

void dualBankOps(void) {
	FLASH_AdvOBProgramInitTypeDef adv_config;
	HAL_StatusTypeDef result;

	printf("Begin dual_bank ops\r\n");
	// read BFB2 bits
	//	printf("BFB2 bit: %d\r\n", 0x01);
	/* Get the current configuration */
	HAL_FLASHEx_AdvOBGetConfig(&adv_config);
	printf("BootConfig: %d\r\n", adv_config.BootConfig);

	// Read EEPROM out



	printHelp();
}
