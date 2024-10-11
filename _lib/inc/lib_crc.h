
// ****************************************************************************
//
//                                    CRC
//                          Cyclic Redundancy Check
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

// CRC-64
// CRC-32 Normal and Reversed
// CRC-16 CCITT Normal, Reversed and Normal Alternative
// CRC-8 Dallas
// Parity
// Simple checksum on 8-bit, 16-bit and 32-bit data
// CRC-XOR

#if USE_CRC	// use CRC Check Sum (lib_crc.c, lib_crc.h)

#ifndef _LIB_CRC_H
#define _LIB_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

// check patterns
#define CRCPAT1LEN 9	// length of CRC pattern 1
extern const char ALIGNED CrcPat1[];

#define CRCPAT2LEN 3	// length of CRC pattern 2
extern const u8 ALIGNED CrcPat2[CRCPAT2LEN+1];

#define CRCPAT3LEN 1024	// length of CRC pattern 3
#define CrcPat3 Crc32BTable

// check all CRC functions (returns False on error)
Bool CrcCheck();

// ============================================================================
//                         SHA256 hash (256-bit checksum)
// ============================================================================

//  Sample "123456789" -> 0x15,0xe2,0xb0,0xd3,0xc3,0x38,0x91,0xeb,0xb0,0xf1,0xef,0x60,0x9e,0xc4,0x19,0x42,0x0c,0x20,0xe3,0x20,0xce,0x94,0xc6,0x5f,0xbc,0x8c,0x33,0x12,0x44,0x8e,0xb2,0x25
//  Sample 0xFC 0x05 0x4A -> 0xc5,0x52,0x39,0xc3,0xfd,0xc5,0x22,0xad,0x47,0x45,0xba,0x78,0x0c,0x35,0xcf,0xed,0xc5,0xe2,0x50,0x9d,0xfc,0x28,0xe5,0x08,0x7b,0x5a,0x20,0xfa,0x37,0xae,0xac,0xf9
//  Sample CRC32BTable 1KB -> 0x12,0xf3,0xe0,0x57,0x6d,0x44,0x7e,0xb3,0x7b,0x36,0xd8,0x2b,0xa0,0xc1,0xc5,0x48,0x1b,0x8f,0x0d,0x12,0xfd,0xc7,0x03,0x47,0xce,0x4a,0x07,0x6b,0x22,0x9d,0x4c,0x86

#define SHA256_HASH_SIZE	32	// SHA256 hash size in number of bytes

// SHA256 context
typedef struct {
	u64	datalen;	// size of input in bytes
	union {
		u32	state[8];	// current accumulation of the hash
		u8	result[32];	// hash result
	};
	u8	data[64];	// input buffer
} SHA256_Context;

// initialize SHA256 context
void SHA256_Init(SHA256_Context* ctx);

// add buffer to SHA256 accumulator
void SHA256_AddBuf(SHA256_Context* ctx, const void* buf, int len);

// add byte to SHA256 accumulator
void SHA256_AddByte(SHA256_Context* ctx, u8 data);

// finalize (ctx->buffer = hash result)
// - input data will be aligned to 512-bit boundary, with end header
void SHA256_Final(SHA256_Context* ctx);

// calculace SHA256 hash (dst = pointer to destination 32-byte buffer)
// - input data will be aligned to 512-bit boundary, with end header
//   Calculation speed: 1580 us per 1 KB
void SHA256_Calc(u8* dst, const void* src, int len);

// check SHA256 hash calculations (returns False on error)
Bool SHA256_Check(void);

// ============================================================================
//                         MD5 hash (128-bit checksum)
// ============================================================================
//  Sample "123456789" -> 0x25,0xf9,0xe7,0x94,0x32,0x3b,0x45,0x38,0x85,0xf5,0x18,0x1f,0x1b,0x62,0x4d,0x0b
//  Sample 0xFC 0x05 0x4A -> 0x4f,0x58,0xb5,0x2f,0xf8,0x00,0x09,0x60,0xfd,0xce,0x70,0x63,0x2a,0xb7,0x04,0x53
//  Sample CRC32BTable 1KB -> 0x11,0x3b,0x12,0xab,0xbc,0x21,0x2d,0xae,0x31,0xc2,0xa6,0xc7,0xb4,0x07,0x6c,0x19

