/*
 * dual_bank.h
 *
 *  Created on: 2 Feb 2020
 *      Author: yangjun
 */

#ifndef DUAL_BANK_H_
#define DUAL_BANK_H_


#ifdef __cplusplus
extern "C" {
#endif


/* Exported types ------------------------------------------------------------*/



/* Exported macros -----------------------------------------------------------*/

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

void dualBankOps(void);

uint32_t readMagic(uint32_t offset);

uint32_t saveMagic(uint32_t offset, uint32_t magic);



#ifdef __cplusplus
}
#endif


#endif /* DUAL_BANK_H_ */
