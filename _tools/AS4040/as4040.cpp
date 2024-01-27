// Intel 4004/4040 assembler compiler v1.10
// (c) 2020-2024 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com

#include "include.h"

/*
Non-standard extensions:
	.strict		- strict mode:
					- registers must have name R0..R15
					- register pairs must have name R01..R14R15
	.page [fill]	- align code to 256-byte page, can have fill byte or 0 default
	*=addr[,fill] - set address, can have fill byte or 0 default
	.set a b	- 'a' will by substituted by 'b' (use text without spaces)
	R01			- Register pair can be entered with name R01, R23, R45, R67, R89, R1011, R1213, R1415
	@label		- low byte of label can be used instead of data byte
	-number		- negate sign can be used before data byte
	'A'			- ASCII character can be used as data byte
	$12, 0x12, 12h - hex data byte can be entered with $, 0x or H
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
	NOP ... no operation
	HLT ... stop (only I4040)
	BBS ... return from interrupt (only I4040; interrupt service starts at address 0x003)
	LCR ... load DCL into A (only I4040)
	OR4 ... OR r4 to A (only I4040)
	OR5 ... OR r5 to A (only I4040)
	AN6 ... AND r6 to A (only I4040)
	AN7 ... AND r7 to A (only I4040)
	DB0 ... select ROM bank 0 (only I4040)
	DB1 ... select ROM bank 1 (only I4040)
	SB0 ... select register bank 0, register 0..7 (only I4040)
	SB1 ... select register bank 1, register 0..7 (only I4040)
	EIN ... enable interrupt (only I4040)
	DIN ... disable interrupt (only I4040)
	RPM ... read program memory from address 0xf00+SRC to A, first/last nibble (only I4040)

group 0x10..0xDF:
	JCN n,s ... jump with condition (bit0 TEST is 0 active, bit1 C is set, bit2 A is zero, bit3 negate condition)
		JT s .... jump if TEST is active (TEST=0)
		JNT s ... jump if TEST is not active (TEST=1)
		JC s .... jump if carry is set
		JNC s ... jump if carry is not set
		JZ s .... jump if A is zero
		JNZ s ... jump if A is not zero
		JR s .... jump relative (unconditional short jump)
	FIM rr,d ... fetch immediate data byte d into register pair rr (higher nibble into first register)
	SRC rr ... send register control from register pair
	FIN rr ... fetch indirect data byte from ROM address R0R1 into register pair rr
	JIN rr ... jump indirect to ROM address rr
	JUN a (JMP a) ... jump to address a
	JMS a (CALL a) ... call subroutine a
	INC r ... increment register r
	ISZ r,s (IJNZ r,s) ... increment r and jump if not zero to address s
	ADD r ... add register r and carry to A (A = A + r + carry)
	SUB r ... subtract register r and carry from A (A = A + ~r + ~carry)
	LD r ... load register r to A
	XCH r ... exchange register r and A
	BBL n (RET n) ... return from subroutine with code in A
	LDM n (LDI n) ... load immediate to A

group 0xE0..0xEF:
	WRM ... write A into RAM memory
	WMP ... write A into RAM port
	WRR ... write A into ROM port
	WPM ... write A into program RAM memory to address 0xf00+SRC to A, first/last nibble
	WR0 ... write A into RAM status 0
	WR1 ... write A into RAM status 1
	WR2 ... write A into RAM status 2
	WR3 ... write A into RAM status 3
	SBM ... subtract RAM and carry from A
	RDM ... read RAM memory into A
	RDR ... read ROM port into A
	ADM ... add RAM and carry to A
	RD0 ... read RAM status 0 into A
	RD1 ... read RAM status 1 into A
	RD2 ... read RAM status 2 into A
	RD3 ... read RAM status 3 into A

group 0xF0..0xFF:
	CLB ... clear both (A and carry)
	CLC .. clear carry
	IAC (INC A) ... increrment A (overflow -> carry)
	CMC ... complement carry
	CMA ... complement A
	RAL ... rotate A left through carry (carry<-a3<-a2<-a1<-a0<-carry)
	RAR ... rotate A right through carry (carry->a3->a2->a1->a0->carry)
	TCC ... transmit carry to A (0 or 1), clear carry
	DAC (DEC A) ... decrement A (~borrow -> carry)
	TCS ... transfer carry subtract to A (0->9 or 1->10), clear carry
	STC ... set carry
	DAA ... decimal adjust A (add 6 if A>9 or carry=1 and set carry to 1; or carry not affected otherwise)
	KBP ... keyboard process (A change "1 of 4" to bit number, or 15 if invalid)
	DCL ... designate command line (high byte of RAM address)
*/

