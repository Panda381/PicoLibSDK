// HID compiler and decompiler

// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#pragma warning(disable : 4996) // deprecated function
#include <stdio.h>

// base types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
typedef signed long long int s64;
typedef unsigned long long int u64;
typedef unsigned int uint;
typedef unsigned char Bool;
#define True 1
#define False 0
#define count_of(a) (sizeof(a)/sizeof((a)[0]))

#include "HidCompDef.h"			// definitions
#include "HidCompTab.h"			// token tables

FILE* FileIn; // input file
FILE* FileOut; // output file

// input buffer
#define BUFN	100000	// buffer size
char BufIn[BUFN];		// input buffer
int BufInNum;			// number of character in input buffer
int BufInInx;			// index in input buffer

// report array
typedef struct {
	u8		prefix;			// prexif
	u32		data;			// data
} sRep;

sRep Rep[BUFN];			// report array
int RepNum = 0;			// number of entries in report array

// list of constant tables
const sToken* TokenList[6] = { TokenInput, TokenColl, TokenPage, TokenDesktop, TokenConsumer, TokenFido };
int TokenCount[6] = { count_of(TokenInput), count_of(TokenColl), count_of(TokenPage), count_of(TokenDesktop), count_of(TokenConsumer) }; 

// load character from input buffer (0 = end of text)
char ParseCh()
{
	char ch = BufIn[BufInInx];
	if (ch != 0) BufInInx++;
	return ch;
}

// return character back to input buffer
void ParseRet() { BufInInx--; }

// skip white spaces and comments
void ParseSpc()
{
	char ch;
	for (;;)
	{
		// skip white spaces
		for (;;)
		{
			ch = ParseCh();
			if (ch == 0) return; // end of text
			if (((ch < 0) || (ch > 32)) && (ch != ',') &&
					(ch != '\\') && (ch != '|') && (ch != '+')) // valid character
			{
				ParseRet();
				break;;
			}
		}

		// comment to end of line
		if ((BufIn[BufInInx] == '/') &&
			(BufIn[BufInInx+1] == '/'))
		{
			for (;;)
			{
				ch = ParseCh();
				if (ch == 0) return;
				if ((ch == 13) || (ch == 10)) break;
			}
		}
		else

		// long comment
		if ((BufIn[BufInInx] == '/') &&
			(BufIn[BufInInx+1] == '*'))
		{
			ParseCh();
			ParseCh();

			for (;;)
			{
				ch = ParseCh();
				if (ch == 0) return;
				if ((ch == '*') && (BufIn[BufInInx] == '/'))
				{
					ParseCh();
					break;
				}
			}
		}
		else
			// no other comments
			break;
	}
}

// find token (returns token index or -1 if not found)
int FindToken(const sToken* tab, int num)
{
	// skip white spaces and comments
	ParseSpc();

	// start of token
	const char* txt = &BufIn[BufInInx];

	// loop table
	int i, j;
	for (i = 0; i < num; i++)
	{
		// compare text of one token
		for (j = 0; ; j++)
		{
			// end of token
			if (tab->name[j] == 0)
			{
				// check end of searched text
				if ((txt[j] == '_') ||
					((txt[j] >= 'A') && (txt[j] <= 'Z')))
					break; // searched token is longer

				// token has been found OK
				BufInInx += j; // skip token
				return i;
			}

			// not equal
			if (tab->name[j] != txt[j]) break;
		}
		tab++;
	}

	// not found
	return -1;
}

// find token value (returns token index of -1 if not found)
int FindVal(const sToken* tab, int num, int val)
{
	// loop table
	int i;
	for (i = 0; i < num; i++)
	{
		if (tab->val == val) return i;
		tab++;
	}

	// not found
	return -1;
}

