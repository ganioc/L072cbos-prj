/*
 * nbmodule.h
 *
 *  Created on: 22 Feb 2020
 *      Author: yangjun
 */

#ifndef NBMODULE_H_
#define NBMODULE_H_

#include "cmsis_os.h"
#include "main.h"

#define MODULE_NAME "M5311"
#define AT_RSP      "AT"
#define AT_0D       0x0D
#define AT_0A       0x0A


typedef enum ModulePacketTypeDef {
	PACKET_NONE    = -3,
	PACKET_INVALID = -2,
	PACKET_UNKNOWN = -1,
	PACKET_VALID = 0,
	PACKET_AT
} ModulePacketType;

typedef enum ParseStateDef{
	PS_NONE = 0,
	PS_1D = 1,
	PS_1A = 2,
	PS_BODY,
	PS_2D,
	PS_2A
} ParseState;

enum ModuleState {
OFF_LINE = 0, ON_LINE = 1
};

int Module_Put(char* param);

/***
 * Return a valid packet
 */
ModulePacketType Module_GetAPacket(uint8_t *buf, uint16_t timeout);
void initATEnv();
int isPacketAT(uint8_t *buf);
int isPacketOK(uint8_t *buf);
int isPacketRegistered(uint8_t * buf);
void testATCmd();

#endif /* NBMODULE_H_ */
