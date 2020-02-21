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
#define FLASH_END_BANK2               ((uint32_t)0x08020000)
#define FLASH_WORDS_BANK2             ((FLASH_END_BANK2 - FLASH_START_BANK2)/4)
#define USER_FLASH_END_ADDRESS        ((uint32_t)0x08020000)

#define WORDS_IN_HALF_PAGE            ((uint32_t)16)
#define FLASH_HALF_PAGE_SIZE          ((uint32_t)WORDS_IN_HALF_PAGE*4)
//#define FLASH_PAGE_SIZE               ((uint32_t)128)

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

void testCRC32();

void dualBankOps(void);

FLASHIF_StatusTypeDef FLASH_If_Erase(void);

FLASHIF_StatusTypeDef FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
FLASHIF_StatusTypeDef FLASH_If_Check(uint32_t start);

uint32_t readMagic(uint32_t offset);

uint32_t saveMagic(uint32_t offset, uint32_t magic);

void printOtherBank(void);

#ifdef __cplusplus
}
#endif


#endif /* DUAL_BANK_H_ */