// load number
int ParseNum()
{
	char ch;
	int n = 0;

	// skip white spaces and comments
	ParseSpc();

	// load sign
	Bool neg = False;
	if (BufIn[BufInInx] == '-')
	{
		BufInInx++;
		neg = True;
		ParseSpc();
	}

	// hex number
	if ((BufIn[BufInInx] == '0') && (BufIn[BufInInx+1] == 'X'))
	{
		BufInInx += 2;

		for (;;)
		{
			ch = ParseCh();
			if ((ch >= '0') && (ch <= '9'))
			{
				n = n*16 + ch - '0';
			}
			else
				if ((ch >= 'A') && (ch <= 'F'))
				{
					n = n*16 + ch - 'A' + 10;
				}
				else
			{
				if (ch != 0) ParseRet();
				break;
			}
		}
	}
	else
	{
		// decimal number
		for (;;)
		{
			ch = ParseCh();
			if ((ch >= '0') && (ch <= '9'))
			{
				n = n*10 + ch - '0';
			}
			else
			{
				if (ch != 0) ParseRet();
				break;
			}
		}
	}
	return neg ? -n : n;
}

// read data
int ParseData(int len)
{
	int n = 0;
	if (len >= 1)
	{
		n = ParseNum();
		if (len >= 2)
		{
			n += ParseNum() * 256;
			if (len >= 3)
			{
				n += ParseNum() * 256*256;
				n += ParseNum() * 256*256*256;
			}
		}
	}
	return n;
}

// parse input file (return False on error)
Bool ParseIn(const char* filename)
{
	// reset pointer
	BufInInx = 0;

	// reset report array
	RepNum = 0;

	// parse entries
	while (BufIn[BufInInx] != 0)
	{
		ParseSpc(); // skip spaces
		if (BufIn[BufInInx] == 0) break;

		// search prefix token
		int token = FindToken(TokenPrefix, count_of(TokenPrefix));
		if (token >= 0)
		{
			Rep[RepNum].prefix = TokenPrefix[token].val;
			Rep[RepNum].data = 0;

			// load length
			int len = 1;
			if (BufIn[BufInInx] == '0')
			{
				BufInInx++;
				len = 0;
			}
			else if (BufIn[BufInInx] == '2')
			{
				BufInInx++;
				len = 2;
			}
			else if (BufIn[BufInInx] == '3')
			{
				BufInInx++;
				len = 3;
			}

			// check parameter
			ParseSpc(); // skip spaces
			if (BufIn[BufInInx] == '(')
			{
				ParseCh();

				// parse constants
				for (;;)
				{
					ParseSpc(); // skip spaces

					// end
					if (BufIn[BufInInx] == ')')
					{
						ParseCh();
						break;
					}
					if (BufIn[BufInInx] == 0) break;

					// search tokens
					int k;
					for (k = 0; k < 6; k++)
					{
						int token2 = FindToken(TokenList[k], TokenCount[k]);
						if (token2 >= 0)
						{
							Rep[RepNum].data += (TokenList[k])[token2].val;
							break;
						}
					}

					// token not found, try number
					if (k == 6)
					{
						int oldinx = BufInInx;
						int n = ParseNum();
						if (oldinx != BufInInx)
						{
							Rep[RepNum].data += n;
						}
						else
						{
							// syntax error
							if (BufInInx < BufInNum-50) BufIn[BufInInx+50] = 0;
							printf("ERROR %s, cannot parse: %s\n", filename, &BufIn[BufInInx]);
							return False;
						}
					}
				}
			}

			// limit data - unsigned token
			if ((token <= HID_USAGE_PAGE_INX) || (token == HID_USAGE_INX))
			{
				if (len == 1)
					Rep[RepNum].data = (u8)Rep[RepNum].data;
				else if (len == 2)
					Rep[RepNum].data = (u16)Rep[RepNum].data;
				else if (len == 0)
					Rep[RepNum].data = 0;
			}

			// limit data - signed token
			else
			{
				if (len == 1)
					Rep[RepNum].data = (s8)Rep[RepNum].data;
				else if (len == 2)
					Rep[RepNum].data = (s16)Rep[RepNum].data;
				else if (len == 0)
					Rep[RepNum].data = 0;
			}

			RepNum++;
		}

		// token not found, try to load number
		else
		{
			int oldinx = BufInInx;
			int n = ParseNum();
			if (oldinx != BufInInx)
			{
				Rep[RepNum].prefix = (u8)n;
				Rep[RepNum].data = ParseData(n & 3);

				// search token
				int token = FindVal(TokenPrefix, count_of(TokenPrefix), (u8)n & ~3);

				// unsigned limit data
				int len = (u8)n & 3;
				if (((token >= 0) && (token <= HID_USAGE_PAGE_INX)) || (token == HID_USAGE_INX))
				{
					if (len == 1)
						Rep[RepNum].data = (u8)Rep[RepNum].data;
					else if (len == 2)
						Rep[RepNum].data = (u16)Rep[RepNum].data;
					else if (len == 0)
						Rep[RepNum].data = 0;
				}

				// limit data - signed token
				else
				{
					if (len == 1)
						Rep[RepNum].data = (s8)Rep[RepNum].data;
					else if (len == 2)
						Rep[RepNum].data = (s16)Rep[RepNum].data;
					else if (len == 0)
						Rep[RepNum].data = 0;
				}

				RepNum++;
			}
			else
			{
				// syntax error
				if (BufInInx < BufInNum-50) BufIn[BufInInx+50] = 0;
				printf("ERROR %s, cannot parse: %s\n", filename, &BufIn[BufInInx]);
				return False;
			}
		}
	}

	printf("Parse %s: OK, %d tokens\n", filename, RepNum);
	return True;
}

