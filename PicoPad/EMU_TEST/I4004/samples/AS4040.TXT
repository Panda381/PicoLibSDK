Intel 4004/4040 assembler compiler v1.10

(c) 2020-2024 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
 	https://github.com/Panda381/PicoLibSDK
	https://www.breatharian.eu/hw/picolibsdk/index_en.html
	https://github.com/pajenicko/picopad
	https://picopad.eu/en/
License:
	This source code is freely available for any purpose, including commercial.
	It is possible to take and modify the code or parts of it, without restriction.


Program syntax: as4040 file.s
- output is file.bin, file.lst and file.cpp



Non-standard extensions:
	.strict		- strict mode:
				- registers must have name R0..R15
				- register pairs must have name R01..R14R15
	.page [fill]	- align code to 256-byte page, can have fill byte or 0 default
	*=addr[,fill]	- set address, can have fill byte or 0 default
	.set a b	- 'a' will by substituted by 'b' (use text without spaces)
	R01		- Register pair can be entered with name R01, R23, R45, R67, R89, R1011, R1213, R1415
	@label		- low byte of label can be used instead of data byte
	-number		- negate sign can be used before data byte
	'A'		- ASCII character can be used as data byte
	$12, 0x12, 12h	- hex data byte can be entered with $, 0x or H
	par,par2	- parameters can be separated with comma
	= a,b		- constant data can have 1 or 2 bytes

Instruction alternatives:
	JCN 0001,a ... JCN 1,a .... JCN TZ,a ... JT a .... TEST signal is active (TEST = 0)
	JCN 1001,a ... JCN 9,a .... JCN TN,a ... JNT a ... TEST signal is not active (TEST = 1)
	JCN 0010,a ... JCN 2,a .... JCN C1,a ... JC a .... carry is set
	JCN 1010,a ... JCN 10,a ... JCN C0,a ... JNC a ... carry is not set
	JCN 0100,a ... JCN 4,a .... JCN AZ,a ... JZ a .... ACC is zero
	JCN 1100,a ... JCN 12,a ... JCN AN,a ... JNZ a ... ACC is not zero
	JCN 1000,a ... JCN 8,a .... JR a ................. jump relative, unconditional short jump

	JUN a ... JMP a ... jump long
	JMS a ... CALL a ... call subroutine

	ISZ r,a ... IJNZ r,a ... increment and skip is zero / increment and jump if not zero

	IAC ... INC A ... increment accumulator
	DAC ... DEC A ... decrement accumulator

	BBL n ... RET n ... return from subroutine

	LDM d ... LDI d ... load immediate to accumulator

Instruction list:
-----------------
	d ... 8-bit number (byte)
	n ... 4-bit number (nibble)
	s ... 8-bit address (in current page)
	a ... 12-bit address
	r ... register
	rr ... register pair
	c ... condition (4-bit number)
	C ... carry
	A ... accumulator

group 0x00..0x0F:
	NOP ... [8] no operation
	HLT ... [8] stop (only I4040)
	BBS ... [8] return from interrupt (only I4040; interrupt service starts at address 0x003)
	LCR ... [8] load DCL into A (only I4040)
	OR4 ... [8] OR r4 to A (only I4040)
	OR5 ... [8] OR r5 to A (only I4040)
	AN6 ... [8] AND r6 to A (only I4040)
	AN7 ... [8] AND r7 to A (only I4040)
	DB0 ... [8] select ROM bank 0 (only I4040)
	DB1 ... [8] select ROM bank 1 (only I4040)
	SB0 ... [8] select register bank 0, register 0..7 (only I4040)
	SB1 ... [8] select register bank 1, register 0..7 (only I4040)
	EIN ... [8] enable interrupt (only I4040)
	DIN ... [8] disable interrupt (only I4040)
	RPM ... [8] read program memory from address 0xf00+SRC to A, first/last nibble (only I4040)

