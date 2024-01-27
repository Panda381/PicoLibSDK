
// ****************************************************************************
//
//                               I8052 CPU Emulator
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// I8052 ... 12 MHz, 8 KB ROM, 256 B RAM, 32-bit I/O, 3x 16-bit counter, 1x UART
// Clock is divided by 12 steps of one instruction cycle - it gives 1 us, 2 us or 4us per instruction on 12 MHz

// RAM 128 bytes:
//  1) address 0..0x1F (32 bytes): 4 banks of 8 registers R0..R7
//  2) address 0x20.0x2F (16 bytes): bit-addressable space (bit address 0..0x7F)
//  3) 0x30..0x7F: scratch pad area (user data RAM, direct and indirect addressing)
//  4) 0x80..0xFF: Special function registers SFR, direct addressing only
//     0x80..0xFF: extended RAM on 8052, indirect addressing only
// Stack pointer initialized to 0x07

// index addressing - only ROM (dptr = base address, accumulator = offset)

// @TODO: interrupts, timers, hardware

#if USE_EMU_I8052	// use I8052 CPU emulator

// constants
#define I8052_CLOCKMUL		1		// clock multiplier (to achieve lower frequencies and finer timing)
#define I8052_ROMSIZE		0x2000		// ROM size (8 KB)
#define I8052_RAMSIZE		0x100		// RAM size (256 B)
#define I8052_SFR_BASE		0x80		// base address of SFR registers

// flags in PSW
#define I8052_C			B7	// carry
#define I8052_AC		B6	// auxiliary carry
#define I8052_F0		B5	// general purpose status flag
#define I8052_RS1		B4	// register select bit 1
#define I8052_RS0		B3	// register select bit 0 (register bank R0..R7 base address: 0x00, 0x08, 0x10, 0x18)
#define I8052_OV		B2	// overflow
//#define I8052_F1		B1	// user definable flag
#define I8052_P			B0	// parity of Accumulator, 1=odd, 0=even

//#define I8052_RS_SHIFT	3	// shift of register bank select
//#define I8052_RS_MASK		0x03	// mask of register bank select
#define I8052_REGBANK_MASK	0x18	// mask of register bank in PSW (can be added to register index)

// Special function registers SFR, (*) = bit addressable
#define I8052_ADDR_P0		0x80	// (*) P0 port 0 (reset: 0xFF) ... addressable bits 0x80..0x87
#define I8052_ADDR_SP		0x81	// SP stack pointer (reset: 0x07)
#define I8052_ADDR_DPL		0x82	// DPL data pointer low (reset: 0x00)
#define I8052_ADDR_DPH		0x83	// DPH data pointer high (reset: 0x00)
#define I8052_ADDR_PCON		0x87	// PCON power control (reset: 0x00)
#define I8052_ADDR_TCON		0x88	// (*) TCON timer/counter control (reset: 0x00) ... addressable bits 0x88..0x8F
#define I8052_ADDR_TMOD		0x89	// TMOD timer/counter mode control (reset: 0x00)
#define I8052_ADDR_TL0		0x8a	// TL0 timer/counter 0 low byte (reset: 0x00)
#define I8052_ADDR_TL1		0x8b	// TL1 timer/counter 1 low byte (reset: 0x00)
#define I8052_ADDR_TH0		0x8c	// TH0 timer/counter 0 high byte (reset: 0x00)
#define I8052_ADDR_TH1		0x8d	// TH1 timer/counter 1 high byte (reset: 0x00)
#define I8052_ADDR_P1		0x90	// (*) P1 port 1 (reset: 0xFF) ... addressable bits 0x90..0x97
#define I8052_ADDR_SCON		0x98	// (*) SCON serial control (reset: 0x00) ... addressable bits 0x98..0x9F
#define I8052_ADDR_SBUF		0x99	// SBUF serial data buffer (reset: indeterminate)
#define I8052_ADDR_P2		0xa0	// (*) P2 port 2 (reset: 0xFF) ... addressable bits 0xA0..0xA7
#define I8052_ADDR_IE		0xa8	// (*) IE interrupt enable control (reset: 0x00) ... addressable bits 0xA8..0xAF
#define I8052_ADDR_P3		0xb0	// (*) P3 port 3 (reset: 0xFF) ... addressable bits 0xB0..0xB7
#define I8052_ADDR_IP		0xb8	// (*) IP interrupt priority control (reset: 0x00) ... addressable bits 0xB8..0xBF
				//0xc0	// (*) ... addressable bits 0xC0..0xC7