CText FileIn;		// input filename
CText FileOut;		// output filename
CText FileList;		// list filename
CText FileCpp;		// C++ filename

CText FileListText;	// text of list file
CText FileCppText;	// text of C++ file

CText TextIn;		// input text
CTextList TextInList;	// input rows

int Pass; // current pass (1 or 2)
int Row; // current processed row
int RowNum; // number of rows
int Pos; // current processed position
BOOL Strict = FALSE; // strict mode (switch .strict) - require register name
int ErrorNum = 0; // error counter

CText RowText0;		// current row original
CText RowText;		// current row in uppercase
CText RowText2;		// current row original, without label

// output buffer
#define OUTMAX	4096	// max. size of output program
u8 OutBuf[OUTMAX];		// output buffer
int OutNum;				// number of bytes in output buffer, can be > OUTMAX

// label with address
typedef struct {
	CText	label;
	int		addr;		// address, -1=yet undefine
} sLabel;

#define LABMAX	10000	// max. labels
sLabel Label[LABMAX];	// labels
int LabelNum = 0;	// number of labels

// set symbols
CTextList SymListA, SymListB; // substitute symbolA by B

// controls
CText CmdStrict(_T(".STRICT"));
CText CmdPage(_T(".PAGE"));
CText CmdSet(_T(".SET"));

// commands
CText CmdNOP(_T("NOP"));
CText CmdHLT(_T("HLT"));
CText CmdBBS(_T("BBS"));
CText CmdLCR(_T("LCR"));
CText CmdOR4(_T("OR4"));
CText CmdOR5(_T("OR5"));
CText CmdAN6(_T("AN6"));
CText CmdAN7(_T("AN7"));
CText CmdDB0(_T("DB0"));
CText CmdDB1(_T("DB1"));
CText CmdSB0(_T("SB0"));
CText CmdSB1(_T("SB1"));
CText CmdEIN(_T("EIN"));
CText CmdDIN(_T("DIN"));
CText CmdRPM(_T("RPM"));

CText CmdJCN(_T("JCN"));
  CText CmdJT(_T("JT"));
  CText CmdJNT(_T("JNT"));
  CText CmdJC(_T("JC"));
  CText CmdJNC(_T("JNC"));
  CText CmdJZ(_T("JZ"));
  CText CmdJNZ(_T("JNZ"));
  CText CmdJR(_T("JR"));
CText CmdFIM(_T("FIM"));
CText CmdSRC(_T("SRC"));
CText CmdFIN(_T("FIN"));
CText CmdJIN(_T("JIN"));
CText CmdJUN(_T("JUN")); CText CmdJMP(_T("JMP"));
CText CmdJMS(_T("JMS")); CText CmdCALL(_T("CALL"));
CText CmdINC(_T("INC"));
CText CmdDEC(_T("DEC"));
CText CmdISZ(_T("ISZ")); CText CmdIJNZ(_T("IJNZ"));
CText CmdADD(_T("ADD"));
CText CmdSUB(_T("SUB"));
CText CmdLD(_T("LD"));
CText CmdXCH(_T("XCH"));
CText CmdBBL(_T("BBL")); CText CmdRET(_T("RET"));
CText CmdLDM(_T("LDM")); CText CmdLDI(_T("LDI"));

CText CmdWRM(_T("WRM")); // write memory
CText CmdWMP(_T("WMP")); // write memory port
CText CmdWRR(_T("WRR")); // write ROM port
CText CmdWPM(_T("WPM")); // write program memory
CText CmdWR0(_T("WR0"));
CText CmdWR1(_T("WR1"));
CText CmdWR2(_T("WR2"));
CText CmdWR3(_T("WR3"));
CText CmdSBM(_T("SBM"));
CText CmdRDM(_T("RDM"));
CText CmdRDR(_T("RDR"));
CText CmdADM(_T("ADM"));
CText CmdRD0(_T("RD0"));
CText CmdRD1(_T("RD1"));
CText CmdRD2(_T("RD2"));
CText CmdRD3(_T("RD3"));

CText CmdCLB(_T("CLB"));
CText CmdCLC(_T("CLC"));
CText CmdIAC(_T("IAC"));
CText CmdCMC(_T("CMC"));
CText CmdCMA(_T("CMA"));
CText CmdRAL(_T("RAL"));
CText CmdRAR(_T("RAR"));
CText CmdTCC(_T("TCC"));
CText CmdDAC(_T("DAC"));
CText CmdTCS(_T("TCS"));
CText CmdSTC(_T("STC"));
CText CmdDAA(_T("DAA"));
CText CmdKBP(_T("KBP"));
CText CmdDCL(_T("DCL"));

