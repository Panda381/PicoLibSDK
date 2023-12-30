; ==== Firmware of Busicom 141-PF (Unicom 141P) calculator, based on Intel 4004 processor ====
; binaries: Tim McNerney and Fred Huettig
; disassembly and analysis: Barry Silverman, Brian Silverman, Tim McNerney, Lajos Kintli
; more comments and re-edited for AS4040 assembly: Miroslav Nemecek

	.strict			; strict mode (requires register names, not register numbers)

; ----- start main loop, clear accumulator and carry
; All registers and memory are cleared on start of processor

Reset:	clb			; clear A and C

; ----- wait for end of drum sector signal

Reset2:	jt	Reset2		; wait while drum sector signal is active (wait if TEST = 0)

; ----- call keyboard service
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: R89 = $00 (select KR)
;	  R67 = $10 (select WR)
;	  A = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Should be called strictly after sector signal becomes inactive (TEST goes -> 1).
; On finish waits for sector signal becomes active (TEST goes -> 0).

	call	Keyboard	; keyboard service (returns R8R9 = $00 to select KR)

; At this point, sector signal is active.

; ----- load 2 digits from keyboard buffer (2x right shift through R13)
; INPUT: A or R13 = new digit to write to M15 (highest digit)
;	 R8 = address of number register
;	 R9 = index of last digit + 1 (usually 0 = shift whole number)
; OUTPUT: R13 = digit from M0

; here is A = 0 = new digit 0 inserted to KR from left; R8R9 = 0 to select KR

	call	ShiftR		; right shift KR (addressed R8R9 = $00) -> R13 digit from KR.M0
	ld	R13		; A <- R13, lower half of scan code from keyboard buffer
	xch	R1		; A -> R1, save lower half of scan code
	clb			; clear A = 0, Carry = 0
	call	ShiftR		; right shift KR -> R13 digit from KR.M0
	ld	R13		; A <- R13, upper half of scan code from keyboard buffer
	jnz	KeyService	; process if key if valid

; ===== no key, status light handling

; ----- get overflow status from SR.S2 -> C
; INPUT: R8 = 1, index of register WR
; OUTPUT: A = 1, C = bit 0 of S2
;	 R8 = 5

	inc	R8		; shift R8R9 to point to default WR (R8 = 1)
	call	ReadSRS2	; read overflow bit, CY <- SR.S2.bit0

; ----- get negative sign
; here is R6R7 = $10, R8 = 5, C = overflow

	src	R67		; selects WR from R6R7
	rd0			; A <- read WR.S0, negative sign
	ral			; rotate A left with C, A.bit1 = minus sign, A.bit0 = overflow
	xch	R3		; R3 <- A, minus and overflow

; ----- get memory flag (check if MR is not 0)
; INPUT: R8R9 = start address of memory register
;	 R5 = function code
;	 C = 0
; OUTPUT: A = 1, C = 1 if number is 0 (or if number is small <= 14 digits)
;	 R8 = 7

	inc	R8		; increment address R8R9, R8 = 6, R9 = 0
	clb			; clear A=0, CY=0
	call	Check0DR	; check if MR is zero
	cmc			; complement Carry, CY=1 if MR is not zero
	xch	R3		; A <- R3, minus and overflow flags
	ral			; rotate A left into bit 0; A.bit2 = minus sign, A.bit1 = overflow, A.bit0 = memory

; ----- output lamp flags to RAM1 port
; here is SRC selected to 7; RAM1 is already selected

	wmp			; output to RAM1 port lamps (bit 0 = memory, bit 1 = overflow, bit 2 = minut)

; ----- advance paper by 1 row, if button "paper advancing" is pressed

	inc	R6		; increment R6 to 2 to select ROM2
	src	R67		; selects ROM2 from R6R7
	rdr			; read ROM2 port (read paper advancing button)
	ral			; rotate A left, get printer paper advancing button from bit 3 -> C
	tcc			; A <- C, 1=button is pressed
	jz	Reset		; jump to main loop if button is not pressed
	call	PaperAdv_2	; advance paper by 1 row (= 4 drum sector signals)
	jmp	Reset		; jump back to main loop

; ===== found key pressed, decrement keyboard buffer pointer by 2
; here is KR selected, R8R9 = 0
; $029

KeyService:
	xch	R0		; R0 <- A, keyboard scan code High

; ----- decrement keyboard buffer pointer KR.S0

	rd0			; load keyboard buffer pointer KR.S0
	dec	A		; decrement A
	dec	A		; decrement A
	wr0			; save new keyboard buffer pointer to KR.S0

; ----- read decimal point switch into WR.S3
; here is selected row8 of keyboard matrix

	src	R67		; select ROM1 and WR from R6R7 (here is R6R7 = $10)
	rdr			; read ROM1 port - decimal point switch
	wr3			; write it to WR.S3 - number of decimal places

; ----- read rounding switch into WS.S2
; INPUT: R8R9 = ROM port address, R8 must be 0 ($0x=ROM0)
; OUTPUT: A = 0

	call	Key1Shift	; send bit '1' to keyboard shifter, shift to select rounding switch
	src	R67		; select ROM1 and WR from R6R7
	rdr			; read ROM1 port - runding switch
	wr2			; write it to WR.S2 - rounding switch

; ----- translate key code ($81..$A0) to function code and parameter, from table KeyTab

	fin	R45		; fetch table PC:R1R0 to R4R5, translate scan code into function code and parameter

; ----- translate function code to macro address -> R0R1 (table MacroTab)

	fim	R01,@MacroTab0	; $A0; R0R1 <- $A0, translation table
	ld	R5		; A <- R5, function code
	xch	R1		; A <-> R1
	fin	R01		; fetch table PC:R0R1 to R0R1, translate function code to macro address

; ----- read overflow bit from SR.S2.bit0 -> C
; here is R8R9 = 0

	inc	R8		; increment R8R9 address to point default WR
	call	ReadSRS2	; read overflow bit, C <- SR.S2.bit0

; ----- clear status lamps
; here is selected RAM1 from R8R9 = $50

	ldi	0		; A <- 0
	wmp			; output RAM1, clear status lamps

; ----- overflow blocks functions, only C and CE are allowed

	ldi	1		; A <- 1
	cmc			; complement C, 0 if overflow
	ral			; A.bit0 <- !overflow flag (0 if overflow), bit1 = 1
				; here is A = 2 if overflow, or 3 if not oferflow
	kbp			; convert, A <- 15 if not overflow, 2 if overflow
; function code: 1=div/mul, 2=+/-, 3=M+/M-, 4=Ex, 5=diamonds, 6=00, 7=RM,
;	8= =,M=+/M=-, 9=Sqrt, 10=%, 11=CM, 12=000, 13=digit, sign, DP, 14=CE, 15=C
	add	R5		; A <- A + R5 (here is C=0), add function code
				; - if there is no overflow, all functions set CY flag
				; - if there is overflow, only 'C' and 'CE' functions set CY flag
	jnc	Reset		; go to main loop if function is blocked
	clb			; clear A and C
	nop
	nop

; Here is: R0R1 macro address at page $300, R12=drum sector counter

; ----------------------------------------------------------------------------
;                        Macro code interpreter
; ----------------------------------------------------------------------------
; 	R0R1 = macro code pointer (index in page $300)
;	R2R3 = command code
;	R4 = parameter (defined by last pressed button)
;	R5 = function code (defined by last pressed button)
;	R6R7 = $20 pointer to DR number register
;	R8R9 = $10 pointer to WR number register
;	R10R11 = decimal point counter
;	R12 = printer drum sector counter
;	R13 = digit, loop counting
;	R14 = rounding indicator
;	R15 = print position
;	A = 0
;	CY = 0
; On return: A = 0 no jump, A = 1 address follow: CY = 1 jump or CY = 0 skip address
; ----------------------------------------------------------------------------
; $04B

; ----- call keyboard service - only if sector signal is not active (= end of sector drum signal)
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: R89 = $00 (select KR)
;	  R67 = $10 (select WR)
;	  A = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Should be called strictly after sector signal becomes inactive (TEST goes -> 1).
; On finish waits for sector signal becomes active (TEST goes -> 0).

Exec:	jt	Exec2		; skip keyboard service if sector signal is active
	call	Keyboard	; call keyboard service

; ----- prepare source and destination number address

Exec2:	fim	R67,$20		; R6R7 <- source number address (=DR)
				;          bit 0..3: select nibble 0
				;          bit 4..5: select register 2 = DR
				;          bit 6..7: select RAM chip 0
	fim	R89,$10		; R8R9 <- destination number address (=WR)
				;          bit 0..3: select nibble 0
				;          bit 4..5: select register 1 = WR
				;          bit 6..7: select RAM chip 0

; ----- execute one command code
; - load command address from page $300, pointer $R0R1, into R2R3
; - execute command at addres R2R3 on page $100

	call	CmdFetch	; fetch command code into R2R3 from page $300 = function address LOW
	call	ExecCode	; execute code (command in R2R3 is LOW part of function address at page $100)

; ----- increment macro code pointer R0R1

Exec3:	ijnz	R1,Exec4	; increment register R1, skip if not zero (low part of code pointer)
	inc	R0		; increment register R0 (high part of code pointer)

; ----- continue next command if returned A was 0, or process jump command if returned A was 1

Exec4:	jz	Exec		; A = 0, no jump, continue with next command

; ----- A = 1, process jump command: skip next byte (jump address) if C = 0, do jump if C = 1

	tcc			; A <- C, A = 0 to skip next byte, A = 1 to jump
	jz	Exec3		; C = 0: skip next byte (skip jump address)

; ----- if A = 1 and C = 1 go to Jump macro (fetch jump address byte)

	jmp	JmpFetch	; jump to address from next byte into R0R1 (=new macro pointer)


; ----- Keyboard service - no column is active, only increase button scan code by 4
; $061

KeyNone:ldi	4		; A <- 4, number of buttons at one row (to increase button scan code by 4)
	jmp	Keyboard5	; go back

; ----------------------------------------------------------------------------
;                   Send bit to keyboard/printer shifter
; ----------------------------------------------------------------------------
; INPUT: A = output to ROM0 port
;		bit 0 = keyboard shifter clock (1 active)
;		bit 1 = shifter data
;		bit 2 = printer hammer shifter clock (1 active)
;	 R8R9 = ROM port address, R8 must be 0 ($0x=ROM0)
; OUTPUT: A = 0
; ----------------------------------------------------------------------------
; $064

; ----- send '1' to keyboard shifter (1 = row not selected)

Key1Shift:
	ldi	3		; A <- 3, keyboard shifter clock = 1, shifter data = 1

; ----- send A to keyboard/printer shifter

SendShift:
	src	R89		; select ROM0 from R8R9 = $0x
	wrr			; write shifter data from A - start write pulse
	ldi	0		; A <- 0 (clock and data = 0)
	wrr			; write shifter data from A - stop write pulse
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                          Synchronize printer drum
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: A = 0, C = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Called strictly after sector signal becomes inactive (TEST goes -> 1).
; Increments R12 (sector counter), or clears R12 if index signal is active.
; $06A

; ----- increment drum sector counter

DrumSync:
	inc	R12		; increment drum sector counter
	fim	R23,$20		; R2R3 <- $20 to select ROM2 port

; ----- check drum index signal (bit0 of ROM2 port), wait 15 loops

DrumSync2:
	src	R23		; select ROM2 from R2R3=$2x (only bits 4..7 are important)
	rdr			; A <- read ROM2 port
	rar			; A.bit0 -> C (= printer drum index signal)
	ijnz	R3,DrumSync2	; increment R3 and loop back (short wait, loop 15-times)

; ----- reset sector counter if index signal is active (index signal is active if C = 1)

	jnc	DrumSync4	; jump if C = 0, printer drum index is inactive
	clb			; index is active -> clear A and C
	xch	R12		; clear R12 (printer drum sector counter)
DrumSync4:
	ret	0		; return with A = 0, C = 0


; ----- Keyboard service - multiply keys pressed, clear keyboard buffer
; $077

KeyErr:	ld	R9		; A <- R9, scan code (0 = no key, 15 = multiply keys)
	jz	Keyboard6	; jump back to next row if no button is pressed in this column

KeyErr2:fim	R89,$00		; R8 <- 0 to select KR (keyboard register), R9 <- 0 digit index
	clb			; A <- 0 (to write to digits), C <- 0
	call	ClearWR	; clear keyboard buffer
	jmp	Keyboard7	; exit from keyboard handling

; ----------------------------------------------------------------------------
;            Keyboard decode table to translate keyboard scan code into
;         function code (low digit = R5) and parameter (high digit = R4)
; ----------------------------------------------------------------------------
; keyboard scan code: bit 7: 1=button pressed flag, bit 0..6=key index 1..32
; function code: 1=div/mul, 2=+/-, 3=M+/M-, 4=Ex, 5=diamonds, 6=00, 7=RM,
;	8= =,M=+/M=-, 9=Sqrt, 10=%, 11=CM, 12=000, 13=digit, sign, DP, 14=CE, 15=C
; parameter to print: 0..9=0..9, 11=.
;	or 0=diamond, 1=+, 2=-, 3=X, 4=/, 5=M+, 6=M-, 10=Ex, 11=C, 12=R

; $081

*= $81	; scan code of first button

KeyTab:	

; row0
= $bb	; $81 CM (function 11 = CM fn_clrmem, parameter 11 = print C at column 17)
= $c7	; $82 RM (function 7 = RM fn_rm, parameter 12 = print R at column 17)
= $63	; $83 M- (function 3 = M+/M- fn_memadd, parameter 6 = print M- at column 18)
= $53	; $84 M+ (function 3 = M+/M- fn_memadd, parameter 5 = print M+ at column 18)
; row1
= $19	; $85 SQRT (function 9 = Sqrt fn_sqrt, parameter 1)
= $1a	; $86 % (function 10 = % fn_percnt, parameter 1)
= $68	; $87 M=- (function 8 = =/M=+/M=- fn_memeq, parameter 6 = print M-)
= $58	; $88 M=+ (function 8 = =/M=+/M=- fn_memeq, parameter 5 = print M+)
; row2
= $05	; $89 diamond (function 5 = diamond fn_diamnd, parameter 0)
= $41	; $8A / (function 1 = mul/div fn_muldiv, parameter 4 = last operation /)
= $31	; $8B * (function 1 = mul/div fn_muldiv, parameter 3 = last operation *)
= $18	; $8C = (function 8 = =/M=+/M=- fn_memeq, parameter 1)
; row3
= $22	; $8D - (function 2 = +/- fn_addsub, parameter 2 = last operation -)
= $12	; $8E + (function 2 = +/- fn_addsub, parameter 1 = last operation +)
= $05	; $8F diamond2 (function 5 = diamond fn_diamnd, parameter 0)
= $0c	; $90 000 (function 12 = 1100 = enter 2 more digits 0 fn_digit, parameter 0 = enter digit 0)
; row4
= $9d	; $91 9 (function 13 = digit fn_digit, parameter 9 = enter digit 9)
= $6d	; $92 6 (function 13 = digit fn_digit, parameter 6 = enter digit 6)
= $3d	; $93 3 (function 13 = digit fn_digit, parameter 3 = enter digit 3)
= $bd	; $94 . (function 13 = digit fn_digit, parameter 11 = enter decimal point)
; row5
= $8d	; $95 8 (function 13 = digit fn_digit, parameter 8 = enter digit 8)
= $5d	; $96 5 (function 13 = digit fn_digit, parameter 5 = enter digit 5)
= $2d	; $97 2 (function 13 = digit fn_digit, parameter 2 = enter digit 2)
= $06	; $98 00 (function 6 = 01100 = enter 1 more digit 0 fn_digit, parameter 0 = enter digit 0)
; row6
= $7d	; $99 7 (function 13 = digit fn_digit, parameter 7 = enter digit 7)
= $4d	; $9A 4 (function 13 = digit fn_digit, parameter 4 = enter digit 4)
= $1d	; $9B 1 (function 13 = digit fn_digit, parameter 1 = enter digit 1)
= $0d	; $9C 0 (function 13 = digit fn_digit, parameter 0 = enter digit 0)
; row7
= $ad	; $9D S (sign +/-) (function 13 = digit fn_digit, parameter 10 = sign)
= $a4	; $9E EX (function 4 = Ex fn_ex, parameter 10 = print Ex at column 18)
= $0e	; $9F CE (function 14 = CE fn_cleare, parameter 0)
MacroTab0:	; start of MacroTab with low nibble = 0
= $bf	; $A0 C (function 15 = C fn_clear, parameter 11 = print C)