#define I8052_ADDR_PSW		0xd0	// (*) PSW program status word (reset: 0x00) ... addressable bits 0xD0..0xD7
				//0xd8	// (*) ... addressable bits 0xD8..0xDF
#define I8052_ADDR_ACC		0xe0	// (*) ACC accumulator (reset: 0x00) ... addressable bits 0xE0..0xE7
				//0xe8	// (*) ... addressable bits 0xE8..0xEF
#define I8052_ADDR_B		0xf0	// (*) B register (reset: 0x00) ... addressable bits 0xF0..0xF7
				//0xf8	// (*) ... addressable bits 0xF8..0xFF
// 8052 only:
#define I8052_ADDR_T2CON	0xc8	// (*) T2CON timer/counter 2 control (reset: 0x00) ... addressable bits 0xC8..0xCF
#define I8052_ADDR_RCAP2L	0xca	// RCAP2L T/C 2 capture register low byte (reset: 0x00)
#define I8052_ADDR_RCAP2H	0xcb	// RCAP2H T/C 2 capture register high byte (reset: 0x00)
#define I8052_ADDR_TL2		0xcc	// TL2 timer/counter 2 low byte (reset: 0x00)
#define I8052_ADDR_TH2		0xcd	// TH2 timer/counter 2 high byte (reset: 0x00)
// 80C52 only:
#define I8052_ADDR_IPH		0xb7
#define I8052_ADDR_SADDR	0xa9
#define I8052_ADDR_SADEN	0xb9
// Philips 80C52 only:
#define I8052_ADDR_AUXR		0x8e
#define I8052_ADDR_AUXR1	0xa2
// DS5002FP only:
#define I8052_ADDR_CRCR		0xc1
#define I8052_ADDR_CRCL		0xc2
#define I8052_ADDR_CRCH		0xc3
#define I8052_ADDR_MCON		0xc6
#define I8052_ADDR_TA		0xc7
#define I8052_ADDR_RNR		0xcf
#define I8052_ADDR_RPTCL	0xd8
#define I8052_ADDR_RPS		0xda

// interrupt vector address
//  If interrupts arrive simultaneously and have the same interrupt
//  level group, the interrupt with the lower vector address takes
//  precedence. An interrupt with a "high" interrupt level group
//  always takes precedence over a "low" group.
#define I8052_VECT_IE0		0x0003	// IE0
#define I8052_VECT_TF0		0x000B	// TF0
#define I8052_VECT_IE1		0x0013	// IE1
#define I8052_VECT_TF1		0x001B	// TF1
#define I8052_VECT_RITI		0x0023	// RI & TI
#define I8052_VECT_TF2EXF2	0x002B	// TF2 & EXF2

// flags in PCON power control register
//  - unused bits 4,5,6 should be set to 0
//  - not bit addressable
#define I8052_PCON_SMOD		B7	// SMOD: 1=doubled baud rate from Timer1 on serial port modes 1, 2 or 3
#define I8052_PCON_GF1		B3	// GF1: general purpose flag bit
#define I8052_PCON_GF0		B2	// GF0: general purpose flag bit
#define I8052_PCON_PD		B1	// PD: 1=activate power down operation (precedence before IDL flag)
#define I8052_PCON_IDL		B0	// IDL: 1=activate idle mode