group 0x10..0xDF:
	JCN n,s ... [16] jump with condition (bit0 TEST is 0 active, bit1 C is set, bit2 A is zero, bit3 negate condition)
		JT s .... jump if TEST is active (TEST=0)
		JNT s ... jump if TEST is not active (TEST=1)
		JC s .... jump if carry is set
		JNC s ... jump if carry is not set
		JZ s .... jump if A is zero
		JNZ s ... jump if A is not zero
		JR s .... jump relative (unconditional short jump)
	FIM rr,d ... [16] fetch immediate data byte d into register pair rr (higher nibble into first register)
	SRC rr ... [8] send register control from register pair
	FIN rr ... [16] fetch indirect data byte from ROM address R0R1 into register pair rr
	JIN rr ... [8] jump indirect to ROM address rr
	JUN a (JMP a) ... [16] jump to address a
	JMS a (CALL a) ... [16] call subroutine a
	INC r ... [8] increment register r
	ISZ r,s (IJNZ r,s) ... [16] increment r and jump if not zero to address s
	ADD r ... [8] add register r and carry to A (A = A + r + carry)
	SUB r ... [8] subtract register r and carry from A (A = A + ~r + ~carry)
	LD r ... [8] load register r to A
	XCH r ... [8] exchange register r and A
	BBL n (RET n) ... [8] return from subroutine with code in A
	LDM n (LDI n) ... [8] load immediate to A

group 0xE0..0xEF:
	WRM ... [8] write A into RAM memory
	WMP ... [8] write A into RAM port
	WRR ... [8] write A into ROM port
	WPM ... [8] write A into program RAM memory to address 0xf00+SRC to A, first/last nibble
	WR0 ... [8] write A into RAM status 0
	WR1 ... [8] write A into RAM status 1
	WR2 ... [8] write A into RAM status 2
	WR3 ... [8] write A into RAM status 3
	SBM ... [8] subtract RAM and carry from A
	RDM ... [8] read RAM memory into A
	RDR ... [8] read ROM port into A
	ADM ... [8] add RAM and carry to A
	RD0 ... [8] read RAM status 0 into A
	RD1 ... [8] read RAM status 1 into A
	RD2 ... [8] read RAM status 2 into A
	RD3 ... [8] read RAM status 3 into A

group 0xF0..0xFF:
	CLB ... [8] clear both (A and carry)
	CLC ... [8] clear carry
	IAC (INC A) ... [8] increrment A (overflow -> carry)
	CMC ... [8] complement carry
	CMA ... [8] complement A
	RAL ... [8] rotate A left through carry (carry<-a3<-a2<-a1<-a0<-carry)
	RAR ... [8] rotate A right through carry (carry->a3->a2->a1->a0->carry)
	TCC ... [8] transmit carry to A (0 or 1), clear carry
	DAC (DEC A) ... [8] decrement A (~borrow -> carry)
	TCS ... [8] transfer carry subtract to A (0->9 or 1->10), clear carry
	STC ... [8] set carry
	DAA ... [8] decimal adjust A (add 6 if A>9 or carry=1 and set carry to 1; or carry not affected otherwise)
	KBP ... [8] keyboard process (A change "1 of 4" to bit number, or 15 if invalid)
	DCL ... [8] designate command line (high byte of RAM address)


Firmware of Busicom 141-PF (Unicom 141P) calculator, based on Intel 4004 processor
==================================================================================
binaries: Tim McNerney and Fred Huettig
disassembly and analysis: Barry Silverman, Brian Silverman, Tim McNerney, Lajos Kintli
more comments and re-edited for AS4 assembly: Miroslav Nemecek

I4004/I4040 CPU speed: 740 kHz, clock period 1.35 us
1-word instruction 8 clock periods = 10.8 us
2-word instructon 16 clock periods = 21.6 us

1 number (1 register) = 16 digit nibbles + 4 status nibbles = 20 nibbles = 80 bits = 10 bytes
1 RAM chip = 320 bits = 80 nibbles = 40 bytes = 4 numbers
4 RAM chips = 1280 bits = 320 nibbles = 160 bytes = 16 numbers
8 RAM banks = 10240 bits = 2560 nibbles = 1280 bytes = 128 numbers

1 ROM chip = 2048 bits = 256 bytes (= 1 page of the program)
I4004: Max. 16 ROM chips = 32768 bits = 4096 bytes
I4040: Max. 32 ROM chips = 65536 bits = 8192 bytes

4 RAM output ports in 1 RAM bank, 32 RAM output ports in 8 RAM banks (select with DCL)
16 ROM input/output ports

Ports
-----
TEST		printer drum sector signal (= row of digits, 0=active, 1=inactive)