#if USE_MD5	// use MD5 hash - may need to be disabled when colliding with BTStack

#define MD5_HASH_SIZE	16	// MD5 hash size in number of bytes

// MD5 context
typedef struct {
	u64	size;		// size of input in bytes
	u32	buffer[4];	// current accumulation of the hash, hash result
	union {
		u8	input[64];	// input buffer
		u32	input32[16];
	};
} MD5_Context;

// initialize MD5 context
void MD5_Init(MD5_Context* ctx);

// add buffer to MD5 accumulator
void MD5_AddBuf(MD5_Context* ctx, const void* buf, int len);

// add byte to MD5 accumulator
void MD5_AddByte(MD5_Context* ctx, u8 data);

// finalize (ctx->buffer = hash result)
// - input data will be aligned to 512-bit boundary, with end header
void MD5_Final(MD5_Context* ctx);

// calculace MD5 hash (dst = pointer to destination 16-byte buffer)
// - input data will be aligned to 512-bit boundary, with end header
//   Calculation speed: 864 us per 1 KB
void MD5_Calc(u8* dst, const void* src, int len);

// check MD5 hash calculations (returns False on error)
Bool MD5_Check(void);

#endif // USE_MD5

// ============================================================================
//                           CRC-64 Normal (CRC64A)
// ============================================================================
// Polynom: x64 + x62 + x57 + x55 + x54 + x53 + x52 + x47 + x46 + x45 + x40 +
//          x39 + x38 + x37 + x35 + x33 + x32 + x31 + x29 + x27 + x24 + x23 +
//          x22 + x21 + x19 + x17 + x13 + x12 + x10 + x9 + x7 + x4 + x + 1 
//
// CRC-64-ECMA, ECMA-182
// CRC-64 Normal (CRC64A, polynomial 0x42F0E1EBA9EA3693), init word 0xFFFFFFFFFFFFFFFF:
//  Sample "123456789" -> 0x9D13A61C0E5B0FF5ull
//  Sample 0xFC 0x05 0x4A -> 0x21E4F88DB2978548ull
//  Sample CRC32BTable 1KB -> 0x378302BE2C61CF9Eull

// CRC-64 Normal (CRC64A) table (2 KB)
extern const u64 Crc64ATable[256];

// CRC-64 Normal (CRC64A) polynomial
#define CRC64A_POLY 0x42F0E1EBA9EA3693ULL

// CRC-64 Normal (CRC64A) init word (not reversed, not inverted)
#define CRC64A_INIT 0xFFFFFFFFFFFFFFFFULL

// Check CRC-64 Normal (CRC64A) table (returns False on error; can be used to generate table)
Bool Crc64ATableCheck();

// Calculate CRC-64 Normal (CRC64A), 1 byte - tabled version (requires 2 KB of flash memory)
u64 Crc64AByteTab(u64 crc, u8 data);

// Calculate CRC-64 Normal (CRC64A), 1 byte - slow version
u64 Crc64AByteSlow(u64 crc, u8 data);

// Calculate CRC-64 Normal (CRC64A), buffer - tabled version (requires 2 KB of flash memory)
u64 Crc64ABufTab(u64 crc, const void* buf, int len);

// Calculate CRC-64 Normal (CRC64A), buffer - slow version
u64 Crc64ABufSlow(u64 crc, const void* buf, int len);

// Calculate CRC-64 Normal (CRC64A) - tabled version (requires 2 KB of flash memory)
//   Calculation speed: 200 us per 1 KB
u64 Crc64ATab(const void* buf, int len);

// Calculate CRC-64 Normal (CRC64A) - slow version
//   Calculation speed: 1000 us per 1 KB
u64 Crc64ASlow(const void* buf, int len);

// Check CRC-64 Normal (CRC64A) calculations (returns False on error)
Bool Crc64ACheck(void);

// ============================================================================
//                           CRC-64 Reversed (CRC64B)
// ============================================================================
// CRC-64-ECMA, ECMA-182
// CRC-64 Reversed (CRC64B, polynomial 0xC96C5795D7870F42), init word 0:
//  Sample "123456789" -> 0x995DC9BBDF1939FAull
//  Sample 0xFC 0x05 0x4A -> 0xF10B3B2CDEF45CFAull
//  Sample CRC32BTable 1KB -> 0xACB732293EDC5DC8ull
// ... this is preferred CRC-64 method