// output character
void OutChar(char ch)
{
	fprintf(FileOut, "%c", ch);
}

// output text
void OutTxt(const char* txt)
{
	fprintf(FileOut, "%s", txt);
}

// output HEX byte
void OutHex(u8 ch)
{
	fprintf(FileOut, "0x%02X,", ch);
}

// output decimal number
void OutNum(int num)
{
	fprintf(FileOut, "%d", num);
}

// main function
int main(int argc, char* argv[])
{
	// check arguments
	if (argc != 3)
	{
		printf("Usage: HidComp input_file output_file\n");
		return 1;
	}

	// open input file
	FileIn = fopen(argv[1], "r");
	if (FileIn == NULL)
	{
		printf("ERROR: Cannot open input file %s\n", argv[1]);
		return 1;
	}

	// read file to input buffer
	BufInNum = (int)fread(BufIn, 1, BUFN-1, FileIn);
	fclose(FileIn);
	if (BufInNum == 0)
	{
		printf("ERROR: Cannot read from input file %s\n", argv[1]);
		return 1;
	}
	BufIn[BufInNum] = 0; // terminating 0

	// open output file
	FileOut = fopen(argv[2], "w");
	if (FileOut == NULL)
	{
		printf("ERROR: Cannot create output file %s\n", argv[2]);
		return 1;
	}

	// convert input buffer to uppercase
	int i, j;
	for (i = 0; i < BufInNum; i++)
		if ((BufIn[i] >= 'a') && (BufIn[i] <= 'z'))
			BufIn[i] = BufIn[i] - 'a' + 'A';

	// parse input file
	if (!ParseIn(argv[1])) return 1;

	// compile output file
	int lastpage = HID_USAGE_PAGE_DESKTOP_INX; // last usage page
	int level = 0;
	sRep* r = Rep;
	for (i = 0; i < RepNum; i++)
	{
		// one tab
		OutChar(9);

		// find token
		u8 pref = r->prefix & ~3;
		int token = FindVal(TokenPrefix, count_of(TokenPrefix), pref);

		// token length
		int val = r->data;
		int len = 0;
		if (val != 0)
		{
			len = 1;

			// unsigned token
			if (((token >= 0) && (token <= HID_USAGE_PAGE_INX)) || (token == HID_USAGE_INX))
			{
				// unsigned token
				if ((val < 0) || (val > 255))
				{
					len = 2;
					if ((val < 0) || (val > 65535)) len = 3;
				}
			}
			else
			{
				// signed token
				if ((val > 127) || (val < -127))
				{
					len = 2;
					if ((val > 32767) || (val < -32767)) len = 3;
				}
			}
		}

		// always parameter
		if ((token >= HID_INPUT_INX) && (len == 0)) len = 1;

		// print HEX code
		OutHex(pref | len);

		// print data
		if (len > 0)
		{
			OutChar(32);
			OutHex((u8)val);
			if (len > 1)
			{
				OutChar(32);
				OutHex((u8)(val >> 8));
				if (len > 2)
				{
					OutChar(32);
					OutHex((u8)(val >> 16));
					OutChar(32);
					OutHex((u8)(val >> 24));
				}
			}
		}

		// print comment
		if (token >= 0)
		{
			// decrease level
			if ((token == HID_COLLECTION_END_INX) && (level > 0)) level--;

			// comment delimiter
			if (len == 0) OutChar(9);
			if (len < 2) OutChar(9);
			OutTxt("\t//\t");

			// print indent
			for (j = 0; j < level; j++) OutChar(9);

			// print token
			OutTxt(TokenPrefix[token].name);

			// token size
			if (len != 1)
			{
				if ((len != 0) || (token >= HID_INPUT_INX)) OutNum(len);
			}

			// print parameter
			if (len > 0)
			{
				OutTxt(" ( ");

				// input, output, feature
				if ((token == HID_INPUT_INX) || (token == HID_OUTPUT_INX) || (token == HID_FEATURE_INX))
				{
					OutTxt(((val & B0) != 0) ? "HID_CONSTANT" : "HID_DATA");
					if ((val & B0) != 0) // constant
					{
						if ((val & B1) != 0) OutTxt(" | HID_VARIABLE");
						if ((val & B2) != 0) OutTxt(" | HID_RELATIVE");
					}
					else // data
					{
						OutTxt(((val & B1) != 0) ? " | HID_VARIABLE" : " | HID_ARRAY");
						OutTxt(((val & B2) != 0) ? " | HID_RELATIVE" : " | HID_ABSOLUTE");
					}
					if ((val & B3) != 0) OutTxt(" | HID_WRAP");
					if ((val & B4) != 0) OutTxt(" | HID_NON_LINEAR");
					if ((val & B5) != 0) OutTxt(" | HID_NO_PREFERRED");
					if ((val & B6) != 0) OutTxt(" | HID_NULL_STATE");
					if ((val & B7) != 0) OutTxt(" | HID_VOLATILE");
					if ((val & B8) != 0) OutTxt(" | HID_BUFFERED_BYTES");
					val &= ~0x1ff;
					if (val != 0) fprintf(FileOut, "| 0x%X", val);
				}

				// collection
				else if (token == HID_COLLECTION_INX)
				{
					int token2 = FindVal(TokenColl, count_of(TokenColl), val);
					if (token2 >= 0)
						OutTxt(TokenColl[token2].name);
					else
						OutNum(val);
				}

				// usage page
				else if (token == HID_USAGE_PAGE_INX)
				{
					int token2 = FindVal(TokenPage, count_of(TokenPage), val);
					if (token2 >= 0)
					{
						lastpage = token2; // last usage page
						OutTxt(TokenPage[token2].name);
					}
					else
						OutNum(val);
				}

				// usage
				else if (token == HID_USAGE_INX)
				{
					// desktop page
					if (lastpage == HID_USAGE_PAGE_DESKTOP_INX)
					{
						int token2 = FindVal(TokenDesktop, count_of(TokenDesktop), val);
						if (token2 >= 0)
							OutTxt(TokenDesktop[token2].name);
						else
							OutNum(val);
					}
					// consumer page
					else if (lastpage == HID_USAGE_PAGE_CONSUMER_INX)
					{
						int token2 = FindVal(TokenConsumer, count_of(TokenConsumer), val);
						if (token2 >= 0)
							OutTxt(TokenConsumer[token2].name);
						else
							OutNum(val);
					}
					// FIDO page
					else if (lastpage == HID_USAGE_PAGE_FIDO_INX)
					{
						int token2 = FindVal(TokenFido, count_of(TokenFido), val);
						if (token2 >= 0)
							OutTxt(TokenFido[token2].name);
						else
							OutNum(val);
					}
					// other pages
					else
						OutNum(val);
				}

				// otherwise
				else
					OutNum(val);

				OutTxt(" )");
			}
			OutChar(',');

			// increase level
			if (token == HID_COLLECTION_INX) level++;
		}

		// end of line
		fprintf(FileOut, "\n");

		// next token
		r++;
	}

	// close output file
	fclose(FileOut);
	return 0;
}