CText CmdEqu(_T("="));
CText CmdSetAddr(_T("*="));

// conditions
CText CondTZ(_T("TZ"));
CText CondTN(_T("TN"));
CText CondC1(_T("C1"));
CText CondC0(_T("C0"));
CText CondAZ(_T("AZ"));
CText CondAN(_T("AN"));

// register pair
CText Pair01(_T("R01"));
CText Pair23(_T("R23"));
CText Pair45(_T("R45"));
CText Pair67(_T("R67"));
CText Pair89(_T("R89"));
CText Pair1011(_T("R1011"));
CText Pair1213(_T("R1213"));
CText Pair1415(_T("R1415"));

CText RegA(_T("A"));

// find label (returns index in table or -1 if not found)
int FindLabel(CText lab)
{
	int i;
	for (i = 0; i < LabelNum; i++)
	{
		if (lab == Label[i].label) return i;
	}
	return -1;
}

// add label (does not check if exist)
void AddLabel(CText lab, int addr)
{
	if (LabelNum >= LABMAX) return;
	Label[LabelNum].label = lab;
	Label[LabelNum].addr = addr;
	LabelNum++;
}

// find symbol (returns index in table or -1 if not found)
int FindSym(CText sym)
{
	int i;
	for (i = 0; i < SymListA.Num(); i++)
	{
		if (sym == SymListA[i]) return i;
	}
	return -1;
}

// add symbol (does not check if exist)
void AddSym(CText sym, CText sub)
{
	SymListA.Add(sym);
	SymListB.Add(sub);
}

// error report
void Error(LPCTSTR msg, ...)
{
	if (Pass != 2) return;
	ErrorNum++;

	va_list args;
	va_start(args, msg);
	CText t;
	t.FormatV(msg, args);
	va_end(args);

	printf("ERROR on row %d: %s\n", Row + 1, (LPCTSTR)t);
	printf("\t%s\n", (LPCTSTR)RowText0);
}

// add byte to output buffer
void AddOut(u8 b)
{
	if (OutNum < OUTMAX) OutBuf[OutNum] = b;
	OutNum++;
}

// parse register (returns 0..15, or -1 on error)
s8 ParseReg(CText par)
{
	int i;
	char ch;
	s8 r = 0;

	// no register
	if (par.IsEmpty())
	{
		Error("missing register Rx");
		return -1;
	}

	// skip 'R'
	if (par.First() == 'R')
		par.DelFirst();
	else
	{
		// strict mode require register name
		if (Strict)
		{
			Error("register R0..R15 required");
			return -1;
		}
	}

	// binary number 4 digits
	if (par.Length() == 4)
	{
		for (i = 0; i < 4; i++)
		{
			ch = par[i];
			if ((ch != '0') && (ch != '1'))
			{
				Error("invalid digit '%c' in register name", ch);
				return -1;
			}
			r <<= 1;
			r += ch - '0';
		}
		return r;
	}

	// decimal number
	if ((par.Length() == 0) || (par.Length() > 2))
	{
		Error("invalid register %s", (LPCTSTR)par);
		return -1;
	}

	ch = par[0];
	if ((ch < '0') || (ch > '9'))
	{
		Error("invalid register name %s", (LPCTSTR)par);
		return -1;
	}
	r = ch - '0';

	if (par.Length() == 2)
	{
		ch = par[1];
		if ((ch < '0') || (ch > '9'))
		{
			Error("invalid register name %s", (LPCTSTR)par);
			return -1;
		}
		r = r*10 + (ch - '0');
	}

	// check register number
	if (r > 15)
	{
		Error("register 0..15 required (invalid register %d)", r);
		return -1;
	}

	return r;
}

// parse register pair (returns 0..14, or -1 on error)
s8 ParsePair(CText par)
{
	int i;
	char ch;
	s8 r = 0;

	// no register
	if (par.IsEmpty())
	{
		Error("missing register pair Px");
		return -1;
	}

	// register pair R01, R23, R45, R67, R89, R1011, R1213, R1415
	if (par == Pair01) return 0;
	if (par == Pair23) return 2;
	if (par == Pair45) return 4;
	if (par == Pair67) return 6;
	if (par == Pair89) return 8;
	if (par == Pair1011) return 10;
	if (par == Pair1213) return 12;
	if (par == Pair1415) return 14;

	// strict mode require register pair name
	if (Strict)
	{
		Error("register pair R01..R1415 required");
		return -1;
	}

	// first character must be 'P' or last character must be '<'
	if ((par.First() != 'P') && (par.Last() != '<'))
	{
		Error("invalid register pair %s", (LPCTSTR)par);
		return -1;
	}

	// skip 'P' and '<'
	if (par.First() == 'P') par.DelFirst();
	if (par.Last() == '<') par.DelLast();

	// binar number 3 digits
	if (par.Length() == 3)
	{
		for (i = 0; i < 3; i++)
		{
			ch = par[i];
			if ((ch != '0') && (ch != '1'))
			{
				Error("invalid digit '%c' in register pair name", ch);
				return -1;
			}
			r <<= 1;
			r += ch - '0';
		}
		return r*2;
	}

	// decimal number
	if (par.Length() != 1)
	{
		Error("invalid register pair name %s", (LPCTSTR)par);
		return -1;
	}

	ch = par[0];
	if ((ch < '0') || (ch > '7'))
	{
		Error("invalid register pair name %s", (LPCTSTR)par);
		return -1;
	}
	r = ch - '0';

	return r*2;
}