// CRC-64 Reversed (CRC64B) table (2 KB)
extern const u64 Crc64BTable[256];

// CRC-64 Reversed (CRC64B) polynomial
#define CRC64B_POLY 0xC96C5795D7870F42ULL

// CRC-64 Reversed (CRC64B) init word (not reversed, not inverted)
#define CRC64B_INIT 0

// Check CRC-64 Reversed (CRC64B) table (returns False on error; can be used to generate table)
Bool Crc64BTableCheck();

// Calculate CRC-64 Reversed (CRC64B), 1 byte - tabled version (requires 2 KB of flash memory)
u64 Crc64BByteTab(u64 crc, u8 data);

// Calculate CRC-64 Reversed (CRC64B), 1 byte - slow version
u64 Crc64BByteSlow(u64 crc, u8 data);

// Calculate CRC-64 Reversed (CRC64B), buffer - tabled version (requires 2 KB of flash memory)
u64 Crc64BBufTab(u64 crc, const void* buf, int len);

// Calculate CRC-64 Reversed (CRC64B), buffer - slow version
u64 Crc64BBufSlow(u64 crc, const void* buf, int len);

// Calculate CRC-64 Reversed (CRC64B) - tabled version (requires 2 KB of flash memory)
//   Calculation speed: 220 us per 1 KB
u64 Crc64BTab(const void* buf, int len);

// Calculate CRC-64 Reversed (CRC64B) - slow version
//   Calculation speed: 2200 us per 1 KB
u64 Crc64BSlow(const void* buf, int len);

// Check CRC-64 Reversed (CRC64B) calculations (returns False on error)
Bool Crc64BCheck(void);

// ============================================================================
//                            CRC-64 Common
// ============================================================================
// Default CRC-64 functions: CRC-64 Reversed CRC64B
//  Sample "123456789" -> 0x995DC9BBDF1939FAull
//  Sample 0xFC 0x05 0x4A -> 0xF10B3B2CDEF45CFAull
//  Sample CRC32BTable 1KB -> 0xACB732293EDC5DC8ull

// CRC-64 init word
#define CRC64_INIT CRC64B_INIT

// Calculate CRC-64, 1 byte
INLINE u64 Crc64Byte(u64 crc, u8 data) { return Crc64BByteSlow(crc, data); }

// Calculate CRC-64, buffer
INLINE u64 Crc64Buf(u32 crc, const void* buf, int len) { return Crc64BBufTab(crc, buf, len); }
 
// Calculate CRC-64
//   Calculation speed: 220 us per 1 KB
INLINE u64 Crc64(const void* buf, int len) { return Crc64BTab(buf, len); }

// ============================================================================
//                           CRC-32 Normal (CRC32A)
// ============================================================================
// Polynom: x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1
// in BIN: 0b0000 0100 1100 0001 0001 1101 1011 0111 (x32 is omitted, CRC is 32-bit long)
// in HEX: 0x04C11DB7
// reversed HEX: 0xEDB88320

// CRC-32 is most commonly used in 2 variants - normal (CRC32A) and reversed (CRC32B).
// Normal variant (CRC32A, older ITU I.363.5 algorithm) is used as checksum
//   of boot loader stage 2 in Raspberry Pico, MPEG-2 and BZIP2 (BZIP2 with inverted result).
// Reversed variant (CRC32B, later ITU V.42 algorithm) us used in Ethernet, FDDI, Gzip and PKZip.

// >>> It is recommended to prefer reversed variant CRC32B, which is more commonly used. <<<

// CRC-32 Normal (CRC32A, polynomial 0x04C11DB7), init word 0xFFFFFFFF:
//  Sample "123456789" -> 0x0376E6E7
//  Sample 0xFC 0x05 0x4A -> 0x8E10D720
//  Sample CRC32BTable 1KB -> 0x5796333D

// CRC-32 Normal (CRC32A) table (1 KB)
extern const u32 Crc32ATable[256];

// CRC-32 Normal (CRC32A) polynomial
#define CRC32A_POLY 0x04C11DB7

// CRC-32 Normal (CRC32A) init word (not reversed, not inverted)
#define CRC32A_INIT 0xFFFFFFFF

