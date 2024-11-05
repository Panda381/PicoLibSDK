/*===================================================================*/
/*                                                                   */
/*  K6502.h : Header file for K6502                                  */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef K6502_H_INCLUDED
#define K6502_H_INCLUDED

/* 6502 Flags */
#define FLAG_C 0x01
#define FLAG_Z 0x02
#define FLAG_I 0x04
#define FLAG_D 0x08
#define FLAG_B 0x10
#define FLAG_R 0x20
#define FLAG_V 0x40
#define FLAG_N 0x80

/* Stack Address */
#define BASE_STACK 0x100

/* Interrupt Vectors */
#define VECTOR_NMI 0xfffa
#define VECTOR_RESET 0xfffc
#define VECTOR_IRQ 0xfffe

// NMI Request
#define NMI_REQ NMI_State = 0

// IRQ Request
#define IRQ_REQ IRQ_State = 0

// Emulator Operation
void K6502_Init();
void K6502_Reset();
void FASTCODE NOFLASH(K6502_Step)(int wClocks);

INLINE void K6502_Set_Int_Wiring(BYTE byNMI_Wiring, BYTE byIRQ_Wiring)
{
  NMI_Wiring = byNMI_Wiring;
  IRQ_Wiring = byIRQ_Wiring;
}

// I/O Operation (User definition)
BYTE FASTCODE NOFLASH(K6502_ReadAbsX)();
BYTE FASTCODE NOFLASH(K6502_ReadAbsY)();
BYTE FASTCODE NOFLASH(K6502_ReadIY)();

INLINE WORD getPassedClocks() { return g_wCurrentClocks; }

#endif /* !K6502_H_INCLUDED */
