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
/* Notable Flash addresses */
#define FLASH_START_BANK1             ((uint32_t)0x08000000)
#define FLASH_START_BANK2             ((uint32_t)0x08010000)
#define USER_FLASH_END_ADDRESS        ((uint32_t)0x08020000)

/* Error code */
typedef enum
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR,
  FLASHIF_CRCKO,
  FLASHIF_RECORD_ERROR,
  FLASHIF_EMPTY,
  FLASHIF_PROTECTION_ERRROR
} FLASHIF_StatusTypeDef;

/* Exported macros -----------------------------------------------------------*/

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

void dualBankOps(void);

FLASHIF_StatusTypeDef FLASH_If_Erase(void);

uint32_t readMagic(uint32_t offset);

uint32_t saveMagic(uint32_t offset, uint32_t magic);



#ifdef __cplusplus
}
#endif


#endif /* DUAL_BANK_H_ */