// Check CRC-32 Normal (CRC32A) table (returns False on error; can be used to generate table)
Bool Crc32ATableCheck(void);

// Calculate CRC-32 Normal (CRC32A), 1 byte - tabled version (requires 1 KB of flash memory)
u32 Crc32AByteTab(u32 crc, u8 data);

// Calculate CRC-32 Normal (CRC32A), 1 byte - slow version
u32 Crc32AByteSlow(u32 crc, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32AByteDMA(u32 crc, u8 data);
#endif // USE_DMA

// Calculate CRC-32 Normal (CRC32A), buffer - tabled version (requires 1 KB of flash memory)
u32 Crc32ABufTab(u32 crc, const void* buf, int len);

// Calculate CRC-32 Normal (CRC32A), buffer - slow version
u32 Crc32ABufSlow(u32 crc, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32ABufDMA(u32 crc, const void* buf, int len);
#endif // USE_DMA

// Calculate CRC-32 Normal (CRC32A) - tabled version (requires 1 KB of flash memory)
//   Calculation speed: 160 us per 1 KB
u32 Crc32ATab(const void* buf, int len);

// Calculate CRC-32 Normal (CRC32A) - slow version
//   Calculation speed: 790 us per 1 KB
u32 Crc32ASlow(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32ADMA(const void* buf, int len);
#endif // USE_DMA

// Check CRC-32 Normal (CRC32A) calculations (returns False on error)
Bool Crc32ACheck(void);

// ============================================================================
//                          CRC-32 Reversed (CRC32B)
// ============================================================================
// CRC-32 Reversed (CRC32B, polynomial 0xEDB88320), init word 0xFFFFFFFF (we use non-inverted form 0):
//  Sample "123456789" -> 0xCBF43926
//  Sample 0xFC 0x05 0x4A -> 0xA8E10F6D
//  Sample CRC32BTable 1KB -> 0x6FCF9E13

// CRC-32 Reversed (CRC32B) table (1 KB)
extern const u32 Crc32BTable[256];

// CRC-32 Reversed (CRC32B) polynomial
#define CRC32B_POLY 0xEDB88320

// CRC-32 Reversed (CRC32B) init word (reversed bits, not inverted)
// This initialization word is commonly given in its inverted form, as 0xFFFFFFFF.
#define CRC32B_INIT 0

// Check CRC-32 Reversed (CRC32B) table (returns False on error; can be used to generate table)
Bool Crc32BTableCheck(void);

// Calculate CRC-32 Reversed (CRC32B), 1 byte - tabled version (requires 1 KB of flash memory)
u32 Crc32BByteTab(u32 crc, u8 data);

// Calculate CRC-32 Reversed (CRC32B), 1 byte - slow version
u32 Crc32BByteSlow(u32 crc, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BByteDMA(u32 crc, u8 data);
#endif // USE_DMA

// Calculate CRC-32 Reversed (CRC32B), buffer - tabled version (requires 1 KB of flash memory)
u32 Crc32BBufTab(u32 crc, const void* buf, int len);

// Calculate CRC-32 Reversed (CRC32B), buffer - slow version
u32 Crc32BBufSlow(u32 crc, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BBufDMA(u32 crc, const void* buf, int len);
#endif // USE_DMA

// Calculate CRC-32 Reversed (CRC32B) - tabled version (requires 1 KB of flash memory)
//   Calculation speed: 160 us per 1 KB
u32 Crc32BTab(const void* buf, int len);

// Calculate CRC-32 Reversed (CRC32B) - slow version
//   Calculation speed: 900 us per 1 KB
u32 Crc32BSlow(const void* buf, int len); 

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BDMA(const void* buf, int len); 
#endif // USE_DMA

// Check CRC-32 Reversed (CRC32B) calculations (returns False on error)
Bool Crc32BCheck(void);

// ============================================================================
//                            CRC-32 Common
// ============================================================================
// Default CRC-32 functions: CRC-32 Reversed CRC32B with DMA preferred
//  Sample "123456789" -> 0xCBF43926
//  Sample 0xFC 0x05 0x4A -> 0xA8E10F6D
//  Sample CRC32BTable 1KB -> 0x6FCF9E13

// CRC-32 init word
#define CRC32_INIT CRC32B_INIT

// Calculate CRC-32, 1 byte
INLINE u32 Crc32Byte(u32 crc, u8 data) { return Crc32BByteSlow(crc, data); }

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32, buffer (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u32 Crc32Buf(u32 crc, const void* buf, int len) { return Crc32BBufDMA(crc, buf, len); }
 
// Calculate CRC-32 (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u32 Crc32(const void* buf, int len) { return Crc32BDMA(buf, len); }

#else // USE_DMA
// Calculate CRC-32, buffer
INLINE u32 Crc32Buf(u32 crc, const void* buf, int len) { return Crc32BBufTab(crc, buf, len); }
 
// Calculate CRC-32
//   Calculation speed: 160 us per 1 KB
INLINE u32 Crc32(const void* buf, int len) { return Crc32BTab(buf, len); }
#endif // USE_DMA

// ============================================================================
//                       CRC-16 CCITT Normal (CRC16A)
// ============================================================================
// Polynom: x16 + x12 + x5 + 1
// in BIN: 0b0001 0000 0010 0001 (x16 is omitted, CRC is 16-bit long)
// in HEX: 0x1021
// reversed HEX: 0x8408

// CRC-16 CCITT is used in X.25, V.41, HDLC, XMODEM, BlueTooth, SD card ant many others.
// >>> It is recommended to prefer reversed variant CRC16C, which is more commonly used. <<<

// CRC-16 CCITT Normal (CRC16A, polynomial 0x1021), init word 0xFFFF
//  Sample "123456789" -> 0x29B1
//  Sample: 0xFC 0x05 0x4A -> 0x4CD4
//  Sample CRC32 1KB -> 0x4EED

// CRC-16 CCITT Normal (CRC16A) table (512 B)
extern const u16 Crc16ATable[256];

// CRC-16 CCITT Normal (CRC16A) polynomial
#define CRC16A_POLY 0x1021

// CRC-16 CCITT Normal (CRC16A) init word (not reversed, not inverted)
#define CRC16A_INIT 0xFFFF

// Check CRC-16 CCITT Normal (CRC16A) table (returns False on error; can be used to generate table)
Bool Crc16ATableCheck(void);

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - tabled version (requires 512 B of flash memory)
u16 Crc16AByteTab(u16 crc, u8 data);

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - fast version
u16 Crc16AByteFast(u16 crc, u8 data);

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - slow version
u16 Crc16AByteSlow(u16 crc, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16AByteDMA(u16 crc, u8 data);
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - tabled version (requires 512 B of flash memory)
u16 Crc16ABufTab(u16 crc, const void* buf, int len);

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - fast version
u16 Crc16ABufFast(u16 crc, const void* buf, int len);

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - slow version
u16 Crc16ABufSlow(u16 crc, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16ABufDMA(u16 crc, const void* buf, int len);
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal (CRC16A) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 170 us per 1 KB
u16 Crc16ATab(const void* buf, int len);

// Calculate CRC-16 CCITT Normal (CRC16A) - fast version
//   Calculation speed: 200 us per 1 KB
u16 Crc16AFast(const void* buf, int len);

// Calculate CRC-16 CCITT Normal (CRC16A) - slow version
//   Calculation speed: 1000 us per 1 KB
u16 Crc16ASlow(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16ADMA(const void* buf, int len);
#endif // USE_DMA

// Check CRC-16 CCITT Normal (CRC16A) calculations (returns False on error)
Bool Crc16ACheck(void);

// ============================================================================
//                       CRC-16 CCITT Reversed (CRC16B)
// ============================================================================
// CRC-16 CCITT Reversed (CRC16B, polynomial 0x8408), init word 0xFFFF
//  Sample "123456789" -> 0x906E
//  Sample: 0xFC 0x05 0x4A -> 0x7CBD
//  Sample CRC32 1KB -> 0x0C42

// CRC-16 CCITT Reversed (CRC16B) table (512 B)
extern const u16 Crc16BTable[256];

// CRC-16 CCITT Reversed (CRC16B) polynomial
#define CRC16B_POLY 0x8408

// CRC-16 CCITT Reversed (CRC16B) init word (reversed bits, not inverted)
// This initialization word is commonly given in its inverted form, as 0xFFFF.
#define CRC16B_INIT 0

// Check CRC-16 CCITT Reversed (CRC16B) table (returns False on error; can be used to generate table)
Bool Crc16BTableCheck(void);

// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - tabled version (requires 512 B of flash memory)
u16 Crc16BByteTab(u16 crc, u8 data);

// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - slow version
u16 Crc16BByteSlow(u16 crc, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BByteDMA(u16 crc, u8 data);
#endif // USE_DMA

// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - tabled version (requires 512 B of flash memory)
u16 Crc16BBufTab(u16 crc, const void* buf, int len);

// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - slow version
u16 Crc16BBufSlow(u16 crc, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BBufDMA(u16 crc, const void* buf, int len);
#endif // USE_DMA

// Calculate CRC-16 CCITT Reversed (CRC16B) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 160 us per 1 KB
u16 Crc16BTab(const void* buf, int len);

// Calculate CRC-16 CCITT Reversed (CRC16B) - slow version
//   Calculation speed: 880 us per 1 KB
u16 Crc16BSlow(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BDMA(const void* buf, int len);
#endif // USE_DMA

// Check CRC-16 CCITT Reversed (CRC16B) calculations (returns False on error)
Bool Crc16BCheck(void);

// ============================================================================
//                 CRC-16 CCITT Normal Alternative (CRC16C)
// ============================================================================
// CRC-16-CCITT Normal Alternative (CRC16C, polynomial 0x1021), init word 0x1D0F:
//  Sample "123456789" -> 0xE5CC
//  Sample: 0xFC 0x05 0x4A -> 0x9144
//  Sample CRC32 1KB -> 0xE351

// CRC-16 CCITT Normal Alternative (CRC16C) init word (not reversed, not inverted)
#define CRC16C_INIT 0x1D0F

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), 1 byte - tabled version (requires 512 B of flash memory)
INLINE u16 Crc16CByteTab(u16 crc, u8 data) { return Crc16AByteTab(crc, data); }

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), 1 byte - fast version
INLINE u16 Crc16CByteFast(u16 crc, u8 data) { return Crc16AByteFast(crc, data); }

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), 1 byte - slow version
INLINE u16 Crc16CByteSlow(u16 crc, u8 data) { return Crc16AByteSlow(crc, data); }

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal Alternative (CRC16C), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u16 Crc16CByteDMA(u16 crc, u8 data) { return Crc16AByteDMA(crc, data); }
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), buffer - tabled version (requires 512 B of flash memory)
INLINE u16 Crc16CBufTab(u16 crc, const void* buf, int len) { return Crc16ABufTab(crc, buf, len); }

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), buffer - fast version
INLINE u16 Crc16CBufFast(u16 crc, const void* buf, int len) { return Crc16ABufFast(crc, buf, len); }

// Calculate CRC-16 CCITT Normal Alternative (CRC16C), buffer - slow version
INLINE u16 Crc16CBufSlow(u16 crc, const void* buf, int len) { return Crc16ABufSlow(crc, buf, len); }

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal Alternative (CRC16C), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u16 Crc16CBufDMA(u16 crc, const void* buf, int len) { return Crc16ABufDMA(crc, buf, len); }
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 170 us per 1 KB
u16 Crc16CTab(const void* buf, int len);

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - fast version
//   Calculation speed: 200 us per 1 KB
u16 Crc16CFast(const void* buf, int len);

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - slow version
//   Calculation speed: 1000 us per 1 KB
u16 Crc16CSlow(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16CDMA(const void* buf, int len);
#endif // USE_DMA

// Check CRC-16 CCITT Normal Alternative (CRC16C) calculations (returns False on error)
Bool Crc16CCheck(void);

// ============================================================================
//                            CRC-16 Common
// ============================================================================
// Default CRC-16 functions: CRC-16 CCITT Normal Alternative CRC16C with DMA preferred
//  Sample "123456789" -> 0xE5CC
//  Sample: 0xFC 0x05 0x4A -> 0x9144
//  Sample CRC32 1KB -> 0xE351

// CRC-16 init word
#define CRC16_INIT CRC16C_INIT

// Calculate CRC-16, 1 byte
INLINE u32 Crc16Byte(u32 crc, u8 data) { return Crc16CByteFast(crc, data); }

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16, buffer (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u32 Crc16Buf(u32 crc, const void* buf, int len) { return Crc16CBufDMA(crc, buf, len); }
 
// Calculate CRC-16 (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
INLINE u32 Crc16(const void* buf, int len) { return Crc16CDMA(buf, len); }
#else // USE_DMA

// Calculate CRC-16, buffer
INLINE u32 Crc16Buf(u32 crc, const void* buf, int len) { return Crc16CBufTab(crc, buf, len); }

// Calculate CRC-16 (does not need table)
//   Calculation speed: 200 us per 1 KB
INLINE u32 Crc16(const void* buf, int len) { return Crc16CFast(buf, len); }
#endif // USE_DMA

// ============================================================================
//                            CRC-8 Dallas
// ============================================================================
// Polynom: x^8 + x^5 + x^4 + 1
// in BIN: 0b0011 0001 (x8 is omitted, CRC is 8-bit long)
// in HEX: 0x31
// reversed HEX: 0x8C

// Used with Dallas Maxim 1-Wire bus

// CRC-8 Dallas, init word 0:
//  Sample "123456789" -> 0xA1
//  Sample: 0xFC 0x05 0x4A -> 0xF1
//  Sample CRC32 1KB -> 0x1E

// CRC-8 table (256 B)
extern const u8 Crc8Table[256];

// CRC-8 polynomial
#define CRC8_POLY 0x8C

// CRC-8 init word
#define CRC8_INIT 0

// Check CRC-8 table (returns False on error; can be used to generate table)
Bool Crc8TableCheck(void);

// Calculate CRC-8, 1 byte - tabled version (requires 256 B of flash memory)
u8 Crc8ByteTab(u8 crc, u8 data);

// Calculate CRC-8, 1 byte - slow version
u8 Crc8ByteSlow(u8 crc, u8 data);

// Calculate CRC-8, buffer - tabled version (requires 256 B of flash memory)
u8 Crc8BufTab(u8 crc, const void* buf, int len);

// Calculate CRC-8, buffer - slow version
u8 Crc8BufSlow(u8 crc, const void* buf, int len);

// Calculate CRC-8 - tabled version (requires 256 B of flash memory)
//   Calculation speed: 115 us per 1 KB
u8 Crc8Tab(const void* buf, int len);

// Calculate CRC-8 - slow version
//   Calculation speed: 820 us per 1 KB
u8 Crc8Slow(const void* buf, int len);

// Check CRC-8 calculations (returns False on error)
Bool Crc8Check(void);

// === Common functions (used tabled versions)

// Calculate CRC-8, 1 byte
INLINE u8 Crc8Byte(u8 crc, u8 data) { return Crc8ByteTab(crc, data); }

// Calculate CRC-8, buffer
INLINE u8 Crc8Buf(u8 crc, const void* buf, int len) { return Crc8BufTab(crc, buf, len); }

// Calculate CRC-8
//   Calculation speed: 115 us per 1 KB
INLINE u8 Crc8(const void* buf, int len) { return Crc8Tab(buf, len); }

// ============================================================================
//                                   Parity
// ============================================================================
// Parity returns value 1 if number of bits "1" is odd. If number of bits is even, it returns 0.
// Sample "123456789" -> 0x01
// Sample: 0xFC 0x05 0x4A -> 0x01
// Sample CRC32 1KB -> 0x00

// parity init value
#define PARITY_INIT	0

// Calculate parity, 1 byte - software version
u8 ParityByteSoft(u8 par, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity, 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u8 ParityByteDMA(u8 par, u8 data);
#endif // USE_DMA

// Calculate parity, buffer - software version
u8 ParityBufSoft(u8 par, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u8 ParityBufDMA(u8 par, const void* buf, int len);
#endif // USE_DMA

// Calculate parity - software version
//   Calculation speed: 90 us per 1 KB
u8 ParitySoft(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   Calculation speed: 2 us per 1 KB
u8 ParityDMA(const void* buf, int len);
#endif // USE_DMA

// check parity calculation (returns False on error)
Bool ParityCheck(void);

// ============================================================================
//            Simple checksum on 8-bit data with 32-bit result
// ============================================================================
// Sample "123456789" -> 0x000001DD
// Sample: 0xFC 0x05 0x4A -> 0x0000014B
// Sample CRC32 1KB -> 0x0001FE00

// 8-bit checksum init word
#define SUM8_INIT 0

// Calculate 8-bit checksum, 1 byte - software version
u32 Sum8ByteSoft(u32 sum, u8 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum, 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum8ByteDMA(u32 sum, u8 data);
#endif // USE_DMA

// Calculate 8-bit checksum, buffer - software version
u32 Sum8BufSoft(u32 sum, const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum8BufDMA(u32 sum, const void* buf, int len);
#endif // USE_DMA

// Calculate 8-bit checksum - software version
//   Calculation speed: 100 us per 1 KB
u32 Sum8Soft(const void* buf, int len);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   Calculation speed: 8 us per 1 KB
u32 Sum8DMA(const void* buf, int len);
#endif // USE_DMA

// check 8-bit checksum calculation (returns False on error)
Bool Sum8Check(void);

// ============================================================================
//            Simple checksum on 16-bit data with 32-bit result
// ============================================================================
//  Sample "12345678" -> 0x0000D4D0
//  Sample 0xFC 0x05 0x4A 0x3E -> 0x00004446
//  Sample CRC32 1KB -> 0x00FFFF00

// 16-bit checksum init word
#define SUM16_INIT 0

// Calculate 16-bit checksum, 1 word - software version
u32 Sum16WordSoft(u32 sum, u16 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum, 1 word - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum16WordDMA(u32 sum, u16 data);
#endif // USE_DMA

// Calculate 16-bit checksum, buffer - software version
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
u32 Sum16BufSoft(u32 sum, const u16* buf, int num);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum16BufDMA(u32 sum, const u16* buf, int num);
#endif // USE_DMA

// Calculate 16-bit checksum - software version
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Calculation speed: 48 us per 1 KB
u32 Sum16Soft(const u16* buf, int num);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Calculation speed: 4 us per 1 KB
u32 Sum16DMA(const u16* buf, int num);
#endif // USE_DMA

// check 16-bit checksum calculation (returns False on error)
Bool Sum16Check(void);

// ============================================================================
//            Simple checksum on 32-bit data with 32-bit result
// ============================================================================
//  Sample "12345678" -> 0x6C6A6866
//  Sample 0xFC 0x05 0x4A 0x3E -> 0x3E4A05FC
//  Sample CRC32 1KB -> 0xFFFFFF80

// 32-bit checksum init word
#define SUM32_INIT 0

// Calculate 32-bit checksum, 1 double word - software version
u32 Sum32DWordSoft(u32 sum, u32 data);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum, 1 double word - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum32DWordDMA(u32 sum, u32 data);
#endif // USE_DMA

// Calculate 32-bit checksum, buffer - software version
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
u32 Sum32BufSoft(u32 sum, const u32* buf, int num);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum32BufDMA(u32 sum, const u32* buf, int num);
#endif // USE_DMA

// Calculate 32-bit checksum - software version
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Calculation speed: 22 us per 1 KB
u32 Sum32Soft(const u32* buf, int num);

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Calculation speed: 2 us per 1 KB
u32 Sum32DMA(const u32* buf, int num);
#endif // USE_DMA

// check 32-bit checksum calculation (returns False on error)
Bool Sum32Check(void);

// ============================================================================
//                                  CRC-XOR
// ============================================================================
// Simple fast method, but better than simple sum, used to check EEPROMs in Robotron computers.
// Sample "123456789" -> 0x2035
// Sample: 0xFC 0x05 0x4A -> 0x03B0
// Sample CRC32 1KB -> 0x0000

// CRC-XOR init word
#define CRCXOR_INIT 0

// Calculate CRC-XOR, 1 byte
u16 CrcXorByte(u16 crc, u8 data);

// Calculate CRC-XOR, buffer
u16 CrcXorBuf(u16 crc, const void* buf, int len);

// Calculate CRC-XOR
//   Calculation speed: 160 us per 1 KB
u16 CrcXor(const void* buf, int len);

// check Xor checksum calculation (returns False on error)
Bool CrcXorCheck(void);

#ifdef __cplusplus
}
#endif

#endif // _LIB_CRC_H

#endif // USE_CRC	// use CRC Check Sum (lib_crc.c, lib_crc.h)