// flags in IE interrupt enable register
//  - unused bit 6 should be set to 0
//  - bit addressable
#define I8052_IE_EA_BIT		7	// EA: 0=global disable all interrupts, 1=enable interrupts
#define I8052_IE_ET2_BIT	5	// ET2: 1=enable Timer 2 overflow interrupt
#define I8052_IE_ES_BIT		4	// ES: 1=enable serial port interrupt
#define I8052_IE_ET1_BIT	3	// ET1: 1=enable Timer 1 overflow interrupt
#define I8052_IE_EX1_BIT	2	// EX1: 1=enable external interrupt 1
#define I8052_IE_ET0_BIT	1	// ET0: 1=enable Timer 0 overflow interrupt
#define I8052_IE_EX0_BIT	0	// EX0: 1=enable external interrupt 0

#define I8052_IE_EA		B7	// EA: 0=global disable all interrupts, 1=enable interrupts
#define I8052_IE_ET2		B5	// ET2: 1=enable Timer 2 overflow interrupt
#define I8052_IE_ES		B4	// ES: 1=enable serial port interrupt
#define I8052_IE_ET1		B3	// ET1: 1=enable Timer 1 overflow interrupt
#define I8052_IE_EX1		B2	// EX1: 1=enable external interrupt 1
#define I8052_IE_ET0		B1	// ET0: 1=enable Timer 0 overflow interrupt
#define I8052_IE_EX0		B0	// EX0: 1=enable external interrupt 0

// flags in IP interrupt priority register (sets level group "high" or "low")
//  - unused bits 6,7 should be set to 0
//  - bit addressable
//  - interrupt of "low" level group can be interrupted only by "high" level group
//  - priority within the same level group, from high to low,
//     to resolve simultaneous requests: IE0, TF0, IE1, TF1, RI or TI, TF2 or EXF2 (lower interrupt vector takes precedence)
#define I8052_IP_PT2_BIT	5	// PT2: Timer 2 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PS_BIT		4	// PS: serial port interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PT1_BIT	3	// PT1: Timer 1 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PX1_BIT	2	// PX1: external 1 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PT0_BIT	1	// PT0: Timer 0 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PX0_BIT	0	// PX0: external 0 interrupt 1=high priority level group, 0=low priority level group

#define I8052_IP_PT2		B5	// PT2: Timer 2 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PS		B4	// PS: serial port interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PT1		B3	// PT1: Timer 1 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PX1		B2	// PX1: external 1 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PT0		B1	// PT0: Timer 0 interrupt 1=high priority level group, 0=low priority level group
#define I8052_IP_PX0		B0	// PX0: external 0 interrupt 1=high priority level group, 0=low priority level group

// flags in TCON timer/counter 0 and 1 control register
//  - bit addressable
#define I8052_TCON_TF1_BIT	7	// TF1: 1=Timer 1 overflow flag (set/clear by hardware on overflow/service)
#define I8052_TCON_TR1_BIT	6	// TR1: 1=Timer 1 run enabled (set/clear by software)
#define I8052_TCON_TF0_BIT	5	// TF0: 1=Timer 0 overflow flag (set/clear by hardware on overflow/service)
#define I8052_TCON_TR0_BIT	4	// TR0: 1=Timer 0 run enabled (set/clear by software)
#define I8052_TCON_IE1_BIT	3	// IE1: 1=external interrupt 1 edge flag (set/clear by hardware on edge/service)
#define I8052_TCON_IT1_BIT	2	// IT1: external interrupt 1 type control bit 1=falling edge, 0=low level (set/clear by software)
#define I8052_TCON_IE0_BIT	1	// IE0: 1=external interrupt 0 edge flag (set/clear by hardware on edge/service)
#define I8052_TCON_IT0_BIT	0	// IT0: external interrupt 0 type control bit 1=falling edge, 0=low level (set/clear by software)

