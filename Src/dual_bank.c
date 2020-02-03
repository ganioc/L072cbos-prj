/*
 * dual_bank.c
 *
 *  Created on: 2 Feb 2020
 *      Author: yangjun
 */
#include <stdio.h>
#include "dual_bank.h"
#include "stm32l0xx_hal.h"

void dualBankOps(void) {
	FLASH_AdvOBProgramInitTypeDef adv_config;
	HAL_StatusTypeDef result;

	printf("Begin dual_bank ops\r\n");
	// read BFB2 bits
	//	printf("BFB2 bit: %d\r\n", 0x01);
	/* Get the current configuration */
	HAL_FLASHEx_AdvOBGetConfig(&adv_config);
	printf("BootConfig: %d\r\n", adv_config.BootConfig);

}