; ----------------------------------------------------------------------------
;      Table for translating function code to macro address (on page $300)
; ----------------------------------------------------------------------------
; $0A1

;*= $A1	; first entry

MacroTab:
= @fn_muldiv	; $06	; 1: div/mul
= @fn_addsub	; $91	; 2: +/-
= @fn_memadd	; $98	; 3: M+/M-
= @fn_ex	; $f1	; 4: Ex
= @fn_diamnd	; $cd	; 5: diamonds
= @fn_digit	; $d7	; 6: 00
= @fn_rm	; $fd	; 7: RM
= @fn_memeq	; $8a	; 8: =/M=+/M=-
= @fn_sqrt	; $05	; 9: Sqrt
= @fn_percnt	; $61	; 10: %
= @fn_clrmem	; $f9	; 11: CM
= @fn_digit	; $d7	; 12: 000
= @fn_digit	; $d7	; 13: digit/sign/DP
= @fn_cleare	; $ca	; 14: CE
= @fn_clear	; $c5	; 15: C

; ----------------------------------------------------------------------------
;                               Keyboard service
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: R89 = $00 (select KR)
;	  R67 = $10 (select WR)
;	  A = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Should be called strictly after sector signal becomes inactive (TEST goes -> 1).
; On finish waits for sector signal becomes active (TEST goes -> 0).
; $0B0

; ----- synchronize printer drum (count sector conter R12)
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: A = 0, C = 0
; DESTROYS: R2, R3

Keyboard:
	call	DrumSync	; synchronize printer drum (increment sector counter by 1, or clear to 0)

; ----- prepare loop counter and ROM0 address

Keyboard1:
	fim	R89,$07		; R8 <- 0 to select ROM0 chip, R9 <- 7 loop counter (16 - 7 = 9)

; ----- clear keyboard shifter - send 9 bits '1' (= unselect all keyboard rows)
; INPUT: R8R9 = ROM port address, R8 must be 0 ($0x=ROM0)
; OUTPUT: A = 0

Keyboard2:
	call	Key1Shift	; send bit '1' to keyboard shifter = rows not selected
	ijnz	R9,Keyboard2	; loop with incrementing R9, send 9 bits '1' to unselect all rows

; ----- prepare to scan keyboard

	fim	R67,$18		; R6 <- $18 to select ROM1, R7 <- 8 loop counter (16 - 8 = 8 rows)
	fim	R23,$00		; R2R3 <- 0, scan code counter

; ----- select keyboard row 0 (here is R8 = 0, to select ROM0)
; INPUT: A = output to ROM0 port
;		bit 0 = keyboard shifter clock (1 active)
;		bit 1 = shifter data
;		bit 2 = printer hammer shifter clock (1 active)
;	 R8R9 = ROM port address, R8 must be 0 ($0x=ROM0)
; OUTPUT: A = 0

	ldi	1		; A <- 1, keyboard shifter clock = 1, shifter data = 0
	call	SendShift	; send '0' to keyboard shifter = select row 0

; ----- read columns of current selected row

Keyboard3:
	src	R67		; select ROM1 from R6R7=$1x
	rdr			; A <- read ROM1 port, read state of columns col0..col3

; ----- decode keyboard into column number (1=key is pressed)

	kbp			; decode columns (0->0, 1->1, 2->2, 4->3, 8->4, other->15)

; ----- check is some button has been pressed in another row
; - skips counting of scan code after founding pressed button

	xch	R9		; R9 <- A, save scan code
	ld	R2		; A <- R2, pressed flag
	ral			; C <- R2.bit3 (flag of pressed key in another row)
	tcc			; A <- C (flag of pressed key), C <- 0
	jnz	KeyErr		; another button has been already pressed (continue to Keyboard6 if ok)

; ----- check if max. 1 key is pressed (scan code is 15 if multiply keys pressed)

	ld	R9		; A <- key code from R9 (15 = multiply keys)
	ijnz	R9,Keyboard4	; increment and jump if max. 1 column is active
	jmp	KeyErr2		; multiply keys pressed (code = 15), clear buffer and exit

; ----- no key is pressed (here is A = key code)

Keyboard4:
	jz	KeyNone		; A = 0, no key pressed (set A=4, continue at Keyboard5)

; ----- set flag of pressed key into R2 bit 3 (R2 = scan code counter HIGH)

	xch	R2		; A <- R2 (scan code counter)
	ral			; shift left
	stc			; set C (= key was pressed flag)
	rar			; shift right, 1 -> A.bit3
	xch	R2		; R2 <- A (with set bit 3)

; ----- add scan code 1..4 to scan code counter (here is A = key code 1..4, or 4 if no key, C = 0)
; - no key adds 4 to scan code counter
; - valid key adds 1..4 to scan code counter
; - after that, other keys are not counted
; - bit 3 of R3 is set, to indicate pressed button
; This gives scan codes $81, $82,...$A0

Keyboard5:
	add	R3		; A <- R3 + A, add key column
	xch	R3		; R3 <- A (sum of columns)
	ldi	0		; A <- 0
	add	R2		; A <- R2 + C, add carry from R3
	xch	R2		; R2 <- A (scan code counter HIGH)

; ----- select next row of matrix (here is R8 = 0 to select ROM0), loop 8 rows
; INPUT: R8R9 = ROM port address, R8 must be 0 ($0x=ROM0)
; OUTPUT: A = 0

Keyboard6:
	call	Key1Shift	; shift next bit '1' into keyboard shifter
	ijnz	R7,Keyboard3	; increment R7 loop counter, go to next row

; ----- check if any button pressed

	src	R89		; select KR (keyboard register) from R8R9 (R8 = 0)
	ld	R2		; A <- R2, R2.bit3 = button pressed flag
	ral			; C <- A.bit3, button pressed flag
	tcc			; A <- C, button pressed flag
	jz	Keyboard8	; no buttton pressed, write A = 0 into KR.S3

; ----- check if button is already held down (KR.S3 = 15 if held down)

	rd3			; A <- load keyboard pressing status KR.S3 (15 pressed, 0 no)
	inc	A		; increment A, set CY if key held down (status = 15)
	tcc			; A <- C 1=press, 0=no press, C <- 0
	jnz	Keyboard7	; jump if button is already held down

; ----- write scan code LOW into keyboard buffer (scan code = $81..$A0)

	rd0			; read A <- KR.S0, keyboard buffer pointer
	xch	R9		; R9 <- KR.S0, keyboard buffer pointer
	src	R89		; select digit in KR, from R8R9 (here is R8 = 0)
	ld	R3		; A <- R3, scan code LOW
	wrm			; write scan code LOW from A into keyboard buffer

; ----- check overflow keyboard buffer - next digit must be 0

	inc	R9		; increment address R9 to point to next digit
	src	R89		; select next digit in KR, from R8R9
	rdm			; A <- read next digit from KR
	jnz	KeyErr2		; if next digit is not 0, overflow KR, jump to clear

; ----- write scan code HIGH into keyboard buffer (scan code = $81..$A0)

	ld	R2		; A <- R2, scan code HIGH
	wrm			; write scan code HIGH

; ----- write new keyboard buffer pointer

	inc	R9		; increment address to point to next digit
	ld	R9		; A <- R9, keyboard buffer pointer
	wr0			; write A -> KR.S0, keyboard buffer pointer

; ----- write new state of button: 15=button is held down, 0=button is not held down

Keyboard7:
	ldi	15		; A <- 15, flag of button is held down
Keyboard8:
	wr3			; write new KR.S3 state of button is held down (15=held, 0=no)

; ----- reset pointers to KR and WR

	fim	R89,$00		; R8R9 <- $00 to select KR
	fim	R67,$10		; R6R7 <- $10 to select WR

; ----- wait for active printer drum sector signal

Keyboard9:
	jnt	Keyboard9	; wait if TEST signal is not active (wait if TEST = 1)
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                 Execute macro command - jump to address PC:R2R3
; ----------------------------------------------------------------------------
; INPUT: R2R3 = command code = LOW address at page $100
; ----------------------------------------------------------------------------
; $100

; must be ROM-page aligned
.page	; address $100

ExecCode:
	jin	R23		; run to address PC:R2R3 (R2R3 = command code = LOW address)

; ----------------------------------------------------------------------------
;                     Macro command - move number register
; ----------------------------------------------------------------------------
; INPUT: R6R7 = source number register (R7=0), default $20 = select DR
;	 R8R9 = destination register (R9=0), default $10 = select WR
; ----------------------------------------------------------------------------

; ----- copy number from R8R9 (=WR) to indirect register R5+4 (=IR), with S0 and S1
; IR = indirect register = function code R5 + 4
; $101 MOV IR,WR		move IR <- WR
MovIRWR:ld	R5		; A <- R5, function code

; ----- copy number from R8R9 (=WR) to R6R7+2 (=CR), with S0 and S1
; $102 MOV CR,WR		move CR <- WR
MovCRWR:inc	A			; A + 1

; ----- copy number from R8R9 (=WR) to R6R7+1 (=RR), with S0 and S1
; $103 MOV RR,WR		move RR <- WR
MovRRWR:inc	A			; A + 1

; ----- copy number from R8R9 (=WR) to R6R7 (=DR), with S0 and S1
; $104 MOV DR,WR		move DR <- WR
MovDRWR:add	R6		; A <- A + R6 (C = 0)
	xch	R8		; R8 <- A destination index, A <- R8 old source index
	xch	R6		; R6 <- A source index
	jmp	MovWRDR	; move number

; ----- copy number from R6R7+5 (=MR) to R8R9 (=WR), with S0 and S1
; $109 MOV WR,MR		move WR <- MR
MovWRMR:inc	R6		; source = MR (R6 <- 7)

; ----- copy number from R6R7+4 (=TR) to R8R9 (=WR), with S0 and S1
; $10A MOV WR,TR		move WR <- TR
MovWRTR:inc	R6		; source = TR (R6 <- 6)

; ----- copy number from R6R7+3 (=SR) to R8R9 (=WR), with S0 and S1
; $10B MOV WR,SR		move WR <- SR
MovWRSR:inc	R6		; source = SR (R6 <- 5)

; ----- copy number from R6R7+2 (=CR) to R8R9 (=WR), with S0 and S1
; $10C MOV WR,CR		move WR <- CR
MovWRCR:inc	R6		; source = CR (R6 <- 4)

; ----- copy number from R6R7+1 (=RR) to R8R9 (=WR), with S0 and S1
; $10D MOV WR,RR		move WR <- RR
MovWRRR:inc	R6		; source = RR (R6 <- 3)

; ----- copy number from R6R7 (=DR) to R8R9 (=WR), with S0 and S1
; $10E MOV WR,DR		move WR <- DR
MovWRDR:src	R67		; select source number register from R6R7
	rdm			; A <- read one digit from source register
	src	R89		; select destination register from R8R9
	wrm			; write one digit from A
	inc	R9		; increment destination character R9
	ijnz	R7,MovWRDR	; increment destination digit index R7 (0..15), go next digit

; ----- copy status digits

	src	R67		; select source number register R6R7
	rd0			; A <- read status digit S0
	xch	R3		; A -> R3
	rd1			; A <- read status digit S1
	src	R89		; select destination register R8R9
	wr1			; write status digit S1 from A
	xch	R3		; A <- R3
	wr0			; write status digit S0 from A
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                       Macro command - add two numbers
; ----------------------------------------------------------------------------
; INPUT: R6R7 = destination number register (R7=0), default $20 = select DR
;	 R8R9 = source register (R9=0), default $10 = select WR
; ----------------------------------------------------------------------------

; ----- add numbers R5+4 (IR) <- R5+4 (IR) + R8R9 (WR)
; IR = indirect register = function code R5 + 4
; $11E ADD IR,WR		add IR <- IR + WR
AddIRWR:ldi	4		; A <- 4, target will be IR
	add	R5		; A <- 4 + R5, add function code
	xch	R6		; R6 <- A, destination to IR = R5 + 4

; ----- add numbers R6R7 (DR) <- R6R7 (DR) + R8R9 (WR)
; $121 ADD DR,WR		add DR <- DR + WR
AddDRWR:src	R89		; select source number address (WR) from R8R9
	rdm			; A <- read one digit
	src	R67		; select destination number address (DR) from R6R7
	adm			; A <- A + mem (C = 0)
	daa			; decimal correction
	wrm			; write result into destination number address R6R7
	inc	R9		; increase source index R9
	ijnz	R7,AddDRWR	; increase destination index R7 and loop next digit
	clc			; clear Carry
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                       Macro command - subtract two numbers
; ----------------------------------------------------------------------------
; INPUT: R6R7 = destination number register (R7=0), default $20 = select DR
;	 R8R9 = source register (R9=0), default $10 = select WR
;	 R13 = digit counter
; OUTPUT: R13 = incremented if positive (count digit of division)
;	  A = 1
;	  C = 1 jump if positive, 0 skip if negative
; ----------------------------------------------------------------------------

; ----- subtract two numbers R8R9 (WR) <- R8R9 (WR) - R5+4 (IR), jump if non-negative with increment digit R13
; IR = indirect register = function code R5 + 4
; $12C SUB WR,IR + JPC NNEG + INC DIGIT		subtract WR <- WR - IR, jump at non-negative with increment the digit
SubWRIR:ldi	4		; A <- 4
	add	R5		; A <- 4 + R5, function code + 4
	xch	R8		; R8 <- A, set source to IR
	jmp	SubIRWR_2	; set destination address to old R8 = 1 (WR)

; ----- subtract two numbers R5+4 (IR) <- R5+4 (IR) - R8R9 (WR), jump if non-negative with increment digit R13
; IR = indirect register = function code R5 + 4
; $131 SUB IR,WR + JPC NNEG + INC DIGIT		subtract IR <- IR - WR, jump at non-negative with increment the digit
SubIRWR:ldi	4		; A <- 4
	add	R5		; A <- 4 + R5, function code + 4
SubIRWR_2:
	xch	R6		; R6 <- A, set destination to IR

; ----- subtract two numbers R6R7 (DR) <- R6R7 (DR) - R8R9 (WR), jump if non-negative with increment digit R13
; $134 SUB DR,WR + JPC NNEG + INC DIGIT		subtract DR <- DR - WR, jump at non-negative with increment the digit
SubDRWR:stc			; C <- 1 (= no borrow)
SubDRWR_2:
	tcs			; transfer carry A <- 9 + C (A <- 9 or 10), CY = 0
SubDRWR_3:
	src	R89		; select source number address (WR) from R8R9
	sbm			; A <- 10(9) + ~digit + ~C
	clc			; clear carry C <- 0
	src	R67		; select destination number address (DR) from R6R7
	adm			; A <- A + digit (C = 0)
	daa			; decimal adjust