ROM0 out	bit0 keyboard matrix column shifter clock (10 bits; 1=start write pulse, 0=stop write pulse)
		bit1 shifter data (for printer and keyboard shifter; keyboard 0=row selected, printer 1=hammer is selected)
		bit2 printer shifter clock (20 bits; 1=start write pulse, 0=stop write pulse)

ROM1 in	bit0..bit3 keyboard matrix rows input (1=column is active, key is pressed)

ROM2 in	bit0 printer drum index signal input (1=active, 0=inactive)
		bit3 printer paper advancing button input (1=button is pressed)

RAM0 out	bit0 printing color (0=black, 1=red)
		bit1 fire print hammers (1=fire hammers)
		bit3 advance printer paper (1=paper is advancing)

RAM1 out	bit0 memory lamp
		bit1 overflow lamp
		bit2 minus sign lamp

Keyboard
--------
Select row0..9 via shifter (ROM0 bit0 clock, ROM0 bit1 data) and read col0..3 (RIM1 bit0..3).

		col0		col1		col2		col3
row0		CM (81)		RM (82)		M- (83)		M+ (84)
row1		SQRT (85)	% (86)		M=- (87)	M=+ (88)
row2		diamond (89)	/ (8A)		* (8B)		= (8C)
row3		- (8D)		+ (8E)		diamond2 (8F)	000 (90)
row4		9 (91)		6 (92)		3 (93)		. (94)
row5		8 (95)		5 (96)		2 (97)		00 (98)
row6		7 (99)		4 (9A)		1 (9B)		0 (9C)
row7		sign (9D)	EX (9E)		CE (9F)		C (A0)
row8		dp0		dp1		dp2		dp3	(decimal point switch, value 0,1,2,3,4,5,6,8)
row9		sw1						sw2	(rounding switch, value 0,1,8)

Printer
-------
Rotating printer drum contains 18 columns with 13 sectors (13 rows = 13 digits), column 16 is empty.
Wait index signal (=sector 0, ROM2 bit0), count sector signals (TEST), output hammer pattern (ROM0 bit1 data, ROM0 bit2 clock),
fire hammers (RAM0 bit1), shift paper (RAM0 bit3). Sector signal (TEST) 28 ms (35.7 Hz), index signal 13*28 = 364 ms (2.74 Hz).
Shifter bits: bit0 = column 17, bit1 = column 18, bit3..bit17 = column 1..15 (order of send digits: M0..M14, skip, col18, col17).

sector		column 1-15	column 17	column 18
0		0		diamond		#
1		1		+		*
2		2		-		I
3		3		X		II
4		4		/		III
5		5		M+		M+
6		6		M-		M-
7		7		^		T
8		8		=		K
9		9		SQRT		E
10		.		%		Ex
11		.		C		C
12		-		R		M

Number registers
----------------
1 number register = 16 characters (M0..M15) + 4 status character (S0..S3)
Calculator uses 2 RAM chips. 1 RAM chip = 4 number registers. Total 8 number registers.

S0: 0 plus, 1 or 15 minus
S1: decimal point position (number of digits behind decimal point)
Digits are adjustes starting with lowest digit value at M0.

Number -75.43:
  M0 M1 M2 M3 M4 M5 M6 M7 M8 M9 M10 M11 M12 M13 M14 M15  S0 S1 S2 S3
  3  4  5  7  0  0  0  0  0  0   0   0   0   0   0   0   15  2 

NR(0)	KR	keyboard register (keyboard buffer); S0=keyboard buffer pointer, S3=status (0 no button, 15 pressed)
NR(1)	WR	working register (input register, 2nd operand); S2=rounding switch (0=float, 1=round, 8=trunc), S3=dec. point switch (0..8)
NR(2)	DR	dividend register (1st operand of mul/div); S2=status (0 no mul/div, 3 mul, 4 div, 8 def. div CR, B mul CR, C div CR)
NR(3)	RR	result register (temporary register); S2=last operation (0 new number, 1 mul/div, 8 add/sub)
NR(4)	CR	constant register; S2=digit entry mode status (0 no decimal point, 1 digit with DP, 8 digit without DP)
NR(5)	SR	sub total register (1st operand of add/sub); S2=overflow status (1 overflow)
NR(6)	TR	main total register (1st operand of add/sub)
NR(7)	MR	memory register