#define I8052_TCON_TF1		B7	// TF1: 1=Timer 1 overflow flag (set/clear by hardware on overflow/service)
#define I8052_TCON_TR1		B6	// TR1: 1=Timer 1 run enabled (set/clear by software)
#define I8052_TCON_TF0		B5	// TF0: 1=Timer 0 overflow flag (set/clear by hardware on overflow/service)
#define I8052_TCON_TR0		B4	// TR0: 1=Timer 0 run enabled (set/clear by software)
#define I8052_TCON_IE1		B3	// IE1: 1=external interrupt 1 edge flag (set/clear by hardware on edge/service)
#define I8052_TCON_IT1		B2	// IT1: external interrupt 1 type control bit 1=falling edge, 0=low level (set/clear by software)
#define I8052_TCON_IE0		B1	// IE0: 1=external interrupt 0 edge flag (set/clear by hardware on edge/service)
#define I8052_TCON_IT0		B0	// IT0: external interrupt 0 type control bit 1=falling edge, 0=low level (set/clear by software)

// flags in TMOD timer/counter 0 and 1 mode control register
//  - not bit addressable
#define I8052_TMOD_GATE1	B7	// GATE1: timer/counter 1 run if 1=INT1 pin is high and TR1 is 1 (hardware control), 0=TR1 is 1 (software control)
#define I8052_TMOD_CT1		B6	// C/T1: timer/counter 1 operation 1=counter from T1, 0=timer
#define I8052_TMOD_M11		B5	// M11: timer/counter 1 mode selector bit 1 (I8052_TMOD_MODE_*)
#define I8052_TMOD_M10		B4	// M10: timer/counter 1 mode selector bit 0 (I8052_TMOD_MODE_*)
#define I8052_TMOD_GATE0	B3	// GATE0: timer/counter 0 run if 1=INT0 pin is high and TR0 is 1 (hardware control), 0=TR0 is 1 (software control)
#define I8052_TMOD_CT0		B2	// C/T0: timer/counter 0 operation 1=counter from T0, 0=timer
#define I8052_TMOD_M01		B1	// M01: timer/counter 0 mode selector bit 1 (I8052_TMOD_MODE_*)
#define I8052_TMOD_M00		B0	// M00: timer/counter 0 mode selector bit 0 (I8052_TMOD_MODE_*)

#define I8052_TMOD_M1_SHIFT	4	// timer/counter 1 mode shift
#define I8052_TMOD_M0_SHIFT	0	// timer/counter 0 mode shift

// Timer mode selector
#define I8052_TMOD_MODE_13	0	// 13-bit timer (8048 compatible)
#define I8052_TMOD_MODE_16	1	// 16-bit timer/counter
#define I8052_TMOD_MODE_8	2	// 8-bit auto-reload timer/counter (reload TL from TH)
#define I8052_TMOD_MODE_SPLIT	3	// split mode:
					//   - TL0 is 8-bit timer/counter controlled by Timer 0 control,
					//   - TH0 is 8-bit timer/counter controlled by Timer 1 control,
					//   - timer/counter 1 is stopped

#define I8052_TMOD_MODE_MASK	3	// timer/counter mode mask

// flags in T2CON timer/counter 2 control register (8052 only)
//  - bit addressable
#define I8052_T2CON_TF2_BIT	7	// TF2: 1=Timer 2 overflow flag (set/clear by hardware on overflow/service)
#define I8052_T2CON_EXF2_BIT	6	// EXF2: 1=capture or reload (set by hardware, clear by software)
#define I8052_T2CON_RCLK_BIT	5	// RCLK: receive clock of serial port from 1=Timer 2 overflow, 0=Timer 1 overflow
#define I8052_T2CON_TCLK_BIT	4	// TCLK: transmit clock of serial port from 1=Timer 2 overflow, 0=Timer 1 overflow
#define I8052_T2CON_EXEN2_BIT	3	// EXEN2: Timer 2 external enable flag, 1=capture or reload on negative transition on T2EX
#define I8052_T2CON_TR2_BIT	2	// TR2: 1=Timer 2 run enabled (set/clear by software)
#define I8052_T2CON_CT2_BIT	1	// C/T2: timer/counter 2 operation 1=counter from T2EX falling edge, 0=timer
#define I8052_T2CON_CPRL2_BIT	0	// CP/RL2: timer/counter 2 capture on T2EX negative transition