SubDRWR_4:
	wrm			; write digit to destination memory
	inc	R9		; increase source index R9
	ijnz	R7,SubDRWR_2	; increase destination index R7 and loop next digit
SubDRWR_6:
	jnc	SubDRWR_7	; skip if not carry (= negative)
	inc	R13		; increment R13 if positive, digit counter of div result
SubDRWR_7:
	ret	1		; return with A = 1, C = 0 skip if negative, C = 1 jump if positive (repeat div loop)

; ----------------------------------------------------------------------------
;             Macro - clear numeric register (including S0 and S1)
; ----------------------------------------------------------------------------
; INPUT: R8R9 = destination register (R9=0), default $10 = select WR
;	 A = value to write to digits (default 0)
; OUTPUT: A = 0
;	  R8 = new index
; ----------------------------------------------------------------------------

; ----- clear register R8R9+6 (MR)
; $144 CLR MR		clear MR
ClearMR:inc	R8		; destination = MR

; ----- clear register R8R9+5 (TR)
; $145 CLR TR		clear TR
ClearTR:inc	R8		; destination = TR

; ----- clear register R8R9+4 (SR)
; $146 CLR SR		clear SR
ClearSR:inc	R8		; destination = SR

; ----- clear register R8R9+3 (CR)
; $147 CLR CR		clear CR
ClearCR:inc	R8		; destination = CR

; ----- clear register R8R9+2 (RR)
; $148 CLR RR		clear RR
ClearRR:inc	R8		; destination = RR

; ----- clear register R8R9+1 (DR)
; $149 CLR DR		clear DR
ClearDR:inc	R8		; destination = DR

; ----- clear register R8R9 (WR)
; $14A CLR WR		clear WR
ClearWR:src	R89		; select memory register WR from R8R9
	wrm			; write digit from A
	ijnz	R9,ClearWR	; increment digit index R9 and loop

; ----- clear status registers

	wr0			; write S0 = sign
	wr1			; write S1 = place of decimal point
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
; Macro command - left shift number register (R13 <- M15 <- M14..M1 <- M0 <- R13)
; ----------------------------------------------------------------------------
; INPUT: R13 = new digit to write to M0 (lowest digit)
;	 R8 = address of number register
;	 R9 = index of first digit (usually 0 = shift whole number)
; OUTPUT: R13 = digit from M15
;	  R8 = new index
; ----------------------------------------------------------------------------

; ----- left shift register R8R9+2 (RR)
; $151 SHL RR		left shift RR with R13
ShiftL_RR:
	inc	R8		; shift target address = RR

; ----- left shift register R8R9+1 (DR)
; $152 SHL DR		left shift DR with R13
ShiftL_DR:
	inc	R8		; shift target address = DR

; ----- left shift register R8R9 (WR)
; $153 SHL WR		left shift WR with R13
ShiftL:	src	R89		; select number register WR from R8R9
	rdm			; A <- read one digit
	xch	R13		; A <-> R13, exchange digits
	wrm			; write previous digit
	ijnz	R9,ShiftL	; increment digit index, loop next digit
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
; Macro command - right shift number register (A or R13 -> M15 -> M14..M1 -> M0 -> R13)
; ----------------------------------------------------------------------------
; INPUT: A or R13 = new digit to write to M15 (highest digit)
;	 R8 = address of number register
;	 R9 = index of last digit + 1 (usually 0 = shift whole number)
; OUTPUT: R13 = digit from M0
;	  R8 = new index
; ----------------------------------------------------------------------------

; ----- right shift register R8R9+2 (RR), only 14 lower digits
; $15A SSR RR		right shorted shift RR, only 14 digits (R13 = new digit)
ShiftRShort_RR:
	ldi	14		; A <- 14, shift 14 digits
	xch	R9		; R9 <- 14, index of last digit
	ld	R13		; A <- R13, inserted digit

; ----- right shift register R8R9+2 (RR)
; $15D SHR RR		right shift RR (A = new digit)
ShiftR_RR:
	inc	R8		; shift target address = RR

; ----- right shift register R8R9+1 (DR)
; $15E SHR DR		right shift DR (A = new digit)
ShiftR_DR:
	inc	R8		; shift target address = DR

; ----- right shift register R8R9 (WR)
; $15F SHR WR		right shift WR (A = new digit)
ShiftR:	xch	R13		; R13 <- A, will write 0 to highest digit
	ld	R9		; A <- R9, index of 1st digit, should be 0
ShiftR_2:
	dec	A		; A = A - 1, decrement index of digit
	clc			; clear carry C <- 0
	xch	R9		; R9 <- A, new digit index
	src	R89		; select number register WR from R8R9
	rdm			; A <- read one digit
	xch	R13		; A <- previous digit, R13 <- this digit
	wrm			; write previous digit A
	ld	R9		; A <- R9, index of the digit
	jnz	ShiftR_2	; loop if A != 0 (we will end at M0)
	ret	0		; return wit A = 0

; ----------------------------------------------------------------------------
;                      Macro command - check if status S2 is 0
; ----------------------------------------------------------------------------
; INPUT: R8 = destination register, default $10 = select WR
; OUTPUT: A = 1, C = 1 if S0 = 0
;	  R8 = new index
; ----------------------------------------------------------------------------

; ----- check RR.S2 and jump if new number
; $16C JPC MODENN		jump if new number is entered and not processed (jump if RR.S2 = 0)
JmpNewNum:
	inc	R8		; address = RR

; ----- check DR.S2 and jump if not div/mul specified
; $16D JPC MOPN		jump if div/mul not specified (jump if DR.S2 = 0)
JmpNMul:inc	R8		; address = DR

; ----- check WR.S2 and jump if not truncated/rounded
; $16E JPC NTRUNC		jump if number if not truncated/rounded (jump if WR.S2 = 0)
JmpNTrunc:
	src	R89		; select register WR from R8R9
	rd2			; A <- read status S2
	dec	A		; decrement, C <- 0 if 0, or 1 if >0
	cmc			; complement C (C is 1 if S0 was 0)
	ret	1		; return with A = 1, C = 1 jump if S0 = 0

; ----------------------------------------------------------------------------
;               Macro command - check if bit 0 of status S2 is 1
; ----------------------------------------------------------------------------
; INPUT: R8 = destination register, default $10 = select WR
; OUTPUT: A = 1, C = bit0 of S2
;	  R8 = new index
; ----------------------------------------------------------------------------

; ----- Read status SR.S2.bit0 - 'overflow' flag -> C
; $173 JPC OVFL		jump at overflow (jump if SR.S2.bit0!=0)
ReadSRS2:
	inc	R8		; address = SR

; ----- Read status CR.S2.bit0 - 'digit with DP' flag -> C
; $174 JPC MENTDP		jump if number entered with DP (jump if CR.S2.bit0!=0)
ReadCRS2:
	inc	R8		; address = CR

; ----- Read status RR.S2.bit0 - 'mul/div' last operation flag -> C
; $175 JPC MODEMD		jump if mul/div (jump if RR.S2.bit0!=0)
ReadRRS2:
	inc	R8		; address = RR

; ----- Read status DR.S2.bit0 - 'mul started' flag -> C
; $176 JPC MOPMUL		jump if mul started (jump if DR.S2.bit0!=0)
ReadDRS2:
	inc	R8		; address = DR

; ----- Read status WR.S2.bit0 - rounding switch "1=round" -> C
; $177 JPC ROUND		jump if rounding (jump if WR.S2.bit0!=0)
ReadWRS2:
	src	R89		; select register WR from R8R9
	rd2			; A <- read status S2
	rar			; rotate bit0 into C
	ret	1		; return with A = 1, C = 1 jump if S0.bit0 = 1

; ----------------------------------------------------------------------------
;      Macro command - check if bit 3 of status S2 is 1 (mul/div constant CR)
; ----------------------------------------------------------------------------
; INPUT: R6 = destination number register, default S2 = select DR
; OUTPUT: A = 1, C = bit3 of S2 (DR.S2.bit3 = mul/div constant in CR)
; ----------------------------------------------------------------------------

; $17B JPC MOPCONST		jump if mul/div constant (jump if DR.S2.bit3!=0)
CheckDivCR:
	src	R67		; select address R6R7
	rd2			; read status S2
	ral			; rotate bit 3 into C
	ret	1		; return with A = 1, C = 1 jump if S0.bit3 = 1 (=mul/div constant CR)

; ----------------------------------------------------------------------------
;                         Macro command - clear status S2
; ----------------------------------------------------------------------------
; INPUT: R6 = destination number register, default 2 = select DR
;	 A = 0 (to write to S2)
; OUTPUT: A = 0
; ----------------------------------------------------------------------------

; ----- clear status SR.S2 (overflow)
; $17F CLR OVFL		clear overflow (SR.S2=0)
ClearSRS2:
	inc	R6		; target = SR

; ----- clear status CR.S2 (digit entry mode)
; $180 CLR DIGMODE	clear digit entry mode
ClearCRS2:
	inc	R6		; target = CR

; ----- clear status RR.S2 (last operation)
; $181 CLR LASTOP	clear last operation
ClearRRS2:
	inc	R6		; target = RR

; ----- clear status DR.S2 (div/mul status)
; $182 CLR MOP		clear div/mul (DR.S2=0)
ClearDRS2:
	src	R67		; select register DR from R6R7
	wr2			; write status 2 to 0 (from A)
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                Macro command - set status S2
; ----------------------------------------------------------------------------
; INPUT: R6 = destination number register, default 2 = select DR
; OUTPUT: A = 0
; ----------------------------------------------------------------------------

; ----- set SR.S2 overflow status to 1
; $185 SET OVFL		set overflow (SR.S2=1)
SetOver:inc	R6		; target = SR

; ----- set CR.S2 digit entry mode stats to 1 = digit with DP
; $186 SET MENTDP		set number with decimal point (CR.S2=1)
SetNumDP:
	inc	R6		; target = CR

; ----- set RR.S2 status (last operation) to 1 = mul/div
; $187 SET MODEMD		set mul/div (RR.S2=1)
SetMulDiv:
	ldi	1		; A <- 1
	jmp	ClearRRS2	; set status RR.S2 to 1

; ----- set RR.S2 status (last operation) to 8 = add/sub
; $18A SET MODEAS		set add/sub (RR.S2=8)
SetAddSub:	
	ldi	8		; A <- 8
	jmp	ClearRRS2	; set status RR.S2 to 8

; ----- set DR.S2 status (mul/div status to function parameter R4)
; $18D SET MOPPAR		set mul/div param (DR.S2=param)
SetMulPar:
	ld	R4		; A <- function parameter from R4
	jmp	ClearDRS2	; set status DR.S2 to R4

; ----------------------------------------------------------------------------
;  Macro command - set status DR.S2.bit3 to 1 (set mul/div status constant CR)
; ----------------------------------------------------------------------------
; INPUT: R6 = destination number register, default 2 = select DR
; ----------------------------------------------------------------------------

; $190 SET MOPCONST		set mul/div const (DR.S2.bit3=1)
SetDivCR:
	src	R67		; select register DR from R6R7
	rd2			; A <- read status 2
	ral			; rotate A left, C <- A.bit3
	stc			; set C
	rar			; rotate A right, A.bit3 <- 1
	wr2			; write status 2 (with bit3 set)
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                  Macro command - check zero or small number
; ----------------------------------------------------------------------------
; INPUT: R8 = destination register, default $10 = select WR
;	 R5 = function code
;	 C = 0
; OUTPUT: A = 1, C = 1 if number is 0 (or if number is small <= 14 digits)
; ----------------------------------------------------------------------------

; ----- Check if highest 2 digits of number IR (=R5+4) are zero (returns C = 1 if number is small)
; IR = indirect register = function code R5 + 4
; $197 JPC NBIG_IR		jump if IR not big (upper 2 digits are 0)
CheckSmallIR:
	ldi	4		; A <- 4
	add	R5		; A <- 4 + R5 (C = 0) = function code + 4
	xch	R8		; R8 <- A, function code + 4 (register index)

; ----- Check if highest 2 digits of number R8R9 (WR) are zero (returns C = 1 if number is small)
; $19A JPC NBIG_WR		jump if WR not big (upper 2 digits are 0)
CheckSmallWR:
	ldi	14		; A <- 14
	xch	R9		; R9 <- 14, index of first digit to test
	jmp	Check0WR

; ----- Check if highest 2 digits of number R8R9+1 (DR) are zero (returns C = 1 if number is small)
; $19E CLR DIGIT + JPC NBIG_DR		clear digit (R13=0), jump if DR not big (opper 2 digits are 0)
CheckSmallDR:
	ldi	14		; A <- 14
	xch	R9		; R9 <- 14, index of first digit to test

; ----- Check if number R8R9+1 (DR) is zero (returns C = 1 if zero)
; $1A0 CLR DIGIT + JPC ZERO_DR		clear digit (R13=0), jump if DR is zero
Check0DR:
	xch	R13		; R13 <- 0 (A = 0), first digit
	inc	R8		; register + 1

; ----- Check if number R8R9 (WR) is zero (returns C = 1 if zero)
; $1A2 JPC ZERO_WR		jump if WR = 0
Check0WR:
	src	R89		; select register WR from R8R9
	ldi	15		; A <- 15
	adm			; A <- 15 + digit (C = 0), check if digit is zero, C <- 1 if digit is not 0, 0 if digit is 0
	ijnz	R9,Check0WR	; increment R9 and loop to next digit
				; result is C = 1 if number is not zero
; ----- (unconditional) jump
; $1A7 JMP			jump
Jump:	cmc			; complement Carry
	ret	1		; return with A = 1, Carry = 1 to jump

; ----------------------------------------------------------------------------
;                  Macro command - check digit R13
; ----------------------------------------------------------------------------
; INPUT: R13 = digit
; ----------------------------------------------------------------------------

; ----- check digit R13, set C = 1 if R13 > 0
; $1A9 JPC BIG_DIGIT	jump if digit > 9 (R13 > 9)
CheckR139:
	ld	R13		; A <- R13
	daa			; decimal correction, set Carry = 1 if R13 > 9
	ret	1		; return with A = 1, C = 1 jump if R13 > 9

; ----- decrement digit R13 and jump if R13 was 0 (C = 1 if R13 was 0)
; $1AC JPC ZERO_DIGT + DEC DIGIT	decrement R13 and jump if R13 was 0
DecDigJmp0:
	ld	R13		; A <- R13
	dec	A		; decrement A (= R13 - 1), set C = 1 if R13 was not 0

; ----- clear digit R13 and jump (C = 1)
; $1AE CLR DIGIT + JMP	clear digit (R13 = 0) and jump
ClrDigJmp:
	xch	R13		; R13 <- A (= 0)
	cmc			; complement Carry, negate jump condition
	ret	1		; return with A = 1, C = 1 jump

; ----------------------------------------------------------------------------
;                  Macro command - check function code
; ----------------------------------------------------------------------------
; INPUT: R5 = function code
;	 R4 = function parameter
; ----------------------------------------------------------------------------

; ----- check new operation (set C = 1 if R5 < 8, new operation)
; $1B1 JPC NEWOP		jump at new operation (jump if function code < 8)
CheckNewOp:
	ldi	7		; A <- 7
	sub	R5		; A <- 7 - R5 (C = 0)
	ret	1		; return with A = 1, C = 1 jump if function code < 8

; ----- check memory operation (set C = 1 if R4 > 3, memory operation)
; $1B4 JPC MEMOP		jump at new memor peration (jump if function code > 3)
CheckMemOp:
	ldi	12		; A <- 12
	add	R4		; A <- 12 + R4 (C = 0), function parameter + 12, set Carry if R4 > 3
	ret	1		; return with A = 1, jump if R4 > 3