// parse 12-bit number (returns 0..0x0fff, or -1 on error)
s16 Parse12bit(CText par)
{
	int i;
	s32 d = 0;
	char ch;

	// no data
	if (par.IsEmpty())
	{
		Error("missing immediate data");
		return -1;
	}

	// hex value $12, 0x12, 12h
	BOOL hex = FALSE;
	if (par.First() == '$')
	{
		par.DelFirst();
		hex = TRUE;
	}
	if ((par.Length() >= 2) && (par.First() == '0') && (par[1] == 'X'))
	{
		par.Delete(0, 2);
		hex = TRUE;
	}
	if (par.Last() == 'H')
	{
		par.DelLast();
		hex = TRUE;
	}

	// load HEX number
	if (hex)
	{
		for (i = 0; i < par.Length(); i++)
		{
			ch = par[i];
			if ((ch < '0') || (ch > 'F') || ((ch > '9') && (ch < 'A')))
			{
				Error("invalid digit '%c' in hex number", ch);
				return -1;
			}
			if (ch > '9') ch -= 'A' - '9' - 1;
			d = d*16 + (ch - '0');
			if (d > 0x0fff)
			{
				Error("number %d is out of range", d);
				return -1;
			}
		}
		return (s16)d;
	}

	// load decimal number
	for (i = 0; i < par.Length(); i++)
	{
		ch = par[i];
		if ((ch < '0') || (ch > '9'))
		{
			Error("invalid digit '%c' in number", ch);
			return -1;
		}
		d = d*10 + (ch - '0');
		if (d > 0x0fff)
		{
			Error("number %d is out of range", d);
			return -1;
		}
	}
	return (s16)d;
}

// parse data byte (returns 0..255, or -1 on error)
s16 ParseData(CText par)
{
	int i;
	s16 d = 0;

	// no data
	if (par.IsEmpty())
	{
		Error("missing immediate data");
		return -1;
	}

	// low byte of label @label
	if (par.First() == '@')
	{
		par.DelFirst();
		par.TrimLeft();
		if (par.IsEmpty())
		{
			Error("invalid label %s", (LPCTSTR)par);
			return -1;
		}

		// find label
		i = FindLabel(par);

		// pass 2
		if (Pass == 2)
		{
			if ((i >= 0) && (Label[i].addr >= 0))
				return (u8)Label[i].addr;
			else
			{
				Error("label %s not found", (LPCTSTR)par);
				return -1;
			}
		}

		// pass 1 - add new label
		if (i < 0) AddLabel(par, -1);
		return 0;
	}

	// negate
	BOOL neg = FALSE;
	if (par.First() == '-')
	{
		par.DelFirst();
		par.TrimLeft();
		if (par.IsEmpty())
		{
			Error("missing immediate data");
			return -1;
		}
		neg = TRUE;
	}

	// character constant 'A' or "A"
	if ((((par.First() == '\'') && (par.Last() == '\'')) ||
		((par.First() == '"') && (par.Last() == '"'))) && (par.Length() == 3))
	{
		if (neg)
			return (u8)(-par[1]);
		else
			return (u8)par[1];
	}

	// load number
	d = Parse12bit(par);
	if (d < 0) return -1;
	if (d > 0xff)
	{
		Error("number %d is out of range", d);
		return -1;
	}

	if (neg)
		return (u8)(-d);
	else
		return (u8)d;
}