#define I8052_T2CON_TF2		B7	// TF2: 1=Timer 2 overflow flag (set/clear by hardware on overflow/service)
#define I8052_T2CON_EXF2	B6	// EXF2: 1=capture or reload (set by hardware, clear by software)
#define I8052_T2CON_RCLK	B5	// RCLK: receive clock of serial port from 1=Timer 2 overflow, 0=Timer 1 overflow
#define I8052_T2CON_TCLK	B4	// TCLK: transmit clock of serial port from 1=Timer 2 overflow, 0=Timer 1 overflow
#define I8052_T2CON_EXEN2	B3	// EXEN2: Timer 2 external enable flag, 1=capture or reload on negative transition on T2EX
#define I8052_T2CON_TR2		B2	// TR2: 1=Timer 2 run enabled (set/clear by software)
#define I8052_T2CON_CT2		B1	// C/T2: timer/counter 2 operation 1=counter from T2EX falling edge, 0=timer
#define I8052_T2CON_CPRL2	B0	// CP/RL2: timer/counter 2 capture on T2EX negative transition

// flags on SCON serial port control register
//  - bit addressable
#define I8052_SCON_SM0_BIT	7	// SM0: serial port mode bit 0 (I8052_SCON_MODE_*)
#define I8052_SCON_SM1_BIT	6	// SM1: serial port mode bit 1 (I8052_SCON_MODE_*)
#define I8052_SCON_SM2_BIT	5	// SM2: enable multiprocessor communication
#define I8052_SCON_REN_BIT	4	// REN: 1=enable reception
#define I8052_SCON_TB8_BIT	3	// TB8: transmitted 9th bit
#define I8052_SCON_RB8_BIT	2	// RB8: received 9th bit
#define I8052_SCON_TI_BIT	1	// TI: 1=transmit interrupt flag (set by hardware, clear by software)
#define I8052_SCON_RI_BIT	0	// RI: 1=receive interrupt flag (set by hardware, clear by software)

#define I8052_SCON_SM0		B7	// SM0: serial port mode bit 0 (I8052_SCON_MODE_*)
#define I8052_SCON_SM1		B6	// SM1: serial port mode bit 1 (I8052_SCON_MODE_*)
#define I8052_SCON_SM2		B5	// SM2: enable multiprocessor communication
#define I8052_SCON_REN		B4	// REN: 1=enable reception
#define I8052_SCON_TB8		B3	// TB8: transmitted 9th bit
#define I8052_SCON_RB8		B2	// RB8: received 9th bit
#define I8052_SCON_TI		B1	// TI: 1=transmit interrupt flag (set by hardware, clear by software)
#define I8052_SCON_RI		B0	// RI: 1=receive interrupt flag (set by hardware, clear by software)

#define I8052_SCON_SM_SHIFT	6	// serial mode shift

// serial port mode
#define I8052_SCON_MODE_SHIFT	0	// shift register, baud = osc_fosc/12
#define I8052_SCON_MODE_8_BAUD	1	// 8-bit UART, baud variable, generated by Timer 1 (use mode 8 to reload TH1) or Timer 2 overflow
					//  Timer 1:
					//   baud = (PCON.SMOD + 1) * osc_freq / (32 * 12 * (256 - TH1))
					//   TH1 = 256 - (PCON.SMOD + 1) * osc_freq / (32 * 12 * baud)
					//  Timer 2:
					//   baud = osc_freq / (32 * (65536 - RCAP2))
					//   RCAP2 = 65536 - osc_freq / (32 * baud)