; ----- rotate function code R5 right and jump if bit0 was not 0
; $1B7 JPC ROTFC		rotate function code right and jump if out bit is zero
RotFC:	ld	R5		; A <- R5 function code
	rar			; rotate bit 0 into CY
	xch	R5		; R5 <- A, return rotated value
	cmc			; complement Carry
	ret	1		; return with A = 1, C = 1 jump if bit0 was != 0

; ----- jump if function parameter R4 is odd (jump if bit0 of F4 is 1)
; $1BC JPC ODDPAR		jump if function parameter is odd
OddPar:	ld	R4		; A <- R4 function parameter
	rar			; rotate bit 0 of R4 into CY
	ret	1		; return with A = 1, jump if bit 0 of function parameter = 1

; ----------------------------------------------------------------------------
;      Macro command - set decimal point from decimal point counter R11
; ----------------------------------------------------------------------------
; INPUT: R8 = destination register, default $10 = select WR
;	 R11 = decimal point counter
; ----------------------------------------------------------------------------

; ----- set decimal point IR.S1 from R11 (decimal point counter)
; IR = indirect register = function code R5 + 4
; $1BF SET DP_IR		set decimal point of IR (IR.S1 = R11)
SetDPIR:ldi	4		; A <- 4
	add	R5		; A <- 4 + R5 (C = 0) = function code + 4
	xch	R8		; R8 <- target address

; ----- set decimal point WR.S1 from R11 (decimal point counter)
; $1C2 SET DP_WR		set decimal point of WR (WR.S1 = R11)
SetDPWR:src	R89		; select register WR from R8R9
	ld	R11		; A <- R11, decimal point counter
	wr1			; write decimal point counter from A to S1
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;      Macro command - get decimal point counter R11 from decimal point 	
; ----------------------------------------------------------------------------
; INPUT: R8 = source register, default $10 = select WR
; OUTPUT: R11 = decimal point counter
; ----------------------------------------------------------------------------

; ----- get decimal point position WR.S1 to R11
; $1C6 GET DP_WR		get decimal point of WR (R11 = WR.S1)
GetDPWR:src	R89		; select register WR from R8R9
	rd1			; A <- read S1, position of decimal point
	xch	R11		; R11 <- set decimal point counter
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;          Macro command - increment decimal point counter R10R11
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R11 = decimal point counter
; ----------------------------------------------------------------------------

; ----- increment decimal point counter R10R11
; $1CA INC DPCNT		increment decimal point counter (increment R10R11)
IncDP:	ijnz	R11,IncDP_2	; increment decimal point counter R11 low, jump if not zero
	inc	R10		; increment high part of decimal point counter R10
IncDP_2:ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;             Macro command - check decimal point counter R11
; ----------------------------------------------------------------------------
; INPUT: A = 0 or 13, to check R11 <= A
; INPUT/OUTPUT: R10R11 = decimal point counter
;	 C = 1 if decimal point counter R10R11 <= 0 or 13
; ----------------------------------------------------------------------------

; ----- check decimal point counter R11 if it is <= 13
; $1CE JPC NBIG_DPCNT	jump if decimal point counter does not exceed limit (jump if R10R11 < 0E)
CheckDP13:
	ldi	13		; A <- 13, max. value

; ----- check decimal point counter R11 if it is = 0
; $1CF JPC ZERO_DPCNT	jump if decimal point counter is zero (jump if R10R11 = 0)
CheckDP0:
	sub	R11		; A <- 0,13 - R11 (C = 0), check decimal point counter, C = 1 
	cmc			; complement carry
	ldi	0		; A <- 0
	sub	R10		; A <- 0 - R10 - C (= compare R10R11)
	ret	1		; return with A = 1, C = 1 jump if decimal point counter <= 0 or 13

; ----------------------------------------------------------------------------
;             Macro command - Jump to functions in next segment
; ----------------------------------------------------------------------------

; $1D4 JPC DIFF_SIGN	jump if WR and IR have different sign
CheckSign:
	jmp	CheckSign_2
	nop

; $1D7 DIGIT		digit functions
DigStart:
	jmp	DigStart_2

; $1D9 MOV WR,TR + CLR TR + CLR SR	move WR <- TR, clar TR, clear SR
LoadRes:jmp	LoadRes_2

; $1DB SET MRMFUNC + JMP	set function code the memory function and jump
JmpMem:	jmp	JmpMem_2

; $1DD DEC DPCNT		decrement decimal point counter (decrement R10R11)
DecDP:	jmp	DecDP_2

; $1DF GET DPDIFF		difference of actual dedimal point (WR.S1 = WR.S3, R10R11 <- difference)
DiffDP:	jmp	DiffDP_2

; $1E1 GET DPCNTDIV		adjust decimal point counter for div (R10R11 <- DR.S1 + (13-R11) - WR.S1)
AdjDiv:	jmp	AdjDiv_2

; $1E3 GET DPCNTMUL		adjust decimal point counter for mul (R10R11 <- DR.S1 + R11 + WR.S1)
AdjMul:	jmp	AdjMul_2

; $1E5 SET DIVMUL_SIGN + MOV DIGIT,15	set sign of RR, set DIGIT to 15 (R13=15 loop counting)
SetDMSign:
	jmp	SetDMSign_2

; $1E7 NEG WR		negate WR (WR.S0 <- !WR.S0)
NegWR:	jmp	NegWR_2

; $1E9 ROUNDING		increment WR and R14 if DIGIT R13 > 4
RoundWR:jmp	RoundWR_2

; $1EB PRN ADVANCE + CLR DPCNT	advancing printer paper, clear decimal point counter
PaperAdv:
	jmp	PaperAdv_2

; $1ED SQRT			RR <- SQRT(WR)
Sqrt:	jmp	Sqrt_2

; ----------------------------------------------------------------------------
;                      Macro command - Clear functions
; ----------------------------------------------------------------------------
; INPUT: R6 = destination number register, default 2 = select DR
; ----------------------------------------------------------------------------

; ----- clear mode to no decimal point, no overflow
; $1EF CLR MENT + CLR OVFL + RET	return + clear (CR.S2=0, SR.S2=0, TR.S2=0)
; 1) R6 + 2 = 4 = CR, set flag "no decimal point"
; 2) R6 + 1 = 5 = SR, clear flag "overflow"
; 3) R6 + 1 = 6 = TR, cleared but not used
ClrOver:call	ClearCRS2		; R6 = R6 + 2, clear status digit 2 of NR(R6)

; ----- clear mode to new number, no decimal point
; $1F1 CLR MODE + CLR MENT + RET	return + clear (RR.S2=0, CR.S2=0)
; 1) R6 + 1 = 3 = RR, set flag "new number"
; 2) R6 + 1 = 4 = CR, set flag "no decimal point"
ClrModeNoDP:
	call	ClearRRS2		; R6 = R6 + 1, clear status RR.S2 = last operation is new number

; ----- clear mode to new number (set last operation = new number, reset decimal point counter, exit to main loop)
; $1F3 CLR MODE + RET	return + clear (RR.S2=0, CR.S2=0)
; 1) R6 + 1 = 3 = RR, set flag "new number"
ClrMode:call	ClearRRS2		; R6 = R6 + 1, clear status RR.S2 = last operation is new number
	fim	R1011,0			; R10R11 <- 0, reset decimal point counter
	jmp	Reset			; exit macro interpreter, jump to main loop

; ----------------------------------------------------------------------------
;                       Macro command - Print functions
; ----------------------------------------------------------------------------

; $1F9 PRN FPAR,C		print number with function parameter R4 in column 17 and character 'C' in column 18 (R15 will be = 6 -> 9)
PrintFParC:
	inc	R15

; $1FA PRN FPAR,MEM		print number with function parameter R4 in column 17 and character 'M' in column 18 (R15 will be = 5 -> 10)
PrintFParM:
	inc	R15

; $1FB PRN FPAR,EMPTY		print number with function parameter R4 in column 17 and empty column 18 (R15 will be = 4 -> 11)
PrintFParEmp:
	inc	R15

; $1FC PRN FPAR			print number with function parameter R4 in column 17 and empty column 18 (R15 will be = 3 -> 12)
PrintFPar:
	inc	R15

; $1FD PRN ROUND,FPAR		print number with rounding mark in column 17 and function parameter in column 18 (R15 will be = 2 -> 13)
PrintRoundFPar:
	inc	R15

; $1FE PRN FCODE		print number with function code in column 17 and empty column 18 (R15 will be = 1 -> 14)
PrintFCode:
	inc	R15

; $1FF PRN OVFL			print overflow (dots) and empty columns 17 and 18 (enter with R15 = 0 -> 15)
PrintOvfl:
	xch	R15		; A <-> R15
	cma			; complement accumulator, A <- 15 - R15
	xch	R15		; R15 <- complement R15

; ----- R15 = 15: overflow, print 15 characters '.'

	ijnz	R15,Print2	; check if R15 = 15, increment R15

	ldi	10		; A <- 10, code of decimal point on print drum at columns 1..15
	call	ClearWR		; set WR to digits '.'
	fim	R1415,$FF	; R14R15 <- $FF, last two columns 17 and 18 will be empty
	xch	R10		; R10 <- A = 10, position of decimal point HIGH
	ldi	15		; A <- 15
	xch	R9		; R9 <- 15, 14 valid characters, A <- 0 from R9
	src	R89		; select register WR from R8R9, select M15
	wrm			; write WR.M15 <- 0, column 16 is not used
	jmp	Print6		; start printing from register WR and R1415

; ----- R15 = 14: number with function code in column 17 and empty column 18

Print2:	ijnz	R15,Print3	; check if R15 = 14, increment R15

	ldi	15		; A <- 15
	xch	R15		; R15 <- 15, empty column 18
	ld	R5		; A <- function code
	jmp	Print5		; save A into R14

; ----- R15 = 13: number with rounding mark in column 17 and function parameter in column 18

Print3:	ldi	1		; A <- 1
	add	R15		; A <- 1 + R15
	tcc			; A <- C
	jz	Print4		; jump if R15 < 13

	ld	R4		; A <- R4 function parameter
	xch	R15		; R15 <- A, function parameter at column 18
	xch	R14		; A <- R14, rounding selection (0,1,8)
	rar			; get R14.bit0 -> C, "round" selected
	cmc			; complement C (= !R14.bit0)
	ldi	14		; A <- 14
	rar			; A <- 8*CY + 7 (7 = rounding, 15 = empty char if truncated)
	jmp	Print5		; save A character into R14, to print rounding mark at column 17

; ----- R15 < 13: print function parameter R4 in column 17 and character R15+2 in column 18

Print4:	ld	R4		; A <- R4, function parameter
Print5:	xch	R14		; R14 <- A, code of character in column 17

; ----- load position of decimal point -> R10, R11

	src	R89		; select address R8R9 = WR
	rd1			; A <- read status 1 (= position of decimal point)
	xch	R10		; R10 <- position of decimal point
	rd1			; A <- read status 1 (= position of decimal point)
	xch	R11		; R11 <- position of decimal point

; ----- wait for inactive printer drum sector signal
; Here is:
;  WR pattern of digits in columns 1..15, last digit 16 must be = 15 (not used column 16)
;  R10, R11 position of decimal point
;  R14 character in column 17
;  R15 character in column 18

Print6:	jt	Print6		; wait if TEST = 0 (wait if signal is active)

; ----- select ribbon color

	ldi	2		; A <- 2
	xch	R13		; R13 <- 2, sector loop counter (14 sectors to print)
	rd0			; read A <- WR.S0 (sign)
	rar			; rotate A right, sign flag -> C
	tcc			; A <- C, 0 if positive, 1 if negative
	wmp			; write A to RAM0 port, select color (bit 0): red if negative, black if positive

; ----- keyboard service
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: R89 = $00 (select KR)
;	  R67 = $10 (select WR)
;	  A = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Should be called strictly after sector signal becomes inactive (TEST goes -> 1).
; On finish waits for sector signal becomes active (TEST goes -> 0).

	call	Keyboard	; keyboard handling, count sector
	inc	R8		; restore R8R9 to point to WR again

; ----- count total number of digits (starting with R11 = decimal point position)

Print7:	inc	R11		; total digits counter
	ld	R11		; A <- R11, total digits counter
	xch	R9		; R9 <- total digits counter, start behind highest digit
	call	Check0WR	; check whether remainging part of WR contains any digit (= higher part before number)
	tcc			; A <- carry (1 = if number is zero)
	jz	Print7		; loop if some digits remain

; Here is:
;  R6 = 1 (select WR)
;  R7 = 0 (index of the digit)
;  R8 = 1 (select WR)
;  R9 = 0
;  R10 = place of decimal point
;  R11 = total number of digits
;  R12 = drum sector counter
;  R13 = 2 (printer sector loop counter)
;  R14 = character code on pre-last column
;  R15 = character code on last column

; ----- process printing, R13 = loop couter for printer sectors

Drum1:	jt	Drum1		; wait for inactive printer drum sector signal (wait if TEST = 0)
	clb			; clear A = 0 and Carry = 0
	wmp			; write RAM0 port, printer control signals are set to inactive
	wrr			; write ROM0 port, clear shifter signals
	ijnz	R13,Drum2	; jump to next sector if R13 not 0

; ----- end of printing, advance paper by 1 row

; EB PRN ADVANCE + CLR DPCNT	advancing printer paper, clear decimal point counter
; ----- advance printer paper by 1 row (= time of 4 drum sector signals)
; Here is RAM0 chip selected
PaperAdv_2:
	fim	R1011,$0C	; R10R11 <- $0C, R11 = 12 counter to send ADV signal (number of steps 16 - 12 = 4)
	fim	R1415,$00	; R14R15 <- $00
	ldi	8		; A <- 8, bit 3 = advance printer paper

PaperAdv_3:
	jt	PaperAdv_3	; wait for inactive printer drum sector signal (wait if TEST = 0)
	wmp			; write RAM0 port: first 8 advance printer paper, later 0 to clear signals
	call	Keyboard	; keyboard handling (and wait for active drum sector signal)
	ijnz	R11,PaperAdv_3	; loop, send ADV signal (4 loops)
	ret	0		; return with A = 0

; ----- synchronize printer drum sector
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: A = 0, C = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Called strictly after sector signal becomes inactive (TEST goes -> 1).
; Increments R12 (sector counter), or clears R12 if index signal is active.

Drum2:	call	DrumSync	; R12 synchronization with printer drum sector
	xch	R8		; R8 <- A = 0, reset digit pointer in WR

; ----- check if R15 = 13, print empty column

Drum3:	ldi	13		; A <- 13
	sub	R15		; A <- 13 - R15 (C=0), check if R15 = 13
	clc			; clear carry
	jnz	Drum4		; skip if R15 is not 13

; ----- print empty columns if R15 = 13

	xch	R10		; R10 <- 0, place of decimal point
Drum34:	ldi	15		; A <- 15, print no digit
	jmp	Drum5

; ----- load one digit from WR (in order from M0 to M14)

Drum4:	src	R67		; select register and digit WR from R6R7
	rdm			; A <- read one digit
Drum5:	ijnz	R7,Drum8	; jump to next digit

; ----- all digits from WR are done: skip column 16, add columns 17 and 18

	ld	R10		; A <- R10, position of decimal point
	jnz	Drum6		; decimal point is already shifted (so column 16 is alredy skipped)
	call	DrumShift2	; shift one inactive column into printer shifter - skip column 16

Drum6:	ld	R15		; A <- R15, character in column 18
	call	DrumShift	; if R15=R12, shift 1 into print shifter, or 0 otherwise
	ld	R14		; A <- R14, character in column 17
	call	DrumShift	; if R14=R12, shift 1 into print shifter, or 0 otherwise