// parse nibble (returns 0..15, or -1 on error)
s8 ParseNibble(CText par)
{
	s16 d = 0;
	char ch;
	int i;

	// no data
	if (par.IsEmpty())
	{
		Error("missing immediate data");
		return -1;
	}

	// binary form
	if (par.Length() == 4)
	{
		for (i = 0; i < 4; i++)
		{
			ch = par[i];
			if ((ch != '0') && (ch != '1'))
			{
				Error("invalid digit '%c' in binary number", ch);
				return -1;
			}
			d <<= 1;
			d += ch - '0';
		}
		return (u8)d;
	}

	// negate
	BOOL neg = FALSE;
	if (par.First() == '-')
	{
		par.DelFirst();
		par.TrimLeft();
		if (par.IsEmpty())
		{
			Error("missing immediate data");
			return -1;
		}
		neg = TRUE;
	}

	// parse data
	d = ParseData(par);
	if (d < 0) return -1;
	if (d > 0x0f)
	{
		Error("number %d is out of range", d);
		return -1;
	}

	if (neg) d = -d;
	return (s8)(d & 0x0f);
}

// parse 8-bit address (returns address 0..0xff, or -1 on error)
s16 ParseAddr8(CText par)
{
	int i;

	if (par.IsEmpty())
	{
		Error("invalid label");
		return -1;
	}

	// find label
	i = FindLabel(par);

	// pass 2
	if (Pass == 2)
	{
		// label found ok
		if ((i >= 0) && (Label[i].addr >= 0))
		{
			// check if label is on the same page
			if ((Label[i].addr & 0xf00) != (OutNum & 0xf00))
			{
				Error("label %s is not on the same page", (LPCTSTR)par);
				return -1;
			}		
			return (u8)Label[i].addr;
		}
		else
		{
			Error("label %s not found", (LPCTSTR)par);
			return -1;
		}
	}

	// pass 1 - add new label
	if (i < 0) AddLabel(par, -1);
	return 0;
}

// parse 12-bit address (returns address 0..0xfff, or -1 on error)
s16 ParseAddr12(CText par)
{
	int i;

	if (par.IsEmpty())
	{
		Error("invalid label");
		return -1;
	}

	// find label
	i = FindLabel(par);

	// pass 2
	if (Pass == 2)
	{
		// label found ok
		if ((i >= 0) && (Label[i].addr >= 0))
		{
			return Label[i].addr;
		}
		else
		{
			Error("label %s not found", (LPCTSTR)par);
			return -1;
		}
	}

	// pass 1 - add new label
	if (i < 0) AddLabel(par, -1);
	return 0;
}

// parse jump condition (returns 0..15, or -1 on error)
s8 ParseCond(CText par)
{
	if (par.IsEmpty())
	{
		Error("invalid condition");
		return -1;
	}

	if (par == CondTZ) return 1; // TEST is active, 0 (0001)
	if (par == CondTN) return 9; // TEST is not active, 1 (1001)
	if (par == CondC1) return 2; // C is 1 (0010)
	if (par == CondC0) return 10; // C is 0 (1010)
	if (par == CondAZ) return 4; // A is 0 (0100)
	if (par == CondAN) return 12; // A is not 0 (1100)

	return ParseNibble(par); // number
}