#define I8052_SCON_MODE_9	2	// 9-bit UART, baud fosc/64 (PCON.SMOD = 0) or fosc/32 (PCON.SMOD = 1)
#define I8052_SCON_MODE_9_BAUD	3	// 9-bit UART, baud variable (as mode 1)

#define I8052_SCON_MODE_MASK	3	// mode mask

// IRQ level
#define I8052_IRQ_LEVEL_NO	0	// no IRQ
#define I8052_IRQ_LEVEL_LOW	1	// IRQ level LOW
#define I8052_IRQ_LEVEL_HIGH	2	// IRQ level HIGH

// I8052 CPU descriptor
// - Optimization of thumb1 on RP2040: offset of byte should be <= 31, word <= 62, dword <= 124
// - Entries should be aligned
typedef struct {
	sEmuSync	sync;		// 0x00: time synchronization
	union { u16 pc; struct { u8 pcl, pch; }; }; // program counter PC
	volatile u8	stop;		// 0x0A: 1=request to stop (pause) program execution
	u8		inirq;		// IRQ level in progress I8052_IRQ_LEVEL_*

// @TODO: interrupts, timers, hardware

	volatile u8	int0req;	// 1=external interrupt 0 request (external interrupt signal IT0 is LOW = 0)

	u8		lastport[4];	// last output to the port

	pEmu16Read8	readrom;	// read byte from ROM memory
	pEmu8Read8	readport;	// read byte from port 0..3
	pEmu8Write8	writeport;	// write byte to port 0..3
	pEmu16Write8	writeext;	// write byte to external memory
	pEmu16Read8	readext;	// read byte from external memory
					//	P0 ... address A0..A7/data D0..D7
					//	P2 ... address A8..A15
					//	ALE ... write to address latch A0..A7
					//	WR,RD ... write/read signals
					//	PSEN ... enable memory
	u8		ram[I8052_RAMSIZE]; // 256 bytes RAM (address 0x00..0x7F direct+indirect, address 0x80..0xFF indirect)
	u8		sfr[128];	// SFR special function registers (address 0x80..0xFF direct)

} sI8052;

//STATIC_ASSERT(sizeof(sI8052) == 0x7C, "Incorrect sI8052!");

// current CPU descriptor (NULL = not running)
extern volatile sI8052* I8052_Cpu;

// initialize time synchronization (initialize PWM counter; returns real emulated frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8052_CLOCKMUL is recommended to maintain.
INLINE u32 I8052_SyncInit(sI8052* cpu, int pwm, u32 freq) { return (EmuSyncInit(&cpu->sync, pwm, freq*I8052_CLOCKMUL) + I8052_CLOCKMUL/2)/I8052_CLOCKMUL; }

// initialize I8052 tables
void I8052_InitTab();

// reset processor
void I8052_Reset(sI8052* cpu);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void I8052_SyncStart(sI8052* cpu) { EmuSyncStart(&cpu->sync); }

// execute program (start or continue, until "stop" request)
// Size of code of this function: 6620 code + 1024 jump table = 7644 bytes
// CPU loading at 12 MHz on 240 MHz: used 20-37%, max. 66-83%
void FASTCODE NOFLASH(I8052_Exec)(sI8052* cpu);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
INLINE void I8052_SyncTerm(int pwm) { EmuSyncTerm(pwm); }

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8052_CLOCKMUL is recommended to maintain.
u32 I8052_Start(sI8052* cpu, int pwm, u32 freq);

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8052_Stop(int pwm);

// continue emulation without restart processor
u32 I8052_Cont(sI8052* cpu, int pwm, u32 freq);

// check if emulation is running
INLINE Bool I8052_IsRunning() { return I8052_Cpu != NULL; }

#endif // USE_EMU_I8052