; ----- wait for active sector signal

Drum7:	jnt	Drum7		; wait for active printer drum sector signal

; ----- fire print hammers
; INPUT/OUTPUT: R12 drum sector counter (incremented by 1, or cleared to 0)
; OUTPUT: R89 = $00 (select KR)
;	  R67 = $10 (select WR)
;	  A = 0
; DESTROYS: R2, R3
; ----------------------------------------------------------------------------
; Should be called strictly after sector signal becomes inactive (TEST goes -> 1).
; On finish waits for sector signal becomes active (TEST goes -> 0).

	ldi	2		; A <- 2, bit 1 = fire print hammer
	src	R89		; select RAM0 from R8R9
	wmp			; output port RAM0, fire print hammers
	call	Keyboard1	; keyboard handling (R7 is cleared)
	jmp	Drum1		; loop back to next sector

; ----- add next digit (if A = R12)

Drum8:	call	DrumShift	; if A = R12, shift 1 into printer shifter, or 0 otherwise

; ----- check decimal point

	ld	R10		; A <- R10
	jz	Drum9		; jump if R10 = 0 (decimal point is already sent)
	sub	R7		; A <- A - R7 (C = 0)
	clc			; clear Carry
	jnz	Drum9		; jump if R10 != R7 (decimal point is not in this position

; ----- insert decimal point (if current sector R12 = 10 decimal point)

	ldi	10		; A <- 10, decimal point code character
	call	DrumShift	; if R12=10, shift 1 into printer shifter, or 0 otherwise

; ----- check number of valid digits

Drum9:	ld	R7		; A <- R7, index of the digit
	sub	R11		; A <- R7 - R11
	tcc			; A <- Carry
	jz	Drum3		; loop back for next valid digits
	jmp	Drum34		; loop back for empty columns

; ----- Send bit to printer shifter (send 1 if A = R12 current sector, or 0 otherwise)

DrumShift:
	sub	R12		; A <- A - R12 - CY
	clc			; clear carry - hammer not fired
	jnz	DrumShift2	; skip if A != R12
	stc			; set carry - hammer will be fired
DrumShift2:
	ldi	1		; A <- 1
	ral			; A <- 2 + Carry
	ral			; A <- bit 2 is set (clock), bit 1 = Carry (data)
	jmp	SendShift	; send bit into printer shifter

; ----------------------------------------------------------------------------
;                  Macro command - start digit entry mode
; ----------------------------------------------------------------------------
; INPUT: R8 = source register, default $10 = select WR
; OUTPUT: R11 = decimal point counter
; ----------------------------------------------------------------------------

; ----- check if calculator is already in digit entry mode
; $1D7, $294 DIGIT		digit functions (00, 000, . or - pressed)
DigStart_2:
	ld	R4		; A <- R4, function parameter
	xch	R13		; R13 <- function parameter (digit)
	fim	R67,$40		; R6R7 <- $40, address of CR
	src	R67		; select CR register from R6R7
	rd2			; A <- read CR.S2, digit entry mode status
	jnz	DigStart_3	; jump if calculator is already in digit entry mode

; ----- start digit entry mode (without DP), clear registers

	ldi	8		; A <- 8, mode digit without decimal point
	wr2			; write 8 into CR.S2, flag of digit entry mode
	clb			; clear A and C
	call	ClearWR		; clear WR, WR.S0 and WR.S1
DigStart_3:
	jmp	GetDPWR		; R11 = WR.S1, get decimal point position

; ----------------------------------------------------------------------------
;        Macro command - load result (load WR from TR, clear SR and TR)
; ----------------------------------------------------------------------------
; INPUT: R6R7 = source number register (R7=0), default $20 = select DR
;	 R8R9 = destination register (R9=0), default $10 = select WR
; ----------------------------------------------------------------------------

; ----- load WR from TR, clear SR and TR
; $1D9, $2A3  MOV WR,TR + CLR TR + CLR SR	move WR <- TR, clar TR, clear SR
LoadRes_2:
	call	MovWRTR		; WR <- TR
	call	ClearSR		; clear SR (including S0 and S1)
	call	ClearDR		; clear TR (including S0 and S1)
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;          Macro command - set memory function code and jump
; ----------------------------------------------------------------------------
; OUTPUT: R5 = 3, A = 1, C = 1	  
; ----------------------------------------------------------------------------

; $1DB, $2AA SET MRMFUNC + JMP	set function code the memory function and jump
JmpMem_2:
	ldi	3		; A <- 3
	xch	R5		; R5 <- 3, set function code
	stc			; set carry, jump to macro
	ret	1		; return with A = 1, C = 1, jump to macro

; ----------------------------------------------------------------------------
;              Macro command - decrement decimal point counter R10R11
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R10R11 decimal point counter
; ----------------------------------------------------------------------------

; $1DD, $2AE DEC DPCNT		decrement decimal point counter (decrement R10R11)
DecDP_2:ldi	1		; A <- 1, to decrement
	xch	R3		; R3 <- 1, amount of decrement R10R11
	xch	R11		; A <- R11, current decimal point counter
	jmp	AdjDiv_3	; adjust R10R11

; ----------------------------------------------------------------------------
;    Macro command - get difference of required decimals and current decimals
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R10R11 difference of decimal point counter
;   difference = required decimal point switch - current decimal point in WR
; ----------------------------------------------------------------------------

; $1DF, $2B3 GET DPDIFF		difference of actual dedimal point (WR.S1 = WR.S3, R10R11 <- difference)
DiffDP_2:
	call	ReadDP		; read decimal places of DR (->R2) and WR (->R3)
	rd3			; A <- read decimal point switch WR.S3
	wr1			; write decimal places to WR.S1 from A
	jmp	AdjDiv_3	; adjust R10R11 = required DP - current DP

; ----------------------------------------------------------------------------
;           Macro command - adjust decimal point counter for div
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R10R11 difference of decimal point counter
; ----------------------------------------------------------------------------

; ----- adjust decimal point counter R10R11 for div
; R10R11 = DR.S1 (decimals of DR) + 13 (max. decimals) - R11 (current decimals) - WR.S1 (decimals of WR)
; $1E1, $2B9 GET DPCNTDIV	adjust decimal point counter for div (R10R11 <- DR.S1 + (13-R11) - WR.S1)
AdjDiv_2:
	call	ReadDP		; read decimal places of DR (->R2) and WR (->R3)
	ldi	13		; A <- 13, max. decimal places
	sub	R11		; A <- 13 - R11 (C=0), 13-current decimal places
	clc			; clear carry
	add	R2		; A <- 13 - R11 + R2 (add decimals of DR)
	xch	R10		; R10 <- 13 - R11 + R2
	tcc			; A <- C
	xch	R10		; R10 <- C (carry to high nibble), A <- 13 - R11 + R2

; ----- decrease decimal point counter R10R11 by R3
; here is A = current decimal point counter LOW, R3 = amount to decrease
AdjDiv_3:
	sub	R3		; A <- A - R3, decrease decimal point counter LOW
	xch	R11		; R11 <- new value LOW
	cmc			; complement carry
	xch	R10		; A <- decimal point counter HIGH
	sub	R9		; borrow is subtracted from upper half (here is R9=0)
	xch	R10		; R10 <- R10 - C
	clc			; clear carry
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;           Macro command - adjust decimal point counter for mul
; ----------------------------------------------------------------------------
; INPUT/OUTPUT: R10R11 difference of decimal point counter
; ----------------------------------------------------------------------------

; ----- adjust decimal point counter R10R11 for mul
; R10R11 = DR.S1 (decimals of DR) + R11 (current decimals) + WR.S1 (decimals of WR)
; $1E3, $2CA GET DPCNTMUL	adjust decimal point counter for mul (R10R11 <- DR.S1 + R11 + WR.S1)
AdjMul_2:
	call	ReadDP		; read decimal places of DR (->R2) and WR (->R3)
	ld	R3		; A <- R3, decimal places of WR
	add	R11		; A <- R3 + R11 (C=0), C<-0
	add	R2		; A <- R3 + R11 + R2 (C=0)
	xch	R11		; R11 <- R3 + R11 + R2
	tcc			; A <- C
	xch	R10		; R10 <- high nibble
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                   Macro command - compare sign WR and IR
; ----------------------------------------------------------------------------
; INPUT: R5 = command code (IR = R5+4)
;	 R8R9 = source register (R9=0), default $10 = select WR
; OUTPUT: A = 1, C = 1 if WR and IR have differente sign (= jump)
; ----------------------------------------------------------------------------

; ----- compare sign or WR and IR, jump if have different sign (C = 1)
; IR = indirect register = function code R5 + 4
; $1D4, $2D3 JPC DIFF_SIGN	jump if WR and IR have different sign
CheckSign_2:
	ldi	4		; A <- 4
	add	R5		; A <- 4 + R5, function code + 4
	xch	R6		; R6 <- function code + 4 = indirect register

; ----- compare sign of registers R6R7 (IR or DR) and R8R9 (WR)
CheckSign_3:
	src	R67		; select IR register from R6R7
	rd0			; A <- read sign of IR
	xch	R2		; R2 <- sign of IR
	src	R89		; select WR register from R8R9
	rd0			; A <- read sign of WR
	add	R2		; A <- sign IR + sign WR, bit 0 is 0 if both numbers have same sign
	rar			; rotate result bit 0 into C
	ret	1		; return with A = 1, C = 1 jump if numbers have different sign

; ----------------------------------------------------------------------------
;             Macro command - set sign of mul/div result
; ----------------------------------------------------------------------------
; INPUT: R6R7 = destination number register (R7=0), default $20 = select DR
;	 R8R9 = source register (R9=0), default $10 = select WR
; OUTPUT: A = 0
;	 R13 = 15
; ----------------------------------------------------------------------------

; $1E5, $2DE SET DIVMUL_SIGN + MOV DIGIT,15	set sign of RR, set DIGIT to 15 (R13=15 loop counting)
SetDMSign_2:
	call	CheckSign_3	; compare WR and DR sign
	tcc			; A <- CY, 1 if different sign
	inc	R6		; increment R6 to point to RR
	src	R67		; select RR from R6R7
	wr0			; set sign of RR

	ldi	15		; A <- 15
	xch	R13		; R13 <- 15, used as loop-end indicator
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                      Macro command - negate WR
; ----------------------------------------------------------------------------
; INPUT: R8R9 = source register (R9=0), default $10 = select WR
; OUTPUT: A = 0
; ----------------------------------------------------------------------------

; ----- negate WR - flip sign WR.S0
; $1E7, $2E7 NEG WR		negate WR (WR.S0 <- !WR.S0)
NegWR_2:src	R89		; select WR register from R8R9
	rd0			; read sign WR.S0
	cma			; complement accumulator
	wr0			; write new sign WR.S0
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                      Macro command - round WR
; ----------------------------------------------------------------------------
; INPUT: R8R9 = source register (R9=0), default $10 = select WR
;	 R13 = last digit
; OUTPUT: A = 0
; ----------------------------------------------------------------------------

; ----- increment WR and R14 if DIGIT R13 > 4
; $1E9, $2EC ROUNDING		increment WR and R14 if DIGIT R13 > 4
RoundWR_2:
	ldi	11		; A <- 11
	add	R13		; A <- R13 + 11 (C = 0), digit + 11, C <- 1 if R13 > 4
	jnc	RoundWR_3	; skip if R1R675 (CY=0), no rounding
	inc	R14		; increment R14, too

; ----- increment WR in rounding (increment only if C = 1)

RoundWR_3:
	ldi	0		; A <- 0
	src	R89		; select WR register from R8R9
	adm			; A <- A + mem + C
	daa			; decimal adjust
	wrm			; write new digit
	ijnz	R9,RoundWR_3	; increment R9 (digit pointer) and loop to next digit
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;             Macro command - read decimal places of DR and WR
; ----------------------------------------------------------------------------
; INPUT: R6R7 = destination number register (R7=0), default $20 = select DR
;	 R8R9 = source register (R9=0), default $10 = select WR
; OUTPUT: A = 0
;	 R2 = decimal places of DR
;	 R3 = decimal places of WR
; ----------------------------------------------------------------------------

; ----- read decimal places of DR (->R2) and WR (->R3)
; $2F9
ReadDP:	src	R67		; select DR from R6R7
	rd1			; read DR.S1, decimal places
	xch	R2		; R2 <- DR.S1 decimal places
	src	R89		; select WR from R8R9
	rd1			; read WR.S1, decimal places
	xch	R3		; R3 <- WR.S1 decimal plaves
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;                            Macro programs
; ----------------------------------------------------------------------------
; $300

; Address must be aligned to ROM page
.page	; address $300

; ----------------------------------------------------------------------------
;             Fetch next command code indexed by R0R1 into R2R3
; ----------------------------------------------------------------------------
; INPUT: R0R1 = current address at this page
; OUTPUT: R2R3 = command code
;	 A = 0
; ----------------------------------------------------------------------------

; ----- fetch next command code indexed by R0R1 into R2R3

CmdFetch:
	fin	R23		; load code R2R3 <- [PC:R0R1]
	ret	0		; return with A = 0

; ----------------------------------------------------------------------------
;   Fetch jump address as new pointer, and continue to macro interpreter
; ----------------------------------------------------------------------------
; INPUT: R0R1 = current address at this page
; OUTPUT: R0R1 = new address
;	  Jump to macro interpreter to continue
; ----------------------------------------------------------------------------

; ----- fetch jump address as new value of pointer

JmpFetch:
	fin	R01		; load code R0R1 <- [PC:R0R1]
	jmp	Exec		; jump to macro interpreter

; ----------------------------------------------------------------------------
;                        Macro program - SQRT
; ----------------------------------------------------------------------------
; $305

fn_sqrt:   = @Sqrt ;$ed    ;SQRT (+ JMP num_dpadj)			;square root of WR is placed into RR

; ----------------------------------------------------------------------------
;                        Macro program - mul/div
; ----------------------------------------------------------------------------
; function F5=1 div/mul, parameter F4=4 div, 3 mul
; $306

; ----- check if new number is entered
        
fn_muldiv:

	; check RR.S2 and jump if new number is entered
	= @JmpNewNum, @md_prn2 ;$6c $14 ;JPC MODENN,md_prn2 ;jump, if new number is entered
	; Read status RR.S2.bit0 - 'mul/div' last operation flag -> C (jump if last operation = mul/div)
	= @ReadRRS2, @md_prn1 ;$75 $0e ;JPC MODEMD,md_prn1 ;jump, if mul or div was the last operation
	; lat operation is add/sub - recall last total result
	= @LoadRes ;$d9 ;MOV WR,TR + CLR TR + CLR SR ;if add or sub was the last operation, then main total is recalled
	; print number
	= @PrintFPar ;$fc ;PRN FPAR
	; jump to exit
	= @Jump, @md_exitf ; $a7 $0f ;JMP md_exitf

; ----- last operation is mul/div

md_prn1:
	; print number
	= @PrintFParEmp ;$fb ;PRN FPAR,FCODE
md_exitf:
	; set mul/div status DR.S2 from function parameter F4
	= @SetMulPar ;$8d ;SET MOPPAR ;keep the operation (from the parameter) for the next round
md_exitc:
	; move DR <- WR
	= @MovDRWR ;$04 ;MOV DR,WR ;put the number into DR and CR
	; move CR <- WR
	= @MovCRWR ;$02 ;MOV CR,WR
	; set RR.S2 status (last operation) to 1 = mul/div
	= @SetMulDiv ;$87 ;SET MODEMD
	; clear mode to no decimal point, no overflow
	= @ClrOver ;$ef    ;CLR MENT + CLR OVFL + RET

; ----- new number entered

md_prn2:
	; print number
	= @PrintFPar ; $fc ;PRN FPAR
	; jump if div/mul is not specified (other operand is not entered yet)
	= @JmpNMul, @md_exitf ; $6d $0f ;JPC MOPN,md_exitf ;jump, if the other operand is not entered yet
	; jump if constant calculation
	= @CheckDivCR, @md_exitf ; $7b $0f ;JPC MOPCONST,md_exitf ;jump, at constant calculation (new number for calculation)
	; jump if operation is mul
	= @ReadDRS2, @mul_start ;$76 $46 ;JPC MOPMUL,mul_start ;jump, if previous operation is multiply

;----------------------------------------------------------------------------------------------------------------------------------
;dividing:	WR <- RR = DR / WR
;
;DR and WR is left adjusted into position WR.M14<>0 and DR.M14<>0, DR is decreased by WR till it becomes negative. WR is added back
;to DR for getting back the smallest non negative DR. The count, how many times it could be decreased gives the next digit of
;result, which is shifted into RR. DR is shifted left for doing the subfunction for the next digit. The same process is repeated
;14 times. Place of digit point of the result is calculated separately. Finally the result from RR is copied to WR.
;----------------------------------------------------------------------------------------------------------------------------------
; $31B
	; set DR.S2 status (mul/div status) from function parameter R4
	= @SetMulPar ;$8d ;SET MOPPAR ;divide is marked into MOP

; ----- prepare division (jump here from %)

div_chk0:
	; check if WR = 0 (divisor) and jump to overflow
	= @Check0WR, @num_overf ;$a2 $3c ;JPC ZERO_WR,num_overf ;divide by zero would result overflow
	; clear RR (result register)
	= @ClearRR ;$48 ;CLR RR
	; check if DR = 0 (dividend) and jump to zero result
	= @Check0DR, @num_res ;$a0 $73 ;CLR DIGIT + JPC ZERO_DR,num_res	;if dividend is zero, the result will be zero too
	; adjust decimal point counter for div (R10R11 <- DR.S1 (dividend) + (13-R11) - WR.S1 (divisor)
	= @AdjDiv ;$e1  ;GET DPCNTDIV ;digit point initialization for divide

; ----- check dividend DR - shift left if it is too small

div_chkDR: 
	; check if dividend DR is small - rotate DR to leftmost position
	= @CheckSmallDR, @div_lshDR ;$9e $32 ;CLR DIGIT + JPC NBIG_DR,div_lshDR ;rotate DR into leftmost position

; ----- check dividend WR - shift left if it is too small

div_chkWR: 
	; check if divisor WR is small - rotate WR to leftmost position
	= @CheckSmallWR, @div_lshWR ;$9a $36 ;JPC NBIG_WR,div_lshWR ;rotate WR into leftmost position

; ----- set sign of result and prepare mark of loop-end

	; set sign of mul/div result (RR.S0 <- DR.S0 ^ WR.S0), set R13 to 15 as mark of loop-end
	= @SetDMSign ;$e5  ;SET DIVMUL_SIGN + MOV DIGIT,15 ;sign of result is set
	; shift RR left 2x, shift 15 from R13 into M0 as mark loop-end
	= @ShiftL_RR ;$51    ;SHL RR ;15 is shifted into the cleared RR, as a mark for loop end
	= @ShiftL_RR ;$51    ;SHL RR

; ----- divide loop (on start R13 = 0, loop counter)
; - subtract DR-WR repeated until negative result, count R13 digit counter
; - add DR+WR back to previous value
; - shift digit R13 into result RR
; - check shifted mark R13 if end of loop
; - shift divider DR and repeat loop

div_loop:
	; subtract divisor from dividend, DR <- DR - WR, repeat if result is not negative, count R13
	= @SubDRWR, @div_loop ;$34 $29 ;SUB DR,WR + JPC NNEG,div_loop + INC DIGIT ;find, how many times the subtraction can be done
	; result is negative - add divisor back to dvidend, DR <- DR + WR
	= @AddDRWR ;$21  ;ADD DR,WR ;adding back the last unneeded subtract
	; shift result RR left, insert new digit R13 into M0
	= @ShiftL_RR ;$51  ;SHL RR ;next digit of result is shifted into RR
	; check shifted-out digit R13 - if it is 15 (>9), it is end-loop mark, so finish
	= @CheckR139, @div_finsh ;$a9 $3f ;JPC BIG_DIGIT,div_finsh ;if shifted out number>9, end of division
	; shift dividend DR left (here is R13 = 0)
	= @ShiftL_DR ;$52  ;SHL DR ;next digit (shifted out from RR) is shifted into DR
	; repeat divide loop
	= @Jump, @div_loop ;$a7 $29 ;JMP div_loop

; ------ rotate DR (dividend) to leftmost position

div_lshDR:
	; shift dividend DR one position left
	= @ShiftL_DR ;$52    ;SHL DR	;one digit rotate left of DR
	; increment decimal point counter R10R11
	= @IncDP ;$ca    ;INC DPCNT
	; repeat check DR
	= @Jump, @div_chkDR ;$a7 $22 ;JMP div_chkDR

; ------ rotate WR (divisor) to leftmost position

div_lshWR:
	; shift divisor WR one position left
	= @ShiftL ;$53    ;SHL WR ;one digit rotate left of WR
	; check decimal point counter R10R11 - if it is already = 0, it is overflow result
	= @CheckDP0, @num_overf ;$cf $3c ;JPC ZERO_DPCNT,num_overf ;jump if rotate would cause overflow
	; decrement decimal point counter R10R11
	= @DecDP ;$dd   ;DEC DPCNT
	; repeat check WR
	= @Jump, @div_chkWR ;$a7 $24 ;JMP div_chkWR

; ----- overflow (divide by 0)
; $33C
num_overf:
	; print overflow
	= @PrintOvfl ;$ff  ;PRN OVFL ;print overflow
	; set overflow flag
	= @SetOver ;$85    ;SET OVFL					;set overflow flag
	; clear mode and exit
	= @ClrModeNoDP ;$f1  ;CLR MODE + CLR MENT + RET ;exit

; ----- divide finish, adjust result to max. 13 decimal digits
; $33F
div_finsh:
	; rotate result RR one position right
	= @ShiftR_RR ;$5d    ;SHR RR	;rotate the number right
; $340
num_dpadj:
	; check decimal point counter R10R11 if it is <= 13 (jump if OK)
	= @CheckDP13, @num_res ;$ce $73 ;JPC NBIG_DPCNT,num_res	;jump, if the result contains acceptable number of digits
	; decrement decimal point counter
	= @DecDP ;$dd    ;DEC DPCNT	;otherwise shift the number to right
	; shift result right
	= @ShiftR_RR ;$5d    ;SHR RR	;Note: the place of this instruction could have been saved,
	; repeat adjust
	= @Jump, @num_dpadj ;$a7 $40 ;JMP num_dpadj ;  if the jump would go back to div_finsh

;----------------------------------------------------------------------------------------------------------------------------------
;multiplication: WR <- RR = DR * WR
;
;As starting WR is copied to RR and DR copied to WR. DR is cleared.
;DR and RR is shifted right. Last digit of RR is placed into R13, WR is added R13 times to DR. The process is repeated 14 times.
;Two 14 digit operand produces maximum 28 digit result. For us the most significant digits are interesting. Therefore the 28 digit
;result is rotated towards the lower digits, till the upper 14 digits contain nonzero digits, the place of digit point is counted
;in R10 and R11. After rotate the result is finally copied to WR.
;----------------------------------------------------------------------------------------------------------------------------------
; $346
mul_start:
	; set DR.S2 status (mul/div status) from function parameter R4
	= @SetMulPar ;$8d  ;SET MOPPAR  ;multiplication is marked in MOP

; ----- prepare multiplication (jump here from %)

mul_st2:
	; move RR <- WR
	= @MovRRWR ;$03  ;MOV RR,WR
	; adjust decimal point counter for mul (R10R11 <- DR.S1 + R11 + WR.S1)
	= @AdjMul ;$e3  ;GET DPCNTMUL	;digit point initialization for multiply
	; set sign of mul/div result (RR.S0 <- DR.S0 ^ WR.S0), set R13 to 15 as mark of loop-end
	= @SetDMSign ;$e5  ;SET DIVMUL_SIGN + MOV DIGIT,15  ;sign of result is set
	; move WR <- DR
	= @MovWRDR ;$0e  ;MOV WR,DR
	; clear DR
	= @ClearDR ;$49  ;CLR DR
	; shift DR one position left, insert R15=15 as mark of loop-end
	= @ShiftL_DR ;$52  ;SHL DR  ;shift R13=15 into DR, but it is immediately shifted into RR

; ----- multiplication loop
; - shift DR right
; - shift RR right (old WR), shift-out one digit R13 (15 = end of loop)
; - add WR to DR with count R13-times
mul_loopn:
	; shift DR one position right, to get next digit into R13
	= @ShiftR_DR ;$5e  ;SHR DR  ;DR-RR is shifted right
	; shift RR one position right, insert digit R13
	= @ShiftRShort_RR ;$5a  ;SSR RR
	; check shifted-out digit R13 - if it is 15 (>9), it is end-loop mark, so finish
	= @CheckR139, @mul_shres ;$a9 $56 ;JPC BIG_DIGIT,mul_shres ;jump if R13=15 was shifted out (exit from the loop)

; ----- add WR to DR with count R13 (=current digit)

mul_loopd:
	; decrement digit R13 and jump if R13 was 0 (C = 1 if R13 was 0) = end of loop
	= @DecDigJmp0, @mul_loopn ;$ac $4d ;JPC ZERO_DIGIT,mul_loopn + DEC DIGIT ;multiply the number with one digit
	; add WR to DR
	= @AddDRWR ;$21  ;ADD DR,WR  ;finally DR=DR+R13*WR
	; continue loop, to add WR to DR R13-times
	= @Jump, @mul_loopd ;$a7 $51 ;JMP mul_loopd

; ----- normalize result DR:RR to fit into RR

mul_shres:
	; check if DR is 0 - result is adjusted OK
	= @Check0DR, @num_dpadj ;$a0 $40 ;CLR DIGIT + JPC ZERO_DR,num_dpadj ;rotate nonzero digits from DR to RR
	; check decimal point counter R10R11 - if it is already = 0, it is overflow result
	= @CheckDP0, @num_overf ;$cf $3c ;JPC ZERO_DPCNT,num_overf ;jump if overflow occurred
	; shift result DR:RR right
	= @ShiftR_DR ;$5e    ;SHR DR	;DR-RR is shifted right
	= @ShiftRShort_RR ;$5a    ;SSR RR
	; decrease decimal point counter
	= @DecDP ;$dd    ;DEC DPCNT
	; continue loop
	= @Jump, @mul_shres ;$a7 $56 ;JMP mul_shres

; ----- mark mode with decimal point
; $35F
dp_mark:
	; set digit entry mode to 1 = digit with DP
	= @SetNumDP ;$86    ;SET MENTDP	;digit point flag
	; clear mode to new number
	= @ClrMode ;$f3    ;CLR MODE + RET

; ----------------------------------------------------------------------------
;                        Macro program - % percentage
; ----------------------------------------------------------------------------
; $361
fn_percnt:
	; print number
	= @PrintFCode ;$fe    ;PRN FCODE
	; increment decimal point counter R10R11 2-times (to shift result x 100)
	= @IncDP ;$ca    ;INC DPCNT	;increment the digit point place counter by 2
	= @IncDP ;$ca    ;INC DPCNT
	; continue
	= @Jump, @num_md ;$a7 $67 ;JMP num_md

num_prm:
	; print number
	= @PrintFCode ;$fe    ;PRN FCODE

; ----- check mul/div constant CR

num_md:
	; check CR.S2 status if mul/div constant CR
	= @CheckDivCR, @num_mul2 ;$7b $6f ;JPC MOPCONST,num_mul2  ;jump at const divide/multiply
	; set mul/div status constant CR
	= @SetDivCR ;$90    ;SET MOPCONST

; ----- jump to mul or div

num_mul1:
	; read status DR.S2.bit0 - 'mul started', jump to multiplication
	= @ReadDRS2, @mul_st2 ;$76 $47 ;JPC MOPMUL,mul_st2  ;jump to multiply, if previous operation is multiply
	; move CR <- WR (divisor for constant divide)
	= @MovCRWR ;$02    ;MOV CR,WR	;save the divisor for constant divide
	; jump to divide
	= @Jump, @div_chk0 ;$a7 $1c ;JMP div_chk0	;jump to divide

; ----- mul/div by constant CR, load CR into WR

num_mul2:
	; move DR <- WR
	= @MovDRWR ;$04    ;MOV DR,WR	;save the number into DR
	; move WR <- CR
	= @MovWRCR ;$0c    ;MOV WR,CR	;recall previous number from CR
	; jump to mul or div
	= @Jump, @num_mul1 ;$a7 $6a ;JMP num_mul1	;jump to divide or multiply

; ----- check operation

num_res:
	; move result WR <- RR
	= @MovWRRR ;$0d    ;MOV WR,RR	;copy the RR result to WR
	; set decimal point WR.S1 from R11 (decimal point counter)
	= @SetDPWR ;$c2    ;SET DP_WR	;set the digit point position from R10R11
	; check new operation (set C = 1 if R5 < 8, new operation)
	= @CheckNewOp, @md_exitc ;$b1 $10 ;JPC NEWOP,md_exitc	;jump to exit at new mul and div operation
	; check memory operation
	= @CheckMemOp, @num_adj ;$b4 $7b ;JPC MEMOP,num_adj	;jump to adjust at M=+/M=-
	; check WR.S2 and jump if not truncated/rounded
	= @JmpNTrunc, @num_pra2 ;$6e $9e ;JPC NTRUNC,num_pra2	;jump to result print, if digit point should not be adjusted

; ----- check difference of required decimals and current decimals
; $37B
num_adj:   
	; get difference of decimal position
	= @DiffDP ;$df    ;GET DPDIFF	;WR.S1=WR.S3, set R10R11 to the difference between required an actual digit point
					;Rotate the number into the required digit point place

; ----- check to normalize result to required number of decimal digits

num_rotl:
	; check decimal point counter R11 if it is alreaday 0
	= @CheckDP0, @num_pra1 ;$cf $9a ;JPC ZERO_DPCNT,num_pra1  ;jump, if number is at the right digit point place
	; check decimal point counter R11 if it is <= 13 - go to shift left
	= @CheckDP13, @num_lrot ;$ce $84 ;JPC NBIG_DPCNT,num_lrot
	; increment decimal point counter R10R11
	= @IncDP ;$ca    ;INC DPCNT	;Rotate right
	; shift WR right
	= @ShiftR ;$5f    ;SHR WR
	; jump to next test
	= @Jump, @num_rotl ;$a7 $7c ;JMP num_rotl

; ----- shift number left

num_lrot:
	; decrement decimal point counter R10R11
	= @DecDP ;$dd    ;DEC DPCNT	;Rotate left
	; shift WR left
	= @ShiftL ;$53    ;SHL WR
	; check if WR is small, next shift if is it small enough
	= @CheckSmallWR, @num_rotl ;$9a $7c ;JPC NBIG_WR,num_rotl
	; print overflow
	= @Jump, @num_overf ;$a7 $3c ;JMP num_overf	;print overflow

; ----------------------------------------------------------------------------
;                        Macro program - =, M=+, M=-
; ----------------------------------------------------------------------------
; $38A
; ----- calculate result and jump M+ M-

fn_memeq:
	; check RR.S2 and jump if new number
	= @JmpNewNum, @num_prm ;$6c $66 ;JPC MODENN,num_prm	;jump, if new number is entered
	; Read status RR.S2.bit0 - jump if 'mul/div' last operation flag -> C
	= @ReadRRS2, @num_prm ;$75 $66 ;JPC MODEMD,num_prm	;jump, if there is started mul/div operation
	; load WR from TR, clear SR and TR
	= @LoadRes ;$d9    ;MOV WR,TR + CLR TR + CLR SR	  ;recall main total
	; jump to M+ M-
	= @Jump, @fn_memadd ;$a7 $98 ;JMP fn_memadd	;jump to add functions

; ----------------------------------------------------------------------------
;                         Macro program - +/- add, sub
; ----------------------------------------------------------------------------
; function F5=2 add/sub, parameter F4=2 sub, 1 add
; $391
fn_addsub:
	; check RR.S2 and jump if new number to M+ M-
	= @JmpNewNum, @fn_memadd ;$6c $98 ;JPC MODENN,fn_memadd	 ;jump, if new number is enterer
	; Read status RR.S2.bit0 - jump if 'mul/div' last operation flag -> C, clear div/mul status
	= @ReadRRS2, @clr_md ;$75 $97 ;JPC MODEMD,clr_md  ;jump, if there is started mul/div operation
	; jump to M+ M-
	= @Jump, @fn_memadd ;$a7 $98 ;JMP fn_memadd ;jump to add functions

clr_md:
	; clear status DR.S2 (div/mul status)
	= @ClearDRS2 ;$82    ;CLR MOP	;ignore previous mul/div operation

; ----------------------------------------------------------------------------
;                        Macro program - M+/M-
; ----------------------------------------------------------------------------
; $398
fn_memadd:
	; clear digit R13 and jump to adjust
	= @ClrDigJmp @num_adj ;$ae $7b ;CLR DIGIT + JMP num_adj	;jump to adjust the number to the required digits

num_pra1:
	; check new operation, jump at new add/sub operation (set C = 1 if R5 < 8, new operation) - do operation +-
	= @CheckNewOp, @num_pra3 ;$b1 $aa ;JPC NEWOP,num_pra3	;jump at new add/sub operation
	; Read status WR.S2.bit0 - rounding switch "1=round" -> C jump
	= @ReadWRS2, @num_round ;$77 $a3 ;JPC ROUND,num_round	;jump to rounding, if rounding switch is in that position

num_pra2:
	; print number with round
	= @PrintRoundFPar ;$fd    ;PRN ROUND,FPAR
	; advancing paper
	= @PaperAdv ;$eb    ;PRN ADVANCE + CLR DPCNT
	; check if it is memory operation
	= @CheckMemOp, @mem_add ;$b4 $a8 ;JPC MEMOP,mem_add	;jump to change the function code at M=+/M=-/M+/M-
	; clear mode to new number, no decimal point
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

; ----- round WR and check overflow
; $3A3
num_round:
	; round WR (by last digit R13)
	= @RoundWR ;$e9    ;ROUNDING	;do the rounding based on the last shifted out digit in R13
	; check if number WR is small enough
	= @CheckSmallWR, @num_pra2 ;$9a $9e ;JPC NBIG_WR,num_pra2  ;may generate overflow too
	; print overflow
	= @Jump, @num_overf ;$a7 $3c ;JMP num_overf	;print overflow

; ----- memory operation
; $3A8
mem_add:
	; do add/sub memory operation
	= @JmpMem, @do_prpadd ;$db $ab ;SET MEMFUNC + JMP do_prpadd	;Set M+/M- function code

;----------------------------------------------------------------------------------------------------------------------------------
;add/subtract functions:
;
;By this point, numbers are shifted into the place determined by the digit point switch, thus no shifting is needed.
;
;!!! Note, if the digit point switch is changed during an operation, the numbers are incorrectly added/subtracted.
;
;	function code	    parameter		pre1		operation1	pre2	operation2
;+		2		1		RR=WR		TR=TR+WR	WR=RR	SR=SR+WR
;-		2		2		RR=WR		TR=TR-WR	WR=RR	SR=SR-WR
;M+ (M=+)	3		5		RR=WR		MR=MR+WR
;M- (M=-)	3		6		RR=WR		MR=MR-WR
;----------------------------------------------------------------------------------------------------------------------------------
; $3AA
num_pra3:
	; print number
	= @PrintFPar ;$fc    ;PRN FPAR

do_prpadd:
	; get decimal point counter R11 from decimal point WR
	= @GetDPWR ;$c6    ;GET DP_WR

; ----- change subtraction to addition (negate second operand)
; $3AC
do_addsub:
	; move RR <- WR, save original second operand
	= @MovRRWR ;$03    ;MOV RR,WR
	; jump if function parameter R4 is odd (jump if bit0 of F4 is 1) - add + M+
	= @OddPar, @skp_neg ;$bc $b0 ;JPC ODDPAR,skp_neg   ;skip negate the number at add
	; negate WR
	= @NegWR ;$e7    ;NEG WR	;negate the number at sub (convert it to add)

; ----- signs are identical, add second operand to first operand

skp_neg:
	; jump if WR and IR have different sign
	= @CheckSign, @do_sub ;$d4 $b7 ;JPC DIFF_SIGN,do_sub	;jump, when adding a negative and a positive number
	; add numbers IR = IR + WR, destination is TR for +-, MR for M+-
	= @AddIRWR ;$1e    ;ADD IR,WR	;ADD - may generate overflow
	; check overflow, jump if no overflow
	= @CheckSmallIR, @do_next ;$97 $bd ;JPC NBIG_IR,do_next, ;jump, if there is no overflow
	; roll IR back IR = IR - WR, and repotr overflow
	= @SubIRWR, @num_overf ;$31 $3c ;SUB IR,WR + JPC NNEG,num_overf + INC DIGIT	;correct back IR at overflow and jump always

; ----- signs are different, subtract second operand from first operand

do_sub:
	; subtract numbers IR = IR - WR, destination is TR for +-, MR for M+-
	= @SubIRWR, @do_next ;$31 $bd ;SUB IR,WR + JPC NNEG,do_next + INC DIGIT	;SUB - never generates overflow
	; underflow, roll destination register back, IR = IR + WR
	= @AddIRWR ;$1e    ;ADD IR,WR
	; subtract numbers in reverse oder, WR = WR - IR
	= @SubWRIR, @do_cont ;$2c $bc ;SUB WR,IR + JPC NNEG,do_cont	;always goes to the next instruction
do_cont:
	; move result WR to IR
	= @MovIRWR ;$01    ;MOV IR,WR

; ----- correct result, continue with next operation

do_next:
	; restore original second operand
	= @MovWRRR ;$0d    ;MOV WR,RR	;take the original number from RR
	; set decimal point of result from decimal point counter R10R11
	= @SetDPIR ;$bf    ;SET DP_IR	;set the place of digit point
	; check memory operation, exit if so
	= @CheckMemOp, @do_exit ;$b4 $ff ;JPC MEMOP,do_exit	;exit at memory function
	; rotate function code R5, check bit 0
	= @RotFC, @do_addsub ;$b7 $ac ;JPC ROTFC,do_addsub	;do the addsub for the next number, if there is instruction for it
	; mark last operation add/sub
	= @SetAddSub ;$8a    ;SET MODEAS  ;mark, that last operation was add or sub
	; clear mode to no decimal point, no overflow
	= @ClrOver ;$ef    ;CLR MENT + CLR OVFL + RET	;exit

; ----------------------------------------------------------------------------
;                          Macro program - C
; ----------------------------------------------------------------------------
; "C" Clear:	clear WR,DR,SR,TR and print. it does not clear RR,CR and RR.S2
; $3C5

fn_clear:
	; clear status DR.S2 (div/mul status)
	= @ClearDRS2 ;$82    ;CLR MOP
	; clear DR divide register
	= @ClearDR ;$49    ;CLR DR
	; load total result and cear TR,SR
	= @LoadRes ;$d9    ;MOV WR,TR + CLR TR + CLR SR
	; clear WR
	= @ClearWR ;$4a    ;CLR WR
	; print number
	= @PrintFPar ;$fc    ;PRN FPAR

; ----------------------------------------------------------------------------
;                          Macro program - CE
; ----------------------------------------------------------------------------
;"CE" Clear:	clear WR, RR.S2, CR.S2
; $3CA

fn_cleare:
	; clear WR
	= @ClearWR ;$4a    ;CLR WR
	; clear overflow flag
	= @ClearSRS2 ;$7f    ;CLR OVFL
	; clear mode to new number, no decimal point, exit
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

; ----------------------------------------------------------------------------
;                Macro program - Diamond (print sub total)
; ----------------------------------------------------------------------------
; "Diamond" - subtotal: print the number or the subtotal
; $3CD

fn_diamnd:
	; check RR.S2 and jump if new number
	= @JmpNewNum, @dm_prn2 ;$6c $d5 ;JPC MODENN,dm_prn2	;jump in entry mode, print the number, and close the entry mode
	; Read status RR.S2.bit0 - jump if 'mul/div' last operation
	= @ReadRRS2, @dm_prn1 ;$75 $d3 ;JPC MODEMD,dm_prn1	;jump in mul/div mode, print the number, and init
	; move WR <- SR
	= @MovWRSR ;$0b    ;MOV WR,SR	;in add/sub mode, recall the subtotal number from SR and clear SR
	; clear register SR
	= @ClearSR ;$46    ;CLR SR

; ----- last operation is mul/div

dm_prn1:
	; print number
	= @PrintFPar ;$fc    ;PRN FPAR
	; clear mode to no decimal point, no overflow, exit
	= @ClrOver ;$ef    ;CLR MENT + CLR OVFL + RET

; ----- new number

dm_prn2:
	; print number with round
	= @PrintRoundFPar ;$fd    ;PRN ROUND,FPAR
	; clear mode to new number, no decimal point, exit
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

; ----------------------------------------------------------------------------
;           Macro program - Digit, sign, decimal point, 00, 000
; ----------------------------------------------------------------------------
; $3D7               
;entry address at digit, digit number, minus sign button
;		fuction code		parameter
;0..9		13			0..9
;sign		13			10
;digit point	13			11
;00		6			0
;000		12			0

fn_digit:
	; start digit entry mode (save function parameter R4 into R13 = new digit, load decimal point counter R11 from WR.S1)
	= @DigStart ;$d7  ;DIGIT ;save digit into R13, place of digit point (WR.S1) into R11
				;at first entry: WR=0, CR.S2=8
	; check if digit R13 > 9 (= jump at decimal point or minus sign)
	= @CheckR139, @dig_dpsgn ;$a9 $df ;JPC BIG_DIGIT,dig_dpsgn ;jump at digit point, minus sign

dig_numsh:
	; shift WR left and insert new digit int M0
	= @ShiftL ;$53    ;SHL WR	;rotate the number into WR
	; check overflow of WR - jump if number is small
	= @CheckSmallWR, @dig_chkdp ;$9a $e3 ;JPC NBIG_WR,dig_chkdp  ; jump, if there is now overflow
	; at overflow rotate number back, additional digits are lost
	= @ShiftR ;$5f    ;SHR WR	;at overflow, rotate back the number (additional digits are lost)
	; clear mode to new number, and exit
	= @ClrMode ;$f3    ;CLR MODE + RET	;mark that new number is entered since the last operation, and exit

; ------ entered decimal point or minus sign

dig_dpsgn:
	; check odd parameter (= decimal point), mark mode with decimal point
	= @OddPar, @dp_mark ;$bc $5f ;JPC ODDPAR,dp_mark  ;digit point button is pressed
	; sign button - negate WR
	= @NegWR ;$e7    ;NEG WR	;minus sign button is pressed
	; clear mode to new number, and exit
	= @ClrMode ;$f3    ;CLR MODE + RET	;mark that new number is entered since the last operation, and exit

; ----- check decimal point

dig_chkdp:
	; read status CR.S2.bit0 - decimal point is already entered, adjust it
	= @ReadCRS2, @dig_incdp ;$74 $e8 ;JPC MENTDP,dig_incdp	;if digit point is already entered, jump to adjust it
	; continue to next digit
	= @Jump, @dig_nextd ;$a7 $ee ;JMP dig_nextd

= $00    ;(unimplemented, never used)

; ----- adjust decimal point

dig_incdp:
	; increment decimal point counter R10R11
	= @IncDP ;$ca    ;INC DPCNT	;adjust the digit point place with one digit more
	; check decimal point counter R10R11, continue ok if R10R11 <= 13
	= @CheckDP13, @dig_savdp ;$ce $ed ;JPC NBIG_DPCNT,dig_savdp
	; already too much digits, decrement decimal pouint counter back
	= @DecDP ;$dd    ;DEC DPCNT	;if already too much digit entered after the digit point,
	; shift WR right, additional digits are lost
	= @ShiftR ;$5f    ;SHR WR	; ignore the new digit
dig_savdp:
	; save new position of decimal point
	= @SetDPWR ;$c2    ;SET DP_WR	;save the place of digit point

; ------ continue with next digit (number of lower '0' bits if function code R5 = number of digits 0 to enter)

dig_nextd:
	; rotate function code R5 right and repeat if bit0 was not 0 (implementation of button '00' and '000' is here)
	= @RotFC, @dig_numsh ;$b7 $da ;JPC ROTFC,dig_numsh  ;function code contains, how many '0's has to be entered yet
	; clear mode to new number, and exit
	= @ClrMode ;$f3    ;CLR MODE + RET   ;mark that new number is entered since the last operation, and exit

; ----------------------------------------------------------------------------
;              Macro program - Ex (exchange operands DR and WR)
; ----------------------------------------------------------------------------
; Exchange function: CR,DR <- WR and WR,RR <- DR
; $3F1

fn_ex:
	; print number with rounding char
	= @PrintRoundFPar ;$fd    ;PRN ROUND,FPAR
	; move CR <- WR
	= @MovCRWR ;$02    ;MOV CR,WR	;CR=WR (WR is saved to CR)
	; move WR <- DR
	= @MovWRDR ;$0e    ;MOV WR,DR
	; move RR <- WR
	= @MovRRWR ;$03    ;MOV RR,WR	;RR=DR
	; move WR <- CR
	= @MovWRCR ;$0c    ;MOV WR,CR
	; move DR <- WR
	= @MovDRWR ;$04    ;MOV DR,WR	;DR=saved WR
	; move WR <- RR
	= @MovWRRR ;$0d    ;MOV WR,RR	;WR=RR
	; clear mode to new number, no decimal point
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

; ----------------------------------------------------------------------------
;                      Macro program - CM (print and clear MR)
; ----------------------------------------------------------------------------
; Clear memory:	recall (WR=MR), print and clear (R7=0)
; $3F9

fn_clrmem:
	; load memory register WR <- MR
	= @MovWRMR ;$09    ;MOV WR,MR
	; print memory register
	= @PrintFParM ;$fa    ;PRN FPAR,MEM
	; clear memory register MR
	= @ClearMR ;$44    ;CLR MR
	; clear mode to new number, no decimal point
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

; ----------------------------------------------------------------------------
;                   Macro program - RM (recall and print MR)
; ----------------------------------------------------------------------------
; Recall memory: recall (WR=MR) and print
; $3FD

fn_rm:
	; load memory register WR <- MR
	= @MovWRMR ;$09    ;MOV WR,MR
	; print memory register
	= @PrintFParM ;$fa    ;PRN FPAR,MEM
do_exit:
	; clear mode to new number, no decimal point
	= @ClrModeNoDP ;$f1    ;CLR MODE + CLR MENT + RET

;----------------------------------------------------------------------------------------------------------------------------------
; Optional program for making the SQRT function
;----------------------------------------------------------------------------------------------------------------------------------
; $400

; aligned to ROM page $400
.page

; ------ set macro code pointer to Sqrt
; ED SQRT			RR <- SQRT(WR)
Sqrt_2:	fim R01 @sq_start ;$28		; pseudo code entry address of the SQRT function
;
;Similar pseudo code interpreter implementation, like at $04b-05f, just uses the pseudo instruction codes from address range $400-$4ff
;

$402: jt $406		;wait for the inactive printer drum sector signal
          call Keyboard		;keyboard handling
$406: fim R67 $20
          fim R89 $10
          fin R23		;fetch pseudo instruction code into R2R3
          clb
          call SqrtCode		;execute the associated routine
$40e: ijnz R1 $411		;inc R0R1, pseudo code instruction pointer
          inc R0
$411: jz $402		;jump back, if ACC returned by the pseudo instruction was 0
          tcc
          jz $40e		;if CY returned by the pseudo instruction was 0, R0R1 is incremented again
          fin R01		;if CY was set to 1, read the pseudo code jump address
          jmp $402		;jump to continue the pseudo code from the modified address

; unused space
= 0, 0
= 0, 0
= 0, 0
= 0, 0
= 0, 0
= 0, 0
= 0, 0
= 0

;----------------------------------------------------------------------------------------------------------------------------------
;                   Macro program - Square root pseudo code implementation
;----------------------------------------------------------------------------------------------------------------------------------
; *= 0x428

; ----- prepare registers

sq_start:
	; print number with function code (= SQRT) and empty column
	= @Sqrt_PrintFCode ;$51    ;PRN FCODE		;print number with function code (9: SQRT)
	; move CR <- WR, number to compute
	= @Sqrt_MovCRWR ;$a7    ;MOV CR,WR		;save the number to the constant register
	; clear result register RR <- 0
	= @Sqrt_ClearRR ;$53    ;CLR RR			;clear result register
	; if number WR is zero, exit (all done)
	= @Sqrt_Check0WR, @sq_exit ;$61 $3e ;JPC ZERO_WR,sq_exit	;jump, if number is zero (the result will be also zero)
	; clear digit R13 and load decimal point position WR.S1 into R11
	= @Sqrt_ClrDigGetDP ;$65    ;CLR DIGIT + GET DP_WR 	;R10R11=place of digit point

; ----- left shift WR if it is small

sq_bshift:
	; check if WR is small. left shift if so
	= @Sqrt_CheckSmallWR, @sq_lshift ;$63 $44 ;JPC NBIG_WR,sq_lshift  ;number is adjusted to the leftmost position

	; shift WR one position back to right
	= @Sqrt_ShiftR ;$9c    ;SHR WR		;one digit overshift is corrected back
	; move DR <- WR (prepare remainder register)
	= @Sqrt_MovDRWR ;$5b    ;MOV DR,WR	;remainder (DR) is initialized to the shifted number
	; clear WR
	= @Sqrt_ClearWR ;$55    ;CLR WR		;initial subtrahend (WR) is cleared
	; calculate one SQRT digit
	= @Sqrt_Digit, @sq_loopns ;$6a $36 ;SET LPCSQRT + SET DPCNTSQRT + JPC EVENDP,sq_loopns	;R15=13, sqrt digit point calculation
						;jump if original digit point position was even

; ----- SQRT main loop

sq_loopsh:
	; decimal point position was odd, shift DR left
	= @Sqrt_ShiftL_DR ;$58    ;SHL DR		;multiplication by 10 of the remaining part
							;(and possible additional shift if it is needed)
; ----- count one digit

sq_loopns:
	; increment WR from position R15
	= @Sqrt_IncWRPos ;$7a    ;INC WR_POS		;increment the subtrahend (WR from position in R15) by 1
	; subtract DR <- DR - WR, jump if non-negative with increment digit R13
	= @Sqrt_SubDRWR, @sq_rptinc ;$5d $41 ;SUB DR,WR + JPC NNEG,sq_rptinc + INC DIGIT;remainder is decremented by the subtrahend (DR=DR-WR)
							;and jump, if the result is not negative
							;digit counter (R13) is incremented too

; ----- roll-back if negative reached

	; add numbers back DR <- DR + WR
	= @Sqrt_AddDRWR ;$5f    ;ADD DR,WR		;add the subtrahend to get back the last non negative value
	; decrement register WR from position R15
	= @Sqrt_DecWRPos ;$85    ;DEC WR_POS		;decrement the subtrahend by one (prepare it for the next round)

; ----- left shift RR and go next loop

	; left shift register RR
	= @Sqrt_ShiftL_RR ;$57    ;SHL RR		;shift the new digit into the number, R13 is cleared too
	; decrement SQRT loop counter R15 and jump to sq_loopsh if was not 0
	= @Sqrt_Loop, @sq_loopsh ;$98 $35 ;JPC NZERO_LPCSQRT,sq_loopsh + DEC LPCSQRT	;decrement R15, and jump, except when R15 becomes 0

; ----- exit SQRT macro
							;(next round calculates with one more digit)
sq_exit:
	; move DR <- WR (or WR <- CR ?)
	= @Sqrt_MovDRWR2 ;$a9    ;MOV DR,WR	(MOV WR,CR ???)	  ;??? subtrahend is saved (originally it may be WR=CR)
	; move DR <- WR
	= @Sqrt_MovDRWR ;$5b    ;MOV DR,WR		;??? duplicated, but not disturbing code
	; return back from SQRT macro, jump to num_dpadj
	= @Sqrt_Ret ;$9f    ;CLR MOP + RET_BPC		;return back to basic pseudo code interpreter to address $40

; ----- increment digit if result is not negative

sq_rptinc:
	; increment register WR from position R15
	= @Sqrt_IncWRPos ;$7a    ;INC WR_POS		;increment the subtrahend by 1 (WR from position in R15)
	; unconditional jump to sq_loopns
	= @Sqrt_Jump, @sq_loopns ;$96 $36 ;JMP sq_loopns	;jump back

; ----- left shift WR if it is small

sq_lshift:
	; shift WR left (unsert R13 digit to M0)
	= @Sqrt_ShiftL ;$59    ;SHL WR		;rotate number into left position
	; increment decimal point counter R10R11 and jump back
	= @Sqrt_IncDPJmp, @sq_bshift ;$93 $2e ;INC DPCNT + JMP sq_bshift  ;increment R10R11, and jump back

; unused space
= 0, 0
= 0, 0
= 0, 0
= 0, 0
= 0

;----------------------------------------------------------------------------------------------------------------------------------
;                         Macro commands for Sqrt
;----------------------------------------------------------------------------------------------------------------------------------
; *= 0x450

; ----- execute SQRT macro code - jump to address PC:R2R3
; $450
SqrtCode:
	jin	R23		;jump to the pseudo instruction code associated routine

; ----- print number with function code and empty column
; $451, $1FE PRN FCODE		print number with function code in column 17 and empty column 18 (R15 will be = 1 -> 14)
Sqrt_PrintFCode:
	jmp	PrintFCode	;PRN FCODE

; ----- clear register R8R9+2 (RR)
; $453, $148 CLR RR		clear RR
Sqrt_ClearRR:
	jmp	ClearRR		;CLR RR

; ----- clear register R8R9 (WR)
; $455, $14A CLR WR		clear WR
Sqrt_ClearWR:
	jmp	ClearWR		;CLR WR

; ----- left shift register R8R9+2 (RR)
; $457, $151 SHL RR		left shift RR with R13
Sqrt_ShiftL_RR:
	inc	R8		;SHL RR

; ----- left shift register R8R9+1 (DR)
; $458, $152 SHL DR		left shift DR with R13
Sqrt_ShiftL_DR:
	inc	R8		;SHL DR

; ----- left shift register R8R9 (WR)
; $459, $153 SHL WR		left shift WR with R13
Sqrt_ShiftL:
	jmp	ShiftL		;SHL WR

; ----- copy number from R8R9 (=WR) to R6R7 (=DR), with S0 and S1
; $45B, $104 MOV DR,WR		move DR <- WR
Sqrt_MovDRWR:
	jmp	MovDRWR		;MOV DR,WR

; ----- subtract two numbers R6R7 (DR) <- R6R7 (DR) - R8R9 (WR), jump if non-negative with increment digit R13
; $45D, $134 SUB DR,WR + JPC NNEG + INC DIGIT		subtract DR <- DR - WR, jump at non-negative with increment the digit
Sqrt_SubDRWR:
	jmp	SubDRWR		;SUB DR,WR + JPC NNEG + INC DIGIT

; ----- add numbers R6R7 (DR) <- R6R7 (DR) + R8R9 (WR)
; $45F, $121 ADD DR,WR		add DR <- DR + WR
Sqrt_AddDRWR:
	jmp	AddDRWR		;ADD DR,WR

; ----- Check if number R8R9 (WR) is zero (returns C = 1 if zero)
; $461, $1A2 JPC ZERO_WR		jump if WR = 0
Sqrt_Check0WR:
	jmp	Check0WR	;JPC ZERO_WR

; ----- Check if highest 2 digits of number R8R9 (WR) are zero (returns C = 1 if number is small)
; $463, $19A JPC NBIG_WR		jump if WR not big (upper 2 digits are 0)
Sqrt_CheckSmallWR:
	jmp	CheckSmallWR	;JPC NBIG_WR

; ----- clear digit R13 and load decimal point position WR.S1 into R11
; $465, QPC_65:	CLR DIGIT + GET DP_WR
Sqrt_ClrDigGetDP:
	xch	R13		; R13 <- A, clear digit (R13=0)
	src	R89		; select WR register from R8R9 (= $10)
	rd1			; read status WR.S1 = decimal point position
	xch	R11		; R11 <- WR.S1, decimal point position
	ret	0		; return with A = 0

; ----- calculate one SQRT digit
;QPC_6A:	SET LPCSQRT + SET DPCNTSQRT + JPC EVENDP
;		R15=13, R10R11=(R10R11/2+6+((R10R11 mod 2))), jump, if original R10R11 was even
Sqrt_Digit:
	fim	R1415,$6d	; R14 <- 6, R15 <- 13
	ld	R11		; A <- R11, decimal point position
	xch	R7		; R7 <- R11 (save original R11 into R7)
	xch	R10		; A <-> R10  (R10=0 [previous R7])
	rar			; shift A right, C <- R10.bit0, check if decimal point position is even
	ld	R11		; A <- R11, decimal point position
	rar			; A <- 8*(R10.bit0) + (R11 div 2),  C=(R11 mod 2)
	add	R14		; A <- 8*(R10.bit0) + (R11 div 2) + (R11 mod 2) + 6, C=overflow
	xch	R11		; store it to R11
	tcc			; A <- C
	xch	R10		; R10 <- 0 or 1
	xch	R7		; A <- original R11
	rar			; C=(R11 mod 2), rotate bit 0 into C
	cmc			; CY=1-(R11 mod 2), negate the pseudo jump condition
	ret	1		; return with A = 1, C = 1 jump

; ----- increment register WR from position R15
; $47A, QPC_7A:	INC WR_POS	increment WR from position in R15
Sqrt_IncWRPos:
	ld	R15		; A <- R15, start digit position
	xch	R9		; R9 <- R15, start digit position
	stc			; C <- 1, overflow flag to increase digits
Sqrt_IncWRPos2:
	ldi	0		; A <- 0
	src	R89		; select WR register (and digit) from R8R9
	adm			; A <- A + digit
	daa			; decimal adjust
	wrm			; write digit back from A
	ijnz	R9, Sqrt_IncWRPos2 ; increment R9 position and loop back for the next digits
	ret	0		; return with A = 0

; ----- decrement register WR from position R15
; $485, QPC_85:	DEC WR_POS	Decrement WR from position in R15
;
;inside the loop when R7 is subtracted from A and C is complemented:
;
;		C=0			C=1
;-------------------------------------------------------
;A 0		A=0, C=0		A=15->9, C=1
;A 1..9		A=A, C=0		A=A-1, C=0

Sqrt_DecWRPos:
	ld	R15		; A <- R15, start digit position
	xch	R9		; R9 <- R15, start digit position
Sqrt_DecWRPos2:
	cmc			; at first: set CY=1, later complement the borrow bit
	src	R89		; select WR register (and digit) from R8R9
	rdm			; A <- read next digit from WR
	sub	R7		; subtract R7 (=0) from it (to subtract C), A <- A + ~0 + ~C
	jc	Sqrt_DecWRPos4	; jump, if there is no borrow
	ldi	9		; A <- set the number to 9 (BCD adjust)
Sqrt_DecWRPos4:
	wrm			; write back the result digit
	ijnz	R9,Sqrt_DecWRPos2 ; increment R9 position and loop back for the next digits
	clb			; A <- 0, C <- 0
	ret	0		; return with A = 0

; ----- increment decimal point counter R10R11 and jump
; $493, QPC_93:	INC DPCNT + JMP		Increment digit point counter (R10R11) and unconditional jump
Sqrt_IncDPJmp:
	ijnz	R11,Sqrt_Jump	; inc R11, and skip if result is nonzero
	inc	R10		; inc R10 (increment decimal point counter HIGH)

; ----- unconditional jump
; $496, QPC_96:	unconditional jump
Sqrt_Jump:
	stc			; C <- 1, flag to jump
	ret	1		; return with A = 1, C = 1 (jump)

; ----- decrement SQRT loop counter R15 and jump if was not 0
; $498, QPC_98:	JPC NZERO_LPCSQRT + DEC LPCSQRT		decrement R15, and jump, except when R15 was 0
Sqrt_Loop:
	ld	R15		; A <- R15, sqrt loop counter
	dec	A		; decrement sqrt loop counter, C <- 1 if A was not 0
	xch	R15		; R15 <- new loop counter
	ret	1		; return with A = 1, C = 1 if counter was not 0 (= continue to loop)

; ----- right shift register R8R9 (WR)
; $49C, $15F SHR WR		right shift WR (A = new digit)
Sqrt_ShiftR:
	jmp	ShiftR		;one digit right shift of WR with R13 (0 is shifted from left)

= 0	; unused space

; ----- return back from SQRT macro, jump to num_dpadj
; $49F, QPC_9F:	CLR MOP + RET_BPC	Clear divide/multiply operation and return back to basic pseudo code interpreter
Sqrt_Ret:
	src	R67		; select register DR from R6R7 (=$20)
	wr2			; clear DR.S2 = mul/div status
	fim	R01,@num_dpadj	; entry address is num_dpadj (=$340), return to adjust result to max. 13 digits and print result
	fim	R67,$00
	jmp	Exec		;jump back to basic pseudo code interpreter

; ----- copy number from R8R9 (=WR) to R6R7+2 (=CR), with S0 and S1
; $4A7, $102 MOV CR,WR		move CR <- WR
Sqrt_MovCRWR:
	jmp	MovCRWR		; CR <- WR

; ----- copy number from R8R9 (=WR) to R6R7 (=DR), with S0 and S1 (or copy WR <- CR ?)
; $4A9, $104 MOV DR,WR		move DR <- WR
;QPC_A9:	MOV DR,WR (or MOV WR,CR)
;	Move working register into dividend/multiplicand register (DR=WR), but it is very probable that this would be
;	move constant register into working register (WR=CR)
Sqrt_MovDRWR2:
	jmp	MovDRWR		; Maybe it is "jmp $10c"
				;(the difference is only one bit in the code - was the source ROM damaged?)

;4ab          00 00 00 00 00		;Unused NOPs

.page

; fill-up to end of program space (4 KB)
*=$fff $ff
=$ff