// compile one row
BOOL Compile()
{
	CText lab, cmd, par, par2;
	s16 a;
	u8 b;
	int i;
	BOOL garb1 = FALSE;
	BOOL garb2 = FALSE;

	// cut comment
	int pos = RowText.Find(';');
	if (pos >= 0) RowText.DelToEnd(pos);

	// convert to uppercase
	RowText.UpperCase();

	// find label
	pos = RowText.Find(':');
	if (pos >= 0)
	{
		lab = RowText.Left(pos);
		lab.Trim();
		RowText.Delete(0, pos+1);
		RowText2.Delete(0, pos+1);

		// empty label
		if (lab.IsEmpty())
		{
			Error("invalid label");
			return FALSE;
		}

		// substitute label
		i = FindSym(lab);
		if (i >= 0) lab = SymListB[i];

		// find label
		pos = FindLabel(lab);

		// pass 1
		if (Pass == 1)
		{
			// label already exists
			if ((pos >= 0) && (Label[pos].addr >= 0))
				return FALSE;

			// update label or add new label
			if (pos >= 0)
				Label[pos].addr = OutNum;
			else
				AddLabel(lab, OutNum);
		}

		// pass 2
		if (Pass == 2)
		{
			// label already exists
			if ((pos >= 0) && (Label[pos].addr != OutNum))
			{
				Error("label %s already exists", (LPCTSTR)lab);
				return FALSE;
			}

			// only update label
			if (pos >= 0)
				Label[pos].addr = OutNum;
		}
	}

	// trim command line
	RowText.Trim();
	RowText2.TrimLeft();

	// get command
	if (RowText.Left(2) == CmdSetAddr) // *=
		pos = 2;
	else if (RowText.First() == '=') // =
		pos = 1;
	else
	{
		pos = RowText.FindList(" \t");
		if (pos < 0) pos = RowText.Length();
	}
	cmd = RowText.Left(pos);
	if (cmd.IsEmpty()) return TRUE; // no command
	RowText.Delete(0, pos);
	RowText2.Delete(0, pos);

	// get first parameter
	RowText.Trim();
	RowText2.TrimLeft();
	pos = RowText.FindList(" \t,");
	if (pos < 0) pos = RowText.Length();
	par = RowText.Left(pos);
	if (((RowText2.First() == '\'') && (RowText2.At(2) == '\'')) ||
		((RowText2.First() == '"') && (RowText2.At(2) == '"')))
	{
		par = RowText2.Left(3);
		pos = 3;
	}

	// get second parameter
	RowText.Delete(0, pos);
	RowText.Trim();
	if (RowText.First() == ',') RowText.DelFirst();
	RowText.TrimLeft();
	par2 = RowText;

	// set command
	if (cmd == CmdSet)
	{
		if (par.IsEmpty())
		{
			Error("missing symbol to set");
			return FALSE;
		}

		i = FindSym(par);
		if (i >= 0)
			SymListB[i] = par2; // update symbol
		else
			AddSym(par, par2);
		return TRUE;
	}

	// substitute symbols
	i = FindSym(cmd);
	if (i >= 0) cmd = SymListB[i];
	i = FindSym(par);
	if (i >= 0) par = SymListB[i];
	i = FindSym(par2);
	if (i >= 0) par2 = SymListB[i];

	// process command
	if (cmd == CmdStrict) // .strict
	{
		Strict = TRUE;
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdPage) // .page
	{
		b = 0;
		if (par.IsNotEmpty()) b = (u8)ParseData(par);
		while ((OutNum & 0x00ff) != 0) AddOut(b);
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdNOP) // NOP; 0000 0000
	{
		AddOut(0);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdHLT) // HLT; 0000 0001
	{
		AddOut(0x01);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdBBS) // BBS; 0000 0010
	{
		AddOut(0x02);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdLCR) // LCR; 0000 0011
	{
		AddOut(0x03);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdOR4) // OR4; 0000 0100
	{
		AddOut(0x04);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdOR5) // OR5; 0000 0101
	{
		AddOut(0x05);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdAN6) // AN6; 0000 0110
	{
		AddOut(0x06);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdAN7) // AN7; 0000 0111
	{
		AddOut(0x07);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDB0) // DB0; 0000 1000
	{
		AddOut(0x08);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDB1) // DB1; 0000 1001
	{
		AddOut(0x09);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdSB0) // SB0; 0000 1010
	{
		AddOut(0x0A);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdSB1) // SB1; 0000 1011
	{
		AddOut(0x0B);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdEIN) // EIN; 0000 1100
	{
		AddOut(0x0C);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDIN) // DIN; 0000 1101
	{
		AddOut(0x0D);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRPM) // RPM; 0000 1110
	{
		AddOut(0x0E);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdJCN) // JCN c,a; 0001 CCCC AAAA AAAA
	{
		AddOut((u8)(0x10 | ParseCond(par)));
		AddOut((u8)ParseAddr8(par2));
	}
	else if (cmd == CmdJT) // JT a; 0001 0001 AAAA AAAA
	{
		AddOut(0x11);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJNT) // JNT a; 0001 1001 AAAA AAAA
	{
		AddOut(0x19);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJC) // JC a; 0001 0010 AAAA AAAA
	{
		AddOut(0x12);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJNC) // JNC a; 0001 1010 AAAA AAAA
	{
		AddOut(0x1a);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJZ) // JZ a; 0001 0100 AAAA AAAA
	{
		AddOut(0x14);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJNZ) // JNZ a; 0001 1100 AAAA AAAA
	{
		AddOut(0x1c);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJR) // JR a; 0001 1000 AAAA AAAA
	{
		AddOut(0x18);
		AddOut((u8)ParseAddr8(par));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdFIM)	// FIM p,d; 0010 RRR0 DDDD DDDD
	{
		AddOut((u8)(0x20 | ParsePair(par)));
		AddOut((u8)ParseData(par2));
	}
	else if (cmd == CmdSRC) // SRC p; 0010 RRR1
	{
		AddOut((u8)(0x21 | ParsePair(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdFIN) // FIN p; 0011 RRR0
	{
		AddOut((u8)(0x30 | ParsePair(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdJIN) // JIN p; 0011 RRR1
	{
		AddOut((u8)(0x31 | ParsePair(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if ((cmd == CmdJUN) || (cmd == CmdJMP)) // JUN a; 0100 AAAA AAAA AAAA
	{
		a = ParseAddr12(par) | 0x4000;
		AddOut((u8)(a >> 8));
		AddOut((u8)a);
		garb2 = par2.IsNotEmpty();
	}
	else if ((cmd == CmdJMS) || (cmd == CmdCALL)) // JMS a; 0101 AAAA AAAA AAAA
	{
		a = ParseAddr12(par) | 0x5000;
		AddOut((u8)(a >> 8));
		AddOut((u8)a);
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdINC) // INC r; 0110 RRRR .. or INC A; 1111 0010
	{
		if (par == RegA)
			AddOut(0xf2);
		else
			AddOut((u8)(0x60 | ParseReg(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if ((cmd == CmdISZ) || (cmd == CmdIJNZ)) // ISZ r,a; 0111 RRRR AAAA AAAA
	{
		AddOut((u8)(0x70 | ParseReg(par)));
		AddOut((u8)ParseAddr8(par2));
	}
	else if (cmd == CmdADD) // ADD r; 1000 RRRR
	{
		AddOut((u8)(0x80 | ParseReg(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdSUB) // SUB r; 1001 RRRR
	{
		AddOut((u8)(0x90 | ParseReg(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdLD) // LD r; 1010 RRRR
	{
		AddOut((u8)(0xA0 | ParseReg(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdXCH) // XCH r; 1011 RRRR
	{
		AddOut((u8)(0xB0 | ParseReg(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if ((cmd == CmdBBL) || (cmd == CmdRET)) // BBL d; 1100 DDDD
	{
		AddOut((u8)(0xC0 | ParseNibble(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if ((cmd == CmdLDM) || (cmd == CmdLDI)) // LDM d; 1101 DDDD
	{
		AddOut((u8)(0xD0 | ParseNibble(par)));
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdWRM) // WRM; 1110 0000
	{
		AddOut(0xE0);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWMP) // WMP; 1110 0001
	{
		AddOut(0xE1);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWRR) // WRR; 1110 0010
	{
		AddOut(0xE2);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWPM) // WPM; 1110 0011
	{
		AddOut(0xE3);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWR0) // WR0; 1110 0100
	{
		AddOut(0xE4);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWR1) // WR1; 1110 0101
	{
		AddOut(0xE5);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWR2) // WR2; 1110 0110
	{
		AddOut(0xE6);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdWR3) // WR3; 1110 0111
	{
		AddOut(0xE7);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdSBM) // SBM; 1110 1000
	{
		AddOut(0xE8);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRDM) // RDM; 1110 1001
	{
		AddOut(0xE9);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRDR) // RDR; 1110 1010
	{
		AddOut(0xEA);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdADM) // ADM; 1110 1011
	{
		AddOut(0xEB);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRD0) // RD0; 1110 1100
	{
		AddOut(0xEC);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRD1) // RD1; 1110 1101
	{
		AddOut(0xED);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRD2) // RD2; 1110 1110
	{
		AddOut(0xEE);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRD3) // RD3; 1110 1111
	{
		AddOut(0xEF);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdCLB) // CLB; 1111 0000
	{
		AddOut(0xF0);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdCLC) // CLC; 1111 0001
	{
		AddOut(0xF1);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdIAC) // IAC; 1111 0010
	{
		AddOut(0xF2);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdCMC) // CMC; 1111 0011
	{
		AddOut(0xF3);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdCMA) // CMA; 1111 0100
	{
		AddOut(0xF4);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRAL) // RAL; 1111 0101
	{
		AddOut(0xF5);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdRAR) // RAR; 1111 0110
	{
		AddOut(0xF6);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdTCC) // TCC; 1111 0111
	{
		AddOut(0xF7);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDAC) // DAC; 1111 1000
	{
		AddOut(0xF8);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDEC) // DEC A; 1111 1000 (= DAC)
	{
		if (par == RegA)
			AddOut(0xF8);
		else
		{
			Error("only register A supported");
			return FALSE;
		}
		garb2 = par2.IsNotEmpty();
	}
	else if (cmd == CmdTCS) // TCS; 1111 1001
	{
		AddOut(0xF9);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdSTC) // STC; 1111 1010
	{
		AddOut(0xFA);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDAA) // DAA; 1111 1011
	{
		AddOut(0xFB);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdKBP) // KBP; 1111 1100
	{
		AddOut(0xFC);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdDCL) // DCL; 1111 1101
	{
		AddOut(0xFD);
		garb1 = par.IsNotEmpty();
	}
	else if (cmd == CmdEqu) // =
	{
		AddOut((u8)ParseData(par));
		if (par2.IsNotEmpty())
			AddOut((u8)ParseData(par2));
	}
	else if (cmd == CmdSetAddr) // *=
	{
		a = Parse12bit(par);
		if (a < 0) return FALSE;
		if (a < OutNum)
		{
			Error("cannot roll address back, current pc = %d", OutNum);
			return FALSE;
		}
		b = 0;
		if (par2.IsNotEmpty()) b = (u8)ParseData(par2);
		while (OutNum < a) AddOut(b);
	}
	else
	{
		Error("unknown opcode %s", (LPCTSTR)cmd);
		return FALSE;
	}

	// garbage
	if (garb1 || garb2)
	{
		Error("garbage %s at end of line", (LPCTSTR)(garb1 ? par : par2));
		return FALSE;
	}

	return TRUE;
}

// main function
int main(int argc, char **argv)
{
	int oldnum, pos, oldnum2;
	CText t, t1, t2, t3;

	// print info
	printf("Intel 4004/4040 assembler compiler v1.10\n");
	printf("(c) 2020-2024 Miroslav Nemecek, Panda38@seznam.cz\n");

	// check syntax
	if (argc != 2)
	{
		printf("Syntax: as4040 file.s\n");
		return 1;
	}

	// prepare filenames
	FileIn = argv[1];
	int len = FileIn.FindRev('.');
	if (len < 0) len = FileIn.Length();
	FileOut = FileIn.Left(len);
	t1 = FileOut;
	FileList = FileOut;
	FileCpp = FileOut;
	FileOut += _T(".bin");
	FileList += _T(".lst");
	FileCpp += _T(".cpp");

	// load input file
	if (!TextIn.LoadFileName(FileIn))
	{
		printf("Cannot load input file %s\n", (LPCTSTR)FileIn);
		return 1;
	}

	// split text to rows
	TextInList.SplitLines(TextIn);

	// info
	RowNum = TextInList.Num();
	printf("Parsing %d rows of file %s\n", RowNum, (LPCTSTR)FileIn);

	FileCppText.Format("#include \"../include.h\"\n\nconst u8 %s_Prog[] = {\r\n", (LPCTSTR)t1);

	// loop passes
	for (Pass = 1; Pass <= 2; Pass++)
	{
		// reset number of bytes in output buffer
		OutNum = 0;
		oldnum = 0;

		// loop rows
		for (Row = 0; Row < RowNum; Row++)
		{
			// load one row
			RowText0 = TextInList[Row];
			RowText = RowText0;
			RowText2 = RowText0;

			// compile one row
			Compile();

			// generate list file
			if (Pass == 2)
			{
				t.Empty();
				pos = 0;
				if (OutNum > oldnum)
				{
					FileCppText += "\t";
					t.Format("%03X", oldnum);
					oldnum2 = oldnum;
					pos = t.Length();
					while ((OutNum > oldnum) && (oldnum < OUTMAX))
					{
						if (pos >= 3 + 16*3)
						{
							t += _T("\r\n");
							FileListText += t;
							t = _T("   ");
							pos = 3;
							FileCppText += "\r\n\t";
						}

						t2 = t;
						t.Format("%s %02X", (LPCTSTR)t2, OutBuf[oldnum]);
						t3.Format("0x%02X, ", OutBuf[oldnum]);
						FileCppText += t3;
						pos += 3;
						oldnum++;
					}
					if (t.Length() < 8) FileCppText += '\t';
					t3.Format("\t// %03X\t", oldnum2);
					FileCppText += t3;
					FileCppText += RowText0;
					FileCppText += "\r\n";
				}
				if (t.Length() < 8) t += '\t';
				t += '\t';
				t += RowText0;
				t += '\r';
				t += '\n';
				FileListText += t;
			}
		}
	}

	FileCppText += "};\r\n";
	t3.Format("\r\nconst int %s_ProgLen = %d;\r\n", (LPCTSTR)t1, OutNum);
	FileCppText += t3;

	// overflow code size
	if (OutNum > OUTMAX)
	{
		printf("Overflow code size by %d bytes\n", OutNum - OUTMAX);
		return 1;
	}

	// check result
	if (ErrorNum == 0)
	{
		printf("Compiled OK, code size %d bytes\n", OutNum);

		// write output file
		FILE* f = fopen(FileOut, "wb");
		if (f == NULL)
		{
			printf("Cannot create output file %s\n", (LPCTSTR)FileOut);
			return 1;
		}
		int n = (int)fwrite(OutBuf, 1, OutNum, f);
		if (n != OutNum)
		{
			printf("Error write to output file %s\n", (LPCTSTR)FileOut);
			return 1;
		}
		fclose(f);

		// write list file
		FileListText.SaveFileName(FileList);

		// write C file
		FileCppText.SaveFileName(FileCpp);
		return 0;
	}
	else
	{
		printf("Found %d errors\n", ErrorNum);
		return 1;
	}
}
