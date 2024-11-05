/*===================================================================*/
/*                                                                   */
/*  K6502_RW.h : 6502 Reading/Writing Operation for NES              */
/*               This file is included in K6502.cpp                  */
/*                                                                   */
/*  2000/5/23   InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef K6502_RW_H_INCLUDED
#define K6502_RW_H_INCLUDED

// PPU hook - access to PPU memory 0x0000 - 0x3FFF (NULL = not used)
extern void (*PPU_hook)(u16 addr);

/*===================================================================*/
/*                                                                   */
/*            K6502_ReadZp() : Reading from the zero page            */
/*                                                                   */
/*===================================================================*/
//BYTE K6502_ReadZp(BYTE byAddr);
INLINE BYTE K6502_ReadZp(BYTE byAddr) { return RAM[byAddr]; }

/*===================================================================*/
/*                                                                   */
/*               K6502_Read() : Reading operation                    */
/*                                                                   */
/*===================================================================*/
BYTE FASTCODE NOFLASH(K6502_Read)(WORD wAddr);

/*===================================================================*/
/*                                                                   */
/*               K6502_Write() : Writing operation                    */
/*                                                                   */
/*===================================================================*/
void FASTCODE NOFLASH(K6502_Write)(WORD wAddr, BYTE byData);

// Reading/Writing operation (WORD version)
WORD FASTCODE NOFLASH(K6502_ReadW)(WORD wAddr);
void FASTCODE NOFLASH(K6502_WriteW)(WORD wAddr, WORD wData);
WORD FASTCODE NOFLASH(K6502_ReadZpW)(BYTE byAddr);

// 6502's indirect absolute jmp(opcode: 6C) has a bug (added at 01/08/15 )
WORD FASTCODE NOFLASH(K6502_ReadW2)(WORD wAddr);

#endif /* !K6502_RW_H_INCLUDED */
