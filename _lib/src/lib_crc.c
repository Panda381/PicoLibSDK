
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

#include "../../global.h"	// globals

#if USE_CRC	// use CRC Check Sum (lib_crc.c, lib_crc.h)

#include "../../_sdk/inc/sdk_dma.h"	// DMA
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_bootrom.h"
#include "../inc/lib_crc.h"

// check patterns
const char ALIGNED CrcPat1[] = "123456789";
const u8 ALIGNED CrcPat2[CRCPAT2LEN+1] = { 0xFC, 0x05, 0x4A, 0x3E };

// check all CRC functions (returns False on error)
Bool CrcCheck()
{
	// Check CRC-64 Normal (CRC64A) calculations (returns False on error)
	if (!Crc64ACheck()) return False;

	// Check CRC-64 Reversed (CRC64B) calculations (returns False on error)
	if (!Crc64BCheck()) return False;

	// Check CRC-32 Normal (CRC32A) calculations (returns False on error)
	if (!Crc32ACheck()) return False;

	// Check CRC-32 Reversed (CRC32B) calculations (returns False on error)
	if (!Crc32BCheck()) return False;

	// Check CRC-16 CCITT Normal (CRC16A) calculations (returns False on error)
	if (!Crc16ACheck()) return False;

	// Check CRC-16 CCITT Reversed (CRC16B) calculations (returns False on error)
	if (!Crc16BCheck()) return False;

	// Check CRC-16 CCITT Normal Alternative (CRC16C) calculations (returns False on error)
	if (!Crc16CCheck()) return False;

	// Check CRC-8 calculations (returns False on error)
	if (!Crc8Check()) return False;

	// check parity calculation (returns False on error)
	if (!ParityCheck()) return False;

	// check 8-bit checksum calculation (returns False on error)
	if (!Sum8Check()) return False;

	// check 16-bit checksum calculation (returns False on error)
	if (!Sum16Check()) return False;

	// check 32-bit checksum calculation (returns False on error)
	if (!Sum32Check()) return False;

	// check Xor checksum calculation (returns False on error)
	if (!CrcXorCheck()) return False;

#if USE_MD5	// use MD5 hash - may need to be disabled when colliding with BTStack
	// check MD5 hash calculations (returns False on error)
	if (!MD5_Check()) return False;
#endif

	// check SHA256 hash calculations (returns False on error)
	if (!SHA256_Check()) return False;

	return True;
}

// ============================================================================
//                         SHA256 hash (256-bit checksum)
// ============================================================================

// Rotates a 32-bit word left by n bits
INLINE static u32 SHA256_rotateRight(u32 x, u32 n)
{
	return Ror(x, (u8)n);
//	return (x >> n) | (x << (32 - n));
}

#define SHA256_CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x) (SHA256_rotateRight(x,2) ^ SHA256_rotateRight(x,13) ^ SHA256_rotateRight(x,22))
#define SHA256_EP1(x) (SHA256_rotateRight(x,6) ^ SHA256_rotateRight(x,11) ^ SHA256_rotateRight(x,25))
#define SHA256_SIG0(x) (SHA256_rotateRight(x,7) ^ SHA256_rotateRight(x,18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_rotateRight(x,17) ^ SHA256_rotateRight(x,19) ^ ((x) >> 10))

const u32 SHA256_k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

// initialize SHA256 context
void SHA256_Init(SHA256_Context* ctx)
{
	ctx->datalen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

// step of SHA256, after filling out a block of 512 bits
static void SHA256_Step(SHA256_Context* ctx, const u8* data)
{
	u32 a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; i++, j += 4)
	{
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	}

	for ( ; i < 64; i++)
	{
		m[i] = SHA256_SIG1(m[i - 2]) + m[i - 7] + SHA256_SIG0(m[i - 15]) + m[i - 16];
	}

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i)
	{
		t1 = h + SHA256_EP1(e) + SHA256_CH(e,f,g) + SHA256_k[i] + m[i];
		t2 = SHA256_EP0(a) + SHA256_MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

// add buffer to SHA256 accumulator
void SHA256_AddBuf(SHA256_Context* ctx, const void* buf, int len)
{
	// update length
	uint offset = (uint)(ctx->datalen & 0x3f);
	ctx->datalen += len;

	// load input data
	const u8* src = (const u8*)buf;
	for (; len > 0; len--)
	{
		// load next byte
		ctx->data[offset++] = *src++;

		// check 64-byte (512-bit) boundary 
		if (offset == 64)
		{
			// process this data block
			SHA256_Step(ctx, ctx->data);
			offset = 0;
		}
	}
}

// add byte to SHA256 accumulator
void SHA256_AddByte(SHA256_Context* ctx, u8 data)
{
	SHA256_AddBuf(ctx, &data, 1);
}

// finalize (ctx->result = hash result)
// - input data will be aligned to 512-bit boundary, with end header
void SHA256_Final(SHA256_Context* ctx)
{
	// get message size in bits
	u64 size = ctx->datalen * 8;

	// add terminating bit '1'
	SHA256_AddByte(ctx, 0x80);

	// add padding zeroes (8 byte reserve for message size)
	while ((ctx->datalen & 0x3f) != 0x40-8) SHA256_AddByte(ctx, 0);

	// add message size in bits
	SHA256_AddByte(ctx, (u8)(size >> 7*8));
	SHA256_AddByte(ctx, (u8)(size >> 6*8));
	SHA256_AddByte(ctx, (u8)(size >> 5*8));
	SHA256_AddByte(ctx, (u8)(size >> 4*8));
	SHA256_AddByte(ctx, (u8)(size >> 3*8));
	SHA256_AddByte(ctx, (u8)(size >> 2*8));
	SHA256_AddByte(ctx, (u8)(size >> 1*8));
	SHA256_AddByte(ctx, (u8)(size >> 0*8));

	// convert result to big endian
	int i;
	for (i = 0; i < 8; i++) ctx->state[i] = Endian(ctx->state[i]);
}

// calculace SHA256 hash (dst = pointer to destination 32-byte buffer)
// - input data will be aligned to 512-bit boundary, with end header
//   Calculation speed: 1580 us per 1 KB
void SHA256_Calc(u8* dst, const void* src, int len)
{
	// prepare SHA256 context
	SHA256_Context ctx;
	SHA256_Init(&ctx);

	// add data
	SHA256_AddBuf(&ctx, src, len);

	// finalize
	SHA256_Final(&ctx);

	// copy result hash
	memcpy(dst, ctx.result, SHA256_HASH_SIZE);
}

const u8 SHA256_hash1[SHA256_HASH_SIZE] = { 0x15,0xe2,0xb0,0xd3,0xc3,0x38,0x91,0xeb,0xb0,0xf1,0xef,0x60,0x9e,0xc4,0x19,0x42,0x0c,0x20,0xe3,0x20,0xce,0x94,0xc6,0x5f,0xbc,0x8c,0x33,0x12,0x44,0x8e,0xb2,0x25 };
const u8 SHA256_hash2[SHA256_HASH_SIZE] = { 0xc5,0x52,0x39,0xc3,0xfd,0xc5,0x22,0xad,0x47,0x45,0xba,0x78,0x0c,0x35,0xcf,0xed,0xc5,0xe2,0x50,0x9d,0xfc,0x28,0xe5,0x08,0x7b,0x5a,0x20,0xfa,0x37,0xae,0xac,0xf9 };
const u8 SHA256_hash3[SHA256_HASH_SIZE] = { 0x12,0xf3,0xe0,0x57,0x6d,0x44,0x7e,0xb3,0x7b,0x36,0xd8,0x2b,0xa0,0xc1,0xc5,0x48,0x1b,0x8f,0x0d,0x12,0xfd,0xc7,0x03,0x47,0xce,0x4a,0x07,0x6b,0x22,0x9d,0x4c,0x86 };

// check SHA256 hash calculations (returns False on error)
Bool SHA256_Check(void)
{
	u8 hash[SHA256_HASH_SIZE];

	// check pattern 1
	SHA256_Calc(hash, CrcPat1, CRCPAT1LEN);
	if (memcmp(hash, SHA256_hash1, SHA256_HASH_SIZE) != 0) return False;

	// check pattern 2
	SHA256_Calc(hash, CrcPat2, CRCPAT2LEN);
	if (memcmp(hash, SHA256_hash2, SHA256_HASH_SIZE) != 0) return False;

	// check pattern 3
	SHA256_Calc(hash, CrcPat3, CRCPAT3LEN);
	if (memcmp(hash, SHA256_hash3, SHA256_HASH_SIZE) != 0) return False;

	return True;
}

// ============================================================================
//                         MD5 hash (128-bit checksum)
// ============================================================================

#if USE_MD5	// use MD5 hash - may need to be disabled when colliding with BTStack

// rorate amounts table
const u8 MD5_S[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

const u32 MD5_K[] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

// init values
#define MD5_A 0x67452301
#define MD5_B 0xefcdab89
#define MD5_C 0x98badcfe
#define MD5_D 0x10325476

// functions
#define MD5_F(X, Y, Z) (((X) & (Y)) | (~(X) & (Z)))
#define MD5_G(X, Y, Z) (((X) & (Z)) | ((Y) & ~(Z)))
#define MD5_H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define MD5_I(X, Y, Z) ((Y) ^ ((X) | ~(Z)))

// Rotates a 32-bit word left by n bits
INLINE static u32 MD5_rotateLeft(u32 x, u32 n)
{
	return Rol(x, (u8)n);
//	return (x << n) | (x >> (32 - n));
}

// initialize MD5 context
void MD5_Init(MD5_Context* ctx)
{
	ctx->size = 0;
	ctx->buffer[0] = MD5_A;
	ctx->buffer[1] = MD5_B;
	ctx->buffer[2] = MD5_C;
	ctx->buffer[3] = MD5_D;
}

// step of MD5, after filling out a block of 512 bits
static void MD5_Step(u32* buffer, u32* input)
{
	u32 aa = buffer[0];
	u32 bb = buffer[1];
	u32 cc = buffer[2];
	u32 dd = buffer[3];
	u32 e, tmp;
	uint i, j;
	for (i = 0; i < 64; i++)
	{
		switch(i >> 4)
		{
		case 0:
			e = MD5_F(bb, cc, dd);
			j = i;
			break;

		case 1:
			e = MD5_G(bb, cc, dd);
			j = (i*5 + 1) & 0x0f;
			break;

		case 2:
			e = MD5_H(bb, cc, dd);
			j = (i*3 + 5) & 0x0f;
			break;

		default:
			e = MD5_I(bb, cc, dd);
			j = (i*7) & 0x0f;
			break;
		}

		tmp = dd;
		dd = cc;
		cc = bb;
		bb = bb + MD5_rotateLeft(aa + e + MD5_K[i] + input[j], MD5_S[i]);
		aa = tmp;
	}

	buffer[0] += aa;
	buffer[1] += bb;
	buffer[2] += cc;
	buffer[3] += dd;
}

// add buffer to MD5 accumulator
void MD5_AddBuf(MD5_Context* ctx, const void* buf, int len)
{
	// update length
	uint offset = (uint)(ctx->size & 0x3f);
	ctx->size += len;

	// load input data
	const u8* src = (const u8*)buf;
	for (; len > 0; len--)
	{
		// load next byte
		ctx->input[offset++] = *src++;

		// check 64-byte (512-bit) boundary 
		if (offset == 64)
		{
			// process this data block
			MD5_Step(ctx->buffer, ctx->input32);
			offset = 0;
		}
	}
}

// add byte to MD5 accumulator
void MD5_AddByte(MD5_Context* ctx, u8 data)
{
	MD5_AddBuf(ctx, &data, 1);
}

// finalize (ctx->buffer = hash result)
// - input data will be aligned to 512-bit boundary, with end header
void MD5_Final(MD5_Context* ctx)
{
	// get message size in bits
	u64 size = ctx->size * 8;

	// add terminating bit '1'
	MD5_AddByte(ctx, 0x80);

	// add padding zeroes (8 byte reserve for message size)
	while ((ctx->size & 0x3f) != 0x40-8) MD5_AddByte(ctx, 0);

	// add message size in bits
	MD5_AddBuf(ctx, &size, 8);
}

// calculace MD5 hash (dst = pointer to destination 16-byte buffer)
// - input data will be aligned to 512-bit boundary, with end header
//   Calculation speed: 864 us per 1 KB
void MD5_Calc(u8* dst, const void* src, int len)
{
	// prepare MD5 context
	MD5_Context ctx;
	MD5_Init(&ctx);

	// add data
	MD5_AddBuf(&ctx, src, len);

	// finalize
	MD5_Final(&ctx);

	// copy result hash
	memcpy(dst, ctx.buffer, MD5_HASH_SIZE);
}

const u8 MD5_hash1[MD5_HASH_SIZE] = { 0x25,0xf9,0xe7,0x94,0x32,0x3b,0x45,0x38,0x85,0xf5,0x18,0x1f,0x1b,0x62,0x4d,0x0b };
const u8 MD5_hash2[MD5_HASH_SIZE] = { 0x4f,0x58,0xb5,0x2f,0xf8,0x00,0x09,0x60,0xfd,0xce,0x70,0x63,0x2a,0xb7,0x04,0x53 };
const u8 MD5_hash3[MD5_HASH_SIZE] = { 0x11,0x3b,0x12,0xab,0xbc,0x21,0x2d,0xae,0x31,0xc2,0xa6,0xc7,0xb4,0x07,0x6c,0x19 };

// check MD5 hash calculations (returns False on error)
Bool MD5_Check(void)
{
	u8 hash[MD5_HASH_SIZE];

	// check pattern 1
	MD5_Calc(hash, CrcPat1, CRCPAT1LEN);
	if (memcmp(hash, MD5_hash1, MD5_HASH_SIZE) != 0) return False;

	// check pattern 2
	MD5_Calc(hash, CrcPat2, CRCPAT2LEN);
	if (memcmp(hash, MD5_hash2, MD5_HASH_SIZE) != 0) return False;

	// check pattern 3
	MD5_Calc(hash, CrcPat3, CRCPAT3LEN);
	if (memcmp(hash, MD5_hash3, MD5_HASH_SIZE) != 0) return False;

	return True;
}

#endif // USE_MD5

// ============================================================================
//                           CRC-64 Normal (CRC64A)
// ============================================================================

// CRC-64 Normal (CRC64A) table (2 KB)
const u64 Crc64ATable[256] = {
	0x0000000000000000ull, 0x42F0E1EBA9EA3693ull, 0x85E1C3D753D46D26ull, 0xC711223CFA3E5BB5ull,
	0x493366450E42ECDFull, 0x0BC387AEA7A8DA4Cull, 0xCCD2A5925D9681F9ull, 0x8E224479F47CB76Aull,
	0x9266CC8A1C85D9BEull, 0xD0962D61B56FEF2Dull, 0x17870F5D4F51B498ull, 0x5577EEB6E6BB820Bull,
	0xDB55AACF12C73561ull, 0x99A54B24BB2D03F2ull, 0x5EB4691841135847ull, 0x1C4488F3E8F96ED4ull,
	0x663D78FF90E185EFull, 0x24CD9914390BB37Cull, 0xE3DCBB28C335E8C9ull, 0xA12C5AC36ADFDE5Aull,
	0x2F0E1EBA9EA36930ull, 0x6DFEFF5137495FA3ull, 0xAAEFDD6DCD770416ull, 0xE81F3C86649D3285ull,
	0xF45BB4758C645C51ull, 0xB6AB559E258E6AC2ull, 0x71BA77A2DFB03177ull, 0x334A9649765A07E4ull,
	0xBD68D2308226B08Eull, 0xFF9833DB2BCC861Dull, 0x388911E7D1F2DDA8ull, 0x7A79F00C7818EB3Bull,
	0xCC7AF1FF21C30BDEull, 0x8E8A101488293D4Dull, 0x499B3228721766F8ull, 0x0B6BD3C3DBFD506Bull,
	0x854997BA2F81E701ull, 0xC7B97651866BD192ull, 0x00A8546D7C558A27ull, 0x4258B586D5BFBCB4ull,
	0x5E1C3D753D46D260ull, 0x1CECDC9E94ACE4F3ull, 0xDBFDFEA26E92BF46ull, 0x990D1F49C77889D5ull,
	0x172F5B3033043EBFull, 0x55DFBADB9AEE082Cull, 0x92CE98E760D05399ull, 0xD03E790CC93A650Aull,
	0xAA478900B1228E31ull, 0xE8B768EB18C8B8A2ull, 0x2FA64AD7E2F6E317ull, 0x6D56AB3C4B1CD584ull,
	0xE374EF45BF6062EEull, 0xA1840EAE168A547Dull, 0x66952C92ECB40FC8ull, 0x2465CD79455E395Bull,
	0x3821458AADA7578Full, 0x7AD1A461044D611Cull, 0xBDC0865DFE733AA9ull, 0xFF3067B657990C3Aull,
	0x711223CFA3E5BB50ull, 0x33E2C2240A0F8DC3ull, 0xF4F3E018F031D676ull, 0xB60301F359DBE0E5ull,
	0xDA050215EA6C212Full, 0x98F5E3FE438617BCull, 0x5FE4C1C2B9B84C09ull, 0x1D14202910527A9Aull,
	0x93366450E42ECDF0ull, 0xD1C685BB4DC4FB63ull, 0x16D7A787B7FAA0D6ull, 0x5427466C1E109645ull,
	0x4863CE9FF6E9F891ull, 0x0A932F745F03CE02ull, 0xCD820D48A53D95B7ull, 0x8F72ECA30CD7A324ull,
	0x0150A8DAF8AB144Eull, 0x43A04931514122DDull, 0x84B16B0DAB7F7968ull, 0xC6418AE602954FFBull,
	0xBC387AEA7A8DA4C0ull, 0xFEC89B01D3679253ull, 0x39D9B93D2959C9E6ull, 0x7B2958D680B3FF75ull,
	0xF50B1CAF74CF481Full, 0xB7FBFD44DD257E8Cull, 0x70EADF78271B2539ull, 0x321A3E938EF113AAull,
	0x2E5EB66066087D7Eull, 0x6CAE578BCFE24BEDull, 0xABBF75B735DC1058ull, 0xE94F945C9C3626CBull,
	0x676DD025684A91A1ull, 0x259D31CEC1A0A732ull, 0xE28C13F23B9EFC87ull, 0xA07CF2199274CA14ull,
	0x167FF3EACBAF2AF1ull, 0x548F120162451C62ull, 0x939E303D987B47D7ull, 0xD16ED1D631917144ull,
	0x5F4C95AFC5EDC62Eull, 0x1DBC74446C07F0BDull, 0xDAAD56789639AB08ull, 0x985DB7933FD39D9Bull,
	0x84193F60D72AF34Full, 0xC6E9DE8B7EC0C5DCull, 0x01F8FCB784FE9E69ull, 0x43081D5C2D14A8FAull,
	0xCD2A5925D9681F90ull, 0x8FDAB8CE70822903ull, 0x48CB9AF28ABC72B6ull, 0x0A3B7B1923564425ull,
	0x70428B155B4EAF1Eull, 0x32B26AFEF2A4998Dull, 0xF5A348C2089AC238ull, 0xB753A929A170F4ABull,
	0x3971ED50550C43C1ull, 0x7B810CBBFCE67552ull, 0xBC902E8706D82EE7ull, 0xFE60CF6CAF321874ull,
	0xE224479F47CB76A0ull, 0xA0D4A674EE214033ull, 0x67C58448141F1B86ull, 0x253565A3BDF52D15ull,
	0xAB1721DA49899A7Full, 0xE9E7C031E063ACECull, 0x2EF6E20D1A5DF759ull, 0x6C0603E6B3B7C1CAull,
	0xF6FAE5C07D3274CDull, 0xB40A042BD4D8425Eull, 0x731B26172EE619EBull, 0x31EBC7FC870C2F78ull,
	0xBFC9838573709812ull, 0xFD39626EDA9AAE81ull, 0x3A28405220A4F534ull, 0x78D8A1B9894EC3A7ull,
	0x649C294A61B7AD73ull, 0x266CC8A1C85D9BE0ull, 0xE17DEA9D3263C055ull, 0xA38D0B769B89F6C6ull,
	0x2DAF4F0F6FF541ACull, 0x6F5FAEE4C61F773Full, 0xA84E8CD83C212C8Aull, 0xEABE6D3395CB1A19ull,
	0x90C79D3FEDD3F122ull, 0xD2377CD44439C7B1ull, 0x15265EE8BE079C04ull, 0x57D6BF0317EDAA97ull,
	0xD9F4FB7AE3911DFDull, 0x9B041A914A7B2B6Eull, 0x5C1538ADB04570DBull, 0x1EE5D94619AF4648ull,
	0x02A151B5F156289Cull, 0x4051B05E58BC1E0Full, 0x87409262A28245BAull, 0xC5B073890B687329ull,
	0x4B9237F0FF14C443ull, 0x0962D61B56FEF2D0ull, 0xCE73F427ACC0A965ull, 0x8C8315CC052A9FF6ull,
	0x3A80143F5CF17F13ull, 0x7870F5D4F51B4980ull, 0xBF61D7E80F251235ull, 0xFD913603A6CF24A6ull,
	0x73B3727A52B393CCull, 0x31439391FB59A55Full, 0xF652B1AD0167FEEAull, 0xB4A25046A88DC879ull,
	0xA8E6D8B54074A6ADull, 0xEA16395EE99E903Eull, 0x2D071B6213A0CB8Bull, 0x6FF7FA89BA4AFD18ull,
	0xE1D5BEF04E364A72ull, 0xA3255F1BE7DC7CE1ull, 0x64347D271DE22754ull, 0x26C49CCCB40811C7ull,
	0x5CBD6CC0CC10FAFCull, 0x1E4D8D2B65FACC6Full, 0xD95CAF179FC497DAull, 0x9BAC4EFC362EA149ull,
	0x158E0A85C2521623ull, 0x577EEB6E6BB820B0ull, 0x906FC95291867B05ull, 0xD29F28B9386C4D96ull,
	0xCEDBA04AD0952342ull, 0x8C2B41A1797F15D1ull, 0x4B3A639D83414E64ull, 0x09CA82762AAB78F7ull,
	0x87E8C60FDED7CF9Dull, 0xC51827E4773DF90Eull, 0x020905D88D03A2BBull, 0x40F9E43324E99428ull,
	0x2CFFE7D5975E55E2ull, 0x6E0F063E3EB46371ull, 0xA91E2402C48A38C4ull, 0xEBEEC5E96D600E57ull,
	0x65CC8190991CB93Dull, 0x273C607B30F68FAEull, 0xE02D4247CAC8D41Bull, 0xA2DDA3AC6322E288ull,
	0xBE992B5F8BDB8C5Cull, 0xFC69CAB42231BACFull, 0x3B78E888D80FE17Aull, 0x7988096371E5D7E9ull,
	0xF7AA4D1A85996083ull, 0xB55AACF12C735610ull, 0x724B8ECDD64D0DA5ull, 0x30BB6F267FA73B36ull,
	0x4AC29F2A07BFD00Dull, 0x08327EC1AE55E69Eull, 0xCF235CFD546BBD2Bull, 0x8DD3BD16FD818BB8ull,
	0x03F1F96F09FD3CD2ull, 0x41011884A0170A41ull, 0x86103AB85A2951F4ull, 0xC4E0DB53F3C36767ull,
	0xD8A453A01B3A09B3ull, 0x9A54B24BB2D03F20ull, 0x5D45907748EE6495ull, 0x1FB5719CE1045206ull,
	0x919735E51578E56Cull, 0xD367D40EBC92D3FFull, 0x1476F63246AC884Aull, 0x568617D9EF46BED9ull,
	0xE085162AB69D5E3Cull, 0xA275F7C11F7768AFull, 0x6564D5FDE549331Aull, 0x279434164CA30589ull,
	0xA9B6706FB8DFB2E3ull, 0xEB46918411358470ull, 0x2C57B3B8EB0BDFC5ull, 0x6EA7525342E1E956ull,
	0x72E3DAA0AA188782ull, 0x30133B4B03F2B111ull, 0xF7021977F9CCEAA4ull, 0xB5F2F89C5026DC37ull,
	0x3BD0BCE5A45A6B5Dull, 0x79205D0E0DB05DCEull, 0xBE317F32F78E067Bull, 0xFCC19ED95E6430E8ull,
	0x86B86ED5267CDBD3ull, 0xC4488F3E8F96ED40ull, 0x0359AD0275A8B6F5ull, 0x41A94CE9DC428066ull,
	0xCF8B0890283E370Cull, 0x8D7BE97B81D4019Full, 0x4A6ACB477BEA5A2Aull, 0x089A2AACD2006CB9ull,
	0x14DEA25F3AF9026Dull, 0x562E43B4931334FEull, 0x913F6188692D6F4Bull, 0xD3CF8063C0C759D8ull,
	0x5DEDC41A34BBEEB2ull, 0x1F1D25F19D51D821ull, 0xD80C07CD676F8394ull, 0x9AFCE626CE85B507ull,
};

// Check CRC-64 Normal (CRC64A) table (returns False on error; can be used to generate table)
Bool Crc64ATableCheck()
{
	int i, j;
	u64 k;

	for(i = 0; i < 256; i++)
	{
		k = (u64)i << 56;
		for (j = 8; j > 0; j--)
		{
			if ((k & 0x8000000000000000ULL) == 0)
				k <<= 1;
			else
				k = (k << 1) ^ CRC64A_POLY;
		}
		//Crc64ATable[i] = k; // generate table
		if (Crc64ATable[i] != k) return False; // check table
	}
	return True;
}

// Calculate CRC-64 Normal (CRC64A), 1 byte - tabled version (requires 2 KB of flash memory)
u64 Crc64AByteTab(u64 crc, u8 data)
{
	return Crc64ATable[(crc >> 56) ^ data] ^ (crc << 8);
}

// Calculate CRC-64 Normal (CRC64A), 1 byte - slow version
u64 Crc64AByteSlow(u64 crc, u8 data)
{
	int i;
	crc ^= (u64)data << 56;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 0x8000000000000000ull) == 0)
			crc <<= 1;
		else
			crc = (crc << 1) ^ CRC64A_POLY;
	}
	return crc;
}

// Calculate CRC-64 Normal (CRC64A), buffer - tabled version (requires 2 KB of flash memory)
u64 Crc64ABufTab(u64 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;

	for (; len > 0; len--)
	{
		// Normal formula
		crc = Crc64ATable[(crc >> 56) ^ *s++] ^ (crc << 8);
	}
	return crc;
}

// Calculate CRC-64 Normal (CRC64A), buffer - slow version
u64 Crc64ABufSlow(u64 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	int i;

	for (; len > 0; len--)
	{
		crc ^= (u64)*s++ << 56;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 0x8000000000000000ull) == 0)
				crc <<= 1;
			else
				crc = (crc << 1) ^ CRC64A_POLY;
		}
	}
	return crc;
}

// Calculate CRC-64 Normal (CRC64A) - tabled version (requires 2 KB of flash memory)
//   Calculation speed: 200 us per 1 KB
u64 Crc64ATab(const void* buf, int len)
{
	return Crc64ABufTab(CRC64A_INIT, buf, len);
}

// Calculate CRC-64 Normal (CRC64A) - slow version
//   Calculation speed: 1000 us per 1 KB
u64 Crc64ASlow(const void* buf, int len)
{
	return Crc64ABufSlow(CRC64A_INIT, buf, len);
}

// Check CRC-64 Normal (CRC64A) calculations (returns False on error)
Bool Crc64ACheck()
{
	u64 crc;
	int i;
	const u8* src;

	// Check CRC-64 table
	if (!Crc64ATableCheck()) return False;

	// Check CRC-64 pattern
#define CRC64A_RES1 0x9D13A61C0E5B0FF5ull
#define CRC64A_RES2 0x21E4F88DB2978548ull
#define CRC64A_RES3 0x378302BE2C61CF9Eull
	if (Crc64ATab(CrcPat1, CRCPAT1LEN) != CRC64A_RES1) return False;
	if (Crc64ATab(CrcPat2, CRCPAT2LEN) != CRC64A_RES2) return False;
	if (Crc64ATab(CrcPat3, CRCPAT3LEN) != CRC64A_RES3) return False;

	if (Crc64ASlow(CrcPat1, CRCPAT1LEN) != CRC64A_RES1) return False;
	if (Crc64ASlow(CrcPat2, CRCPAT2LEN) != CRC64A_RES2) return False;
	if (Crc64ASlow(CrcPat3, CRCPAT3LEN) != CRC64A_RES3) return False;

	// CRC-64 buffer version
#define CRC64A_SPLIT 503
	crc = Crc64ABufTab(CRC64A_INIT, CrcPat3, CRC64A_SPLIT);
	crc = Crc64ABufTab(crc, (const u8*)CrcPat3+CRC64A_SPLIT, CRCPAT3LEN-CRC64A_SPLIT);
	if (crc != CRC64A_RES3) return False;

	crc = Crc64ABufSlow(CRC64A_INIT, CrcPat3, CRC64A_SPLIT);
	crc = Crc64ABufSlow(crc, (const u8*)CrcPat3+CRC64A_SPLIT, CRCPAT3LEN-CRC64A_SPLIT);
	if (crc != CRC64A_RES3) return False;
	
	// CRC-64 byte version
	crc = CRC64A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc64AByteTab(crc, *src++);
	if (crc != CRC64A_RES3) return False;

	crc = CRC64A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc64AByteSlow(crc, *src++);
	if (crc != CRC64A_RES3) return False;

	return True;
}

// ============================================================================
//                           CRC-64 Reversed (CRC64B)
// ============================================================================

// CRC-64 Reversed (CRC64B) table (2 KB)
const u64 Crc64BTable[256] = {
	0x0000000000000000ull, 0xB32E4CBE03A75F6Full, 0xF4843657A840A05Bull, 0x47AA7AE9ABE7FF34ull,
	0x7BD0C384FF8F5E33ull, 0xC8FE8F3AFC28015Cull, 0x8F54F5D357CFFE68ull, 0x3C7AB96D5468A107ull,
	0xF7A18709FF1EBC66ull, 0x448FCBB7FCB9E309ull, 0x0325B15E575E1C3Dull, 0xB00BFDE054F94352ull,
	0x8C71448D0091E255ull, 0x3F5F08330336BD3Aull, 0x78F572DAA8D1420Eull, 0xCBDB3E64AB761D61ull,
	0x7D9BA13851336649ull, 0xCEB5ED8652943926ull, 0x891F976FF973C612ull, 0x3A31DBD1FAD4997Dull,
	0x064B62BCAEBC387Aull, 0xB5652E02AD1B6715ull, 0xF2CF54EB06FC9821ull, 0x41E11855055BC74Eull,
	0x8A3A2631AE2DDA2Full, 0x39146A8FAD8A8540ull, 0x7EBE1066066D7A74ull, 0xCD905CD805CA251Bull,
	0xF1EAE5B551A2841Cull, 0x42C4A90B5205DB73ull, 0x056ED3E2F9E22447ull, 0xB6409F5CFA457B28ull,
	0xFB374270A266CC92ull, 0x48190ECEA1C193FDull, 0x0FB374270A266CC9ull, 0xBC9D3899098133A6ull,
	0x80E781F45DE992A1ull, 0x33C9CD4A5E4ECDCEull, 0x7463B7A3F5A932FAull, 0xC74DFB1DF60E6D95ull,
	0x0C96C5795D7870F4ull, 0xBFB889C75EDF2F9Bull, 0xF812F32EF538D0AFull, 0x4B3CBF90F69F8FC0ull,
	0x774606FDA2F72EC7ull, 0xC4684A43A15071A8ull, 0x83C230AA0AB78E9Cull, 0x30EC7C140910D1F3ull,
	0x86ACE348F355AADBull, 0x3582AFF6F0F2F5B4ull, 0x7228D51F5B150A80ull, 0xC10699A158B255EFull,
	0xFD7C20CC0CDAF4E8ull, 0x4E526C720F7DAB87ull, 0x09F8169BA49A54B3ull, 0xBAD65A25A73D0BDCull,
	0x710D64410C4B16BDull, 0xC22328FF0FEC49D2ull, 0x85895216A40BB6E6ull, 0x36A71EA8A7ACE989ull,
	0x0ADDA7C5F3C4488Eull, 0xB9F3EB7BF06317E1ull, 0xFE5991925B84E8D5ull, 0x4D77DD2C5823B7BAull,
	0x64B62BCAEBC387A1ull, 0xD7986774E864D8CEull, 0x90321D9D438327FAull, 0x231C512340247895ull,
	0x1F66E84E144CD992ull, 0xAC48A4F017EB86FDull, 0xEBE2DE19BC0C79C9ull, 0x58CC92A7BFAB26A6ull,
	0x9317ACC314DD3BC7ull, 0x2039E07D177A64A8ull, 0x67939A94BC9D9B9Cull, 0xD4BDD62ABF3AC4F3ull,
	0xE8C76F47EB5265F4ull, 0x5BE923F9E8F53A9Bull, 0x1C4359104312C5AFull, 0xAF6D15AE40B59AC0ull,
	0x192D8AF2BAF0E1E8ull, 0xAA03C64CB957BE87ull, 0xEDA9BCA512B041B3ull, 0x5E87F01B11171EDCull,
	0x62FD4976457FBFDBull, 0xD1D305C846D8E0B4ull, 0x96797F21ED3F1F80ull, 0x2557339FEE9840EFull,
	0xEE8C0DFB45EE5D8Eull, 0x5DA24145464902E1ull, 0x1A083BACEDAEFDD5ull, 0xA9267712EE09A2BAull,
	0x955CCE7FBA6103BDull, 0x267282C1B9C65CD2ull, 0x61D8F8281221A3E6ull, 0xD2F6B4961186FC89ull,
	0x9F8169BA49A54B33ull, 0x2CAF25044A02145Cull, 0x6B055FEDE1E5EB68ull, 0xD82B1353E242B407ull,
	0xE451AA3EB62A1500ull, 0x577FE680B58D4A6Full, 0x10D59C691E6AB55Bull, 0xA3FBD0D71DCDEA34ull,
	0x6820EEB3B6BBF755ull, 0xDB0EA20DB51CA83Aull, 0x9CA4D8E41EFB570Eull, 0x2F8A945A1D5C0861ull,
	0x13F02D374934A966ull, 0xA0DE61894A93F609ull, 0xE7741B60E174093Dull, 0x545A57DEE2D35652ull,
	0xE21AC88218962D7Aull, 0x5134843C1B317215ull, 0x169EFED5B0D68D21ull, 0xA5B0B26BB371D24Eull,
	0x99CA0B06E7197349ull, 0x2AE447B8E4BE2C26ull, 0x6D4E3D514F59D312ull, 0xDE6071EF4CFE8C7Dull,
	0x15BB4F8BE788911Cull, 0xA6950335E42FCE73ull, 0xE13F79DC4FC83147ull, 0x521135624C6F6E28ull,
	0x6E6B8C0F1807CF2Full, 0xDD45C0B11BA09040ull, 0x9AEFBA58B0476F74ull, 0x29C1F6E6B3E0301Bull,
	0xC96C5795D7870F42ull, 0x7A421B2BD420502Dull, 0x3DE861C27FC7AF19ull, 0x8EC62D7C7C60F076ull,
	0xB2BC941128085171ull, 0x0192D8AF2BAF0E1Eull, 0x4638A2468048F12Aull, 0xF516EEF883EFAE45ull,
	0x3ECDD09C2899B324ull, 0x8DE39C222B3EEC4Bull, 0xCA49E6CB80D9137Full, 0x7967AA75837E4C10ull,
	0x451D1318D716ED17ull, 0xF6335FA6D4B1B278ull, 0xB199254F7F564D4Cull, 0x02B769F17CF11223ull,
	0xB4F7F6AD86B4690Bull, 0x07D9BA1385133664ull, 0x4073C0FA2EF4C950ull, 0xF35D8C442D53963Full,
	0xCF273529793B3738ull, 0x7C0979977A9C6857ull, 0x3BA3037ED17B9763ull, 0x888D4FC0D2DCC80Cull,
	0x435671A479AAD56Dull, 0xF0783D1A7A0D8A02ull, 0xB7D247F3D1EA7536ull, 0x04FC0B4DD24D2A59ull,
	0x3886B22086258B5Eull, 0x8BA8FE9E8582D431ull, 0xCC0284772E652B05ull, 0x7F2CC8C92DC2746Aull,
	0x325B15E575E1C3D0ull, 0x8175595B76469CBFull, 0xC6DF23B2DDA1638Bull, 0x75F16F0CDE063CE4ull,
	0x498BD6618A6E9DE3ull, 0xFAA59ADF89C9C28Cull, 0xBD0FE036222E3DB8ull, 0x0E21AC88218962D7ull,
	0xC5FA92EC8AFF7FB6ull, 0x76D4DE52895820D9ull, 0x317EA4BB22BFDFEDull, 0x8250E80521188082ull,
	0xBE2A516875702185ull, 0x0D041DD676D77EEAull, 0x4AAE673FDD3081DEull, 0xF9802B81DE97DEB1ull,
	0x4FC0B4DD24D2A599ull, 0xFCEEF8632775FAF6ull, 0xBB44828A8C9205C2ull, 0x086ACE348F355AADull,
	0x34107759DB5DFBAAull, 0x873E3BE7D8FAA4C5ull, 0xC094410E731D5BF1ull, 0x73BA0DB070BA049Eull,
	0xB86133D4DBCC19FFull, 0x0B4F7F6AD86B4690ull, 0x4CE50583738CB9A4ull, 0xFFCB493D702BE6CBull,
	0xC3B1F050244347CCull, 0x709FBCEE27E418A3ull, 0x3735C6078C03E797ull, 0x841B8AB98FA4B8F8ull,
	0xADDA7C5F3C4488E3ull, 0x1EF430E13FE3D78Cull, 0x595E4A08940428B8ull, 0xEA7006B697A377D7ull,
	0xD60ABFDBC3CBD6D0ull, 0x6524F365C06C89BFull, 0x228E898C6B8B768Bull, 0x91A0C532682C29E4ull,
	0x5A7BFB56C35A3485ull, 0xE955B7E8C0FD6BEAull, 0xAEFFCD016B1A94DEull, 0x1DD181BF68BDCBB1ull,
	0x21AB38D23CD56AB6ull, 0x9285746C3F7235D9ull, 0xD52F0E859495CAEDull, 0x6601423B97329582ull,
	0xD041DD676D77EEAAull, 0x636F91D96ED0B1C5ull, 0x24C5EB30C5374EF1ull, 0x97EBA78EC690119Eull,
	0xAB911EE392F8B099ull, 0x18BF525D915FEFF6ull, 0x5F1528B43AB810C2ull, 0xEC3B640A391F4FADull,
	0x27E05A6E926952CCull, 0x94CE16D091CE0DA3ull, 0xD3646C393A29F297ull, 0x604A2087398EADF8ull,
	0x5C3099EA6DE60CFFull, 0xEF1ED5546E415390ull, 0xA8B4AFBDC5A6ACA4ull, 0x1B9AE303C601F3CBull,
	0x56ED3E2F9E224471ull, 0xE5C372919D851B1Eull, 0xA26908783662E42Aull, 0x114744C635C5BB45ull,
	0x2D3DFDAB61AD1A42ull, 0x9E13B115620A452Dull, 0xD9B9CBFCC9EDBA19ull, 0x6A978742CA4AE576ull,
	0xA14CB926613CF817ull, 0x1262F598629BA778ull, 0x55C88F71C97C584Cull, 0xE6E6C3CFCADB0723ull,
	0xDA9C7AA29EB3A624ull, 0x69B2361C9D14F94Bull, 0x2E184CF536F3067Full, 0x9D36004B35545910ull,
	0x2B769F17CF112238ull, 0x9858D3A9CCB67D57ull, 0xDFF2A94067518263ull, 0x6CDCE5FE64F6DD0Cull,
	0x50A65C93309E7C0Bull, 0xE388102D33392364ull, 0xA4226AC498DEDC50ull, 0x170C267A9B79833Full,
	0xDCD7181E300F9E5Eull, 0x6FF954A033A8C131ull, 0x28532E49984F3E05ull, 0x9B7D62F79BE8616Aull,
	0xA707DB9ACF80C06Dull, 0x14299724CC279F02ull, 0x5383EDCD67C06036ull, 0xE0ADA17364673F59ull,
};

// Check CRC-64 Reversed (CRC64B) table (returns False on error; can be used to generate table)
Bool Crc64BTableCheck()
{
	int i, j;
	u64 k;

	for(i = 0; i < 256; i++)
	{
		k = i;
		for (j = 8; j > 0; j--)
		{
			if ((k & 1) == 0)
				k >>= 1;
			else
				k = (k >> 1) ^ CRC64B_POLY;
		}
		// Crc64BTable[i] = k; // generate table
		if (Crc64BTable[i] != k) return False; // check table
	}
	return True;
}

// Calculate CRC-64 Reversed (CRC64B), 1 byte - tabled version (requires 1 KB of flash memory)
u64 Crc64BByteTab(u64 crc, u8 data)
{
	crc = ~crc;
	return ~Crc64BTable[(crc ^ data) & 0xff] ^ (crc >> 8);
}

// Calculate CRC-64 Reversed (CRC64B), 1 byte - slow version
u64 Crc64BByteSlow(u64 crc, u8 data)
{
	int i;
	crc = ~crc;
	crc ^= data;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 1) == 0)
			crc >>= 1;
		else
			crc = (crc >> 1) ^ CRC64B_POLY;
	}
	return ~crc;
}

// Calculate CRC-64 Reversed (CRC64B), buffer - tabled version (requires 2 KB of flash memory)
u64 Crc64BBufTab(u64 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;

	for (; len > 0; len--)
	{
		// Reflect formula
		crc = Crc64BTable[(crc ^ *s++) & 0xff] ^ (crc >> 8);
	}
	return ~crc;
}

// Calculate CRC-64 Reversed (CRC64B), buffer - slow version
u64 Crc64BBufSlow(u64 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;
	int i;

	for (; len > 0; len--)
	{
		crc ^= *s++;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (crc >> 1) ^ CRC64B_POLY;
		}
	}
	return ~crc;
}

// Calculate CRC-64 Reversed (CRC64B) - tabled version (requires 2 KB of flash memory)
//   Calculation speed: 220 us per 1 KB
u64 Crc64BTab(const void* buf, int len)
{
	return Crc64BBufTab(CRC64B_INIT, buf, len);
}

// Calculate CRC-64 Reversed (CRC64B) - slow version
//   Calculation speed: 2200 us per 1 KB
u64 Crc64BSlow(const void* buf, int len)
{
	return Crc64BBufSlow(CRC64B_INIT, buf, len);
}

// Check CRC-64 Reversed (CRC64B) calculations (returns False on error)
Bool Crc64BCheck()
{
	u64 crc;
	int i;
	const u8* src;

	// Check CRC-64 table
	if (!Crc64BTableCheck()) return False;

	// Check CRC-64 pattern
#define CRC64B_RES1 0x995DC9BBDF1939FAull
#define CRC64B_RES2 0xF10B3B2CDEF45CFAull
#define CRC64B_RES3 0xACB732293EDC5DC8ull
	if (Crc64BTab(CrcPat1, CRCPAT1LEN) != CRC64B_RES1) return False;
	if (Crc64BTab(CrcPat2, CRCPAT2LEN) != CRC64B_RES2) return False;
	if (Crc64BTab(CrcPat3, CRCPAT3LEN) != CRC64B_RES3) return False;

	if (Crc64BSlow(CrcPat1, CRCPAT1LEN) != CRC64B_RES1) return False;
	if (Crc64BSlow(CrcPat2, CRCPAT2LEN) != CRC64B_RES2) return False;
	if (Crc64BSlow(CrcPat3, CRCPAT3LEN) != CRC64B_RES3) return False;

	// CRC-64 buffer version
#define CRC64B_SPLIT 503
	crc = Crc64BBufTab(CRC64B_INIT, CrcPat3, CRC64B_SPLIT);
	crc = Crc64BBufTab(crc, (const u8*)CrcPat3+CRC64B_SPLIT, CRCPAT3LEN-CRC64B_SPLIT);
	if (crc != CRC64B_RES3) return False;

	crc = Crc64BBufSlow(CRC64B_INIT, CrcPat3, CRC64B_SPLIT);
	crc = Crc64BBufSlow(crc, (const u8*)CrcPat3+CRC64B_SPLIT, CRCPAT3LEN-CRC64B_SPLIT);
	if (crc != CRC64B_RES3) return False;

	// CRC-64 byte version
	crc = CRC64B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc64BByteTab(crc, *src++);
	if (crc != CRC64B_RES3) return False;

	crc = CRC64B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc64BByteSlow(crc, *src++);
	if (crc != CRC64B_RES3) return False;

	return True;
}

// ============================================================================
//                           CRC-32 Normal (CRC32A)
// ============================================================================

// CRC-32 Normal (CRC32A) table (1 KB)
const u32 Crc32ATable[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,

	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

// Check CRC-32 Normal (CRC32A) table (returns False on error; can be used to generate table)
Bool Crc32ATableCheck()
{
	int i, j;
	u32 k;

	for(i = 0; i < 256; i++)
	{
		k = (u32)i << 24;
		for (j = 8; j > 0; j--)
		{
			if ((k & 0x80000000) == 0)
				k <<= 1;
			else
				k = (k << 1) ^ CRC32A_POLY;
		}
		//Crc32ATable[i] = k; // generate table
		if (Crc32ATable[i] != k) return False; // check table
	}
	return True;
}

// Calculate CRC-32 Normal (CRC32A), 1 byte - tabled version (requires 1 KB of flash memory)
u32 Crc32AByteTab(u32 crc, u8 data)
{
	return Crc32ATable[(crc >> 24) ^ data] ^ (crc << 8);
}

// Calculate CRC-32 Normal (CRC32A), 1 byte - slow version
u32 Crc32AByteSlow(u32 crc, u8 data)
{
	int i;
	crc ^= (u32)data << 24;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 0x80000000) == 0)
			crc <<= 1;
		else
			crc = (crc << 1) ^ CRC32A_POLY;
	}
	return crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32AByteDMA(u32 crc, u8 data)
{
	return DMA_CRC(DMA_CRC_CRC32, crc, DMA_TEMP_CHAN(), &data, 1);
}
#endif // USE_DMA

// Calculate CRC-32 Normal (CRC32A), buffer - tabled version (requires 1 KB of flash memory)
u32 Crc32ABufTab(u32 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;

	for (; len > 0; len--)
	{
		// Normal formula
		crc = Crc32ATable[(crc >> 24) ^ *s++] ^ (crc << 8);
	}
	return crc;
}

// Calculate CRC-32 Normal (CRC32A), buffer - slow version
u32 Crc32ABufSlow(u32 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	int i;

	for (; len > 0; len--)
	{
		crc ^= (u32)*s++ << 24;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 0x80000000) == 0)
				crc <<= 1;
			else
				crc = (crc << 1) ^ CRC32A_POLY;
		}
	}
	return crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32ABufDMA(u32 crc, const void* buf, int len)
{
	return DMA_CRC(DMA_CRC_CRC32, crc, DMA_TEMP_CHAN(), buf, len);
}
#endif // USE_DMA

// Calculate CRC-32 Normal (CRC32A) - tabled version (requires 1 KB of flash memory)
//   Calculation speed: 160 us per 1 KB
u32 Crc32ATab(const void* buf, int len)
{
	return Crc32ABufTab(CRC32A_INIT, buf, len);
}

// Calculate CRC-32 Normal (CRC32A) - slow version
//   Calculation speed: 790 us per 1 KB
u32 Crc32ASlow(const void* buf, int len)
{
	return Crc32ABufSlow(CRC32A_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Normal (CRC32A) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32ADMA(const void* buf, int len)
{
	return Crc32ABufDMA(CRC32A_INIT, buf, len);
}
#endif // USE_DMA

// Check CRC-32 Normal (CRC32A) calculations (returns False on error)
Bool Crc32ACheck()
{
	u32 crc;
	int i;
	const u8* src;

	// Check CRC-32 table
	if (!Crc32ATableCheck()) return False;

	// Check CRC-32 pattern
#define CRC32A_RES1 0x0376E6E7
#define CRC32A_RES2 0x8E10D720
#define CRC32A_RES3 0x5796333D
	if (Crc32ATab(CrcPat1, CRCPAT1LEN) != CRC32A_RES1) return False;
	if (Crc32ATab(CrcPat2, CRCPAT2LEN) != CRC32A_RES2) return False;
	if (Crc32ATab(CrcPat3, CRCPAT3LEN) != CRC32A_RES3) return False;

	if (Crc32ASlow(CrcPat1, CRCPAT1LEN) != CRC32A_RES1) return False;
	if (Crc32ASlow(CrcPat2, CRCPAT2LEN) != CRC32A_RES2) return False;
	if (Crc32ASlow(CrcPat3, CRCPAT3LEN) != CRC32A_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Crc32ADMA(CrcPat1, CRCPAT1LEN) != CRC32A_RES1) return False;
	if (Crc32ADMA(CrcPat2, CRCPAT2LEN) != CRC32A_RES2) return False;
	if (Crc32ADMA(CrcPat3, CRCPAT3LEN) != CRC32A_RES3) return False;
#endif // USE_DMA

	// CRC-32 buffer version
#define CRC32A_SPLIT 503
	crc = Crc32ABufTab(CRC32A_INIT, CrcPat3, CRC32A_SPLIT);
	crc = Crc32ABufTab(crc, (const u8*)CrcPat3+CRC32A_SPLIT, CRCPAT3LEN-CRC32A_SPLIT);
	if (crc != CRC32A_RES3) return False;

	crc = Crc32ABufSlow(CRC32A_INIT, CrcPat3, CRC32A_SPLIT);
	crc = Crc32ABufSlow(crc, (const u8*)CrcPat3+CRC32A_SPLIT, CRCPAT3LEN-CRC32A_SPLIT);
	if (crc != CRC32A_RES3) return False;
	
#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Crc32ABufDMA(CRC32A_INIT, CrcPat3, CRC32A_SPLIT);
	crc = Crc32ABufDMA(crc, (const u8*)CrcPat3+CRC32A_SPLIT, CRCPAT3LEN-CRC32A_SPLIT);
	if (crc != CRC32A_RES3) return False;
#endif // USE_DMA
	
	// CRC-32 byte version
	crc = CRC32A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32AByteTab(crc, *src++);
	if (crc != CRC32A_RES3) return False;

	crc = CRC32A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32AByteSlow(crc, *src++);
	if (crc != CRC32A_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = CRC32A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32AByteDMA(crc, *src++);
	if (crc != CRC32A_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                          CRC-32 Reversed (CRC32B)
// ============================================================================

// CRC-32 Reversed (CRC32B) table (1 KB)
const u32 Crc32BTable[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,

	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

// Check CRC-32 Reversed (CRC32B) table (returns False on error; can be used to generate table)
Bool Crc32BTableCheck()
{
	int i, j;
	u32 k;

	for(i = 0; i < 256; i++)
	{
		k = i;
		for (j = 8; j > 0; j--)
		{
			if ((k & 1) == 0)
				k >>= 1;
			else
				k = (k >> 1) ^ CRC32B_POLY;
		}
		// Crc32BTable[i] = k; // generate table
		if (Crc32BTable[i] != k) return False; // check table
	}
	return True;
}

// Calculate CRC-32 Reversed (CRC32B), 1 byte - tabled version (requires 1 KB of flash memory)
u32 Crc32BByteTab(u32 crc, u8 data)
{
	crc = ~crc;
	return ~Crc32BTable[(crc ^ data) & 0xff] ^ (crc >> 8);
}

// Calculate CRC-32 Reversed (CRC32B), 1 byte - slow version
u32 Crc32BByteSlow(u32 crc, u8 data)
{
	int i;
	crc = ~crc;
	crc ^= data;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 1) == 0)
			crc >>= 1;
		else
			crc = (crc >> 1) ^ CRC32B_POLY;
	}
	return ~crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BByteDMA(u32 crc, u8 data)
{
	return DMA_CRC(DMA_CRC_CRC32REV | DMA_CRC_REV | DMA_CRC_INV, reverse(~crc), DMA_TEMP_CHAN(), &data, 1);
}
#endif // USE_DMA

// Calculate CRC-32 Reversed (CRC32B), buffer - tabled version (requires 1 KB of flash memory)
u32 Crc32BBufTab(u32 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;

	for (; len > 0; len--)
	{
		// Reflect formula
		crc = Crc32BTable[(crc ^ *s++) & 0xff] ^ (crc >> 8);
	}
	return ~crc;
}

// Calculate CRC-32 Reversed (CRC32B), buffer - slow version
u32 Crc32BBufSlow(u32 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;
	int i;

	for (; len > 0; len--)
	{
		crc ^= *s++;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (crc >> 1) ^ CRC32B_POLY;
		}
	}
	return ~crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BBufDMA(u32 crc, const void* buf, int len)
{
	return DMA_CRC(DMA_CRC_CRC32REV | DMA_CRC_REV | DMA_CRC_INV, reverse(~crc), DMA_TEMP_CHAN(), buf, len);
}
#endif // USE_DMA

// Calculate CRC-32 Reversed (CRC32B) - tabled version (requires 1 KB of flash memory)
//   Calculation speed: 160 us per 1 KB
u32 Crc32BTab(const void* buf, int len)
{
	return Crc32BBufTab(CRC32B_INIT, buf, len);
}

// Calculate CRC-32 Reversed (CRC32B) - slow version
//   Calculation speed: 900 us per 1 KB
u32 Crc32BSlow(const void* buf, int len)
{
	return Crc32BBufSlow(CRC32B_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-32 Reversed (CRC32B) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Crc32BDMA(const void* buf, int len)
{
	return Crc32BBufDMA(CRC32B_INIT, buf, len);
}
#endif // USE_DMA

// Check CRC-32 Reversed (CRC32B) calculations (returns False on error)
Bool Crc32BCheck()
{
	u32 crc;
	int i;
	const u8* src;

	// Check CRC-32 table
	if (!Crc32BTableCheck()) return False;

	// Check CRC-32 pattern
#define CRC32B_RES1 0xCBF43926
#define CRC32B_RES2 0xA8E10F6D
#define CRC32B_RES3 0x6FCF9E13
	if (Crc32BTab(CrcPat1, CRCPAT1LEN) != CRC32B_RES1) return False;
	if (Crc32BTab(CrcPat2, CRCPAT2LEN) != CRC32B_RES2) return False;
	if (Crc32BTab(CrcPat3, CRCPAT3LEN) != CRC32B_RES3) return False;

	if (Crc32BSlow(CrcPat1, CRCPAT1LEN) != CRC32B_RES1) return False;
	if (Crc32BSlow(CrcPat2, CRCPAT2LEN) != CRC32B_RES2) return False;
	if (Crc32BSlow(CrcPat3, CRCPAT3LEN) != CRC32B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Crc32BDMA(CrcPat1, CRCPAT1LEN) != CRC32B_RES1) return False;
	if (Crc32BDMA(CrcPat2, CRCPAT2LEN) != CRC32B_RES2) return False;
	if (Crc32BDMA(CrcPat3, CRCPAT3LEN) != CRC32B_RES3) return False;
#endif // USE_DMA

	// CRC-32 buffer version
#define CRC32B_SPLIT 503
	crc = Crc32BBufTab(CRC32B_INIT, CrcPat3, CRC32B_SPLIT);
	crc = Crc32BBufTab(crc, (const u8*)CrcPat3+CRC32B_SPLIT, CRCPAT3LEN-CRC32B_SPLIT);
	if (crc != CRC32B_RES3) return False;

	crc = Crc32BBufSlow(CRC32B_INIT, CrcPat3, CRC32B_SPLIT);
	crc = Crc32BBufSlow(crc, (const u8*)CrcPat3+CRC32B_SPLIT, CRCPAT3LEN-CRC32B_SPLIT);
	if (crc != CRC32B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Crc32BBufDMA(CRC32B_INIT, CrcPat3, CRC32B_SPLIT);
	crc = Crc32BBufDMA(crc, (const u8*)CrcPat3+CRC32B_SPLIT, CRCPAT3LEN-CRC32B_SPLIT);
	if (crc != CRC32B_RES3) return False;
#endif // USE_DMA

	// CRC-32 byte version
	crc = CRC32B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32BByteTab(crc, *src++);
	if (crc != CRC32B_RES3) return False;

	crc = CRC32B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32BByteSlow(crc, *src++);
	if (crc != CRC32B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = CRC32B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc32BByteDMA(crc, *src++);
	if (crc != CRC32B_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                       CRC-16 CCITT Normal (CRC16A)
// ============================================================================

// CRC-16 CCITT Normal (CRC16A) table (512 B)
const u16 Crc16ATable[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,

	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

// Check CRC-16 CCITT Normal (CRC16A) table (returns False on error; can be used to generate table)
Bool Crc16ATableCheck()
{
	int i, j;
	u16 crc, c;

	for(i = 0; i < 256; i++)
	{
		crc = 0;
		c = (u16)(i << 8);
		for (j = 8; j > 0; j--)
		{
			if (((crc ^ c) & 0x8000) == 0)
				crc <<= 1;
			else
				crc = (u16)((crc << 1) ^ CRC16A_POLY);
			c <<= 1;
		}
		// Crc16ATable[i] = crc; // generate table
		if (Crc16ATable[i] != crc) return False;
	}
	return True;
}

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - tabled version (requires 512 B of flash memory)
u16 Crc16AByteTab(u16 crc, u8 data)
{
	return Crc16ATable[(crc >> 8) ^ data] ^ (crc << 8);
}

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - fast version
// http://www.embeddedrelated.com/groups/msp430/show/29689.php
u16 Crc16AByteFast(u16 crc, u8 data)
{
	crc = (crc >> 8) | (crc << 8);
	crc ^= data;
	crc ^= (crc & 0xff) >> 4;
	crc ^= crc << 12;
	crc ^= (crc & 0xff) << 5;
	return crc;
}

// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - slow version
u16 Crc16AByteSlow(u16 crc, u8 data)
{
	int i;
	crc ^= (u16)data << 8;

	for (i = 8; i > 0; i--)
	{
		if ((crc & 0x8000) == 0)
			crc <<= 1;
		else
			crc = (u16)((crc << 1) ^ CRC16A_POLY);
	}
	return crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16AByteDMA(u16 crc, u8 data)
{
	return (u16)DMA_CRC(DMA_CRC_CRC16, crc, DMA_TEMP_CHAN(), &data, 1);
}
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - tabled version (requires 512 B of flash memory)
u16 Crc16ABufTab(u16 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;

	for (; len > 0; len--)
	{
		crc = Crc16ATable[(crc >> 8) ^ *s++] ^ (crc << 8);
	}
	return crc;
}

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - fast version
u16 Crc16ABufFast(u16 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;

	for (; len > 0; len--)
	{
		crc = (crc >> 8) | (crc << 8);
		crc ^= *s++;
		crc ^= (crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0xff) << 5;
	}
	return crc;
}

// Calculate CRC-16 CCITT Normal (CRC16A), buffer - slow version
u16 Crc16ABufSlow(u16 crc, const void* buf, int len)
{
	int i;
	const u8* s = (const u8*)buf;

	for (; len > 0; len--)
	{
		crc ^= (u16)*s++ << 8;

		for (i = 8; i > 0; i--)
		{
			if ((crc & 0x8000) == 0)
				crc <<= 1;
			else
				crc = (u16)((crc << 1) ^ CRC16A_POLY);
		}
	}
	return crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16ABufDMA(u16 crc, const void* buf, int len)
{
	return (u16)DMA_CRC(DMA_CRC_CRC16, crc, DMA_TEMP_CHAN(), buf, len);
}
#endif // USE_DMA

// Calculate CRC-16 CCITT Normal (CRC16A) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 170 us per 1 KB
u16 Crc16ATab(const void* buf, int len)
{
	return Crc16ABufTab(CRC16A_INIT, buf, len);
}

// Calculate CRC-16 CCITT Normal (CRC16A) - fast version
//   Calculation speed: 200 us per 1 KB
u16 Crc16AFast(const void* buf, int len)
{
	return Crc16ABufFast(CRC16A_INIT, buf, len);
}

// Calculate CRC-16 CCITT Normal (CRC16A) - slow version
//   Calculation speed: 1000 us per 1 KB
u16 Crc16ASlow(const void* buf, int len)
{
	return Crc16ABufSlow(CRC16A_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal (CRC16A) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16ADMA(const void* buf, int len)
{
	return Crc16ABufDMA(CRC16A_INIT, buf, len);
}
#endif // USE_DMA

// Check CRC-16 CCITT Normal (CRC16A) fast method (return False on error)
//  Very slow (takes a few seconds) - do not use usually
Bool Crc16AFastCheck()
{
	u16 crc = 0;
	u8 data = 0;
	for (;;)
	{
		if (Crc16AByteTab(crc, data) != (Crc16AByteFast(crc, data))) return False;
		data++;
		if (data == 0)
		{
			crc++;
			if (crc == 0) break;
		}
	}
	return True;
}

// Check CRC-16 CCITT Normal (CRC16A) calculations (returns False on error)
Bool Crc16ACheck()
{
	u16 crc;
	int i;
	const u8* src;

	// Check CRC-16 table
	if (!Crc16ATableCheck()) return False;

	// Check CRC-16 fast method
	//  Very slow (takes a few seconds) - do not use usually
	//if (!Crc16AFastCheck()) return False;

	// Check CRC-16-CCITT (0xFFFF) pattern
#define CRC16A_RES1 0x29B1
#define CRC16A_RES2 0x4CD4
#define CRC16A_RES3 0x4EED
	if (Crc16ATab(CrcPat1, CRCPAT1LEN) != CRC16A_RES1) return False;
	if (Crc16ATab(CrcPat2, CRCPAT2LEN) != CRC16A_RES2) return False;
	if (Crc16ATab(CrcPat3, CRCPAT3LEN) != CRC16A_RES3) return False;

	if (Crc16AFast(CrcPat1, CRCPAT1LEN) != CRC16A_RES1) return False;
	if (Crc16AFast(CrcPat2, CRCPAT2LEN) != CRC16A_RES2) return False;
	if (Crc16AFast(CrcPat3, CRCPAT3LEN) != CRC16A_RES3) return False;

	if (Crc16ASlow(CrcPat1, CRCPAT1LEN) != CRC16A_RES1) return False;
	if (Crc16ASlow(CrcPat2, CRCPAT2LEN) != CRC16A_RES2) return False;
	if (Crc16ASlow(CrcPat3, CRCPAT3LEN) != CRC16A_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Crc16ADMA(CrcPat1, CRCPAT1LEN) != CRC16A_RES1) return False;
	if (Crc16ADMA(CrcPat2, CRCPAT2LEN) != CRC16A_RES2) return False;
	if (Crc16ADMA(CrcPat3, CRCPAT3LEN) != CRC16A_RES3) return False;
#endif // USE_DMA

	// CRC-16 buffer version
#define CRC16A_SPLIT 503
	crc = Crc16ABufTab(CRC16A_INIT, CrcPat3, CRC16A_SPLIT);
	crc = Crc16ABufTab(crc, (const u8*)CrcPat3+CRC16A_SPLIT, CRCPAT3LEN-CRC16A_SPLIT);
	if (crc != CRC16A_RES3) return False;

	crc = Crc16ABufFast(CRC16A_INIT, CrcPat3, CRC16A_SPLIT);
	crc = Crc16ABufFast(crc, (const u8*)CrcPat3+CRC16A_SPLIT, CRCPAT3LEN-CRC16A_SPLIT);
	if (crc != CRC16A_RES3) return False;

	crc = Crc16ABufSlow(CRC16A_INIT, CrcPat3, CRC16A_SPLIT);
	crc = Crc16ABufSlow(crc, (const u8*)CrcPat3+CRC16A_SPLIT, CRCPAT3LEN-CRC16A_SPLIT);
	if (crc != CRC16A_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Crc16ABufDMA(CRC16A_INIT, CrcPat3, CRC16A_SPLIT);
	crc = Crc16ABufDMA(crc, (const u8*)CrcPat3+CRC16A_SPLIT, CRCPAT3LEN-CRC16A_SPLIT);
	if (crc != CRC16A_RES3) return False;
#endif // USE_DMA

	// CRC-16 byte version
	crc = CRC16A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16AByteTab(crc, *src++);
	if (crc != CRC16A_RES3) return False;

	crc = CRC16A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16AByteFast(crc, *src++);
	if (crc != CRC16A_RES3) return False;

	crc = CRC16A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16AByteSlow(crc, *src++);
	if (crc != CRC16A_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = CRC16A_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16AByteDMA(crc, *src++);
	if (crc != CRC16A_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                       CRC-16 CCITT Reversed (CRC16A)
// ============================================================================

// CRC-16 CCITT Reversed (CRC16B) table (512 B)
const u16 Crc16BTable[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,

	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
};

// Check CRC-16 CCITT Reversed (CRC16B) table (returns False on error; can be used to generate table)
Bool Crc16BTableCheck()
{
	int i, j;
	u16 k;

	for(i = 0; i < 256; i++)
	{
		k = (u16)i;
		for (j = 8; j > 0; j--)
		{
			if ((k & 1) == 0)
				k >>= 1;
			else
				k = (u16)((k >> 1) ^ CRC16B_POLY);
		}
		// Crc16BTable[i] = k; // generate table
		if (Crc16BTable[i] != k) return False;
	}
	return True;
}

// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - tabled version (requires 512 B of flash memory)
u16 Crc16BByteTab(u16 crc, u8 data)
{
	crc = ~crc;
	return ~Crc16BTable[(crc ^ data) & 0xff] ^ (crc >> 8);
}

// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - slow version
u16 Crc16BByteSlow(u16 crc, u8 data)
{
	int i;
	crc = ~crc;
	crc ^= data;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 1) == 0)
			crc >>= 1;
		else
			crc = (u16)((crc >> 1) ^ CRC16B_POLY);
	}
	return ~crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B), 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BByteDMA(u16 crc, u8 data)
{
	return (u16)(DMA_CRC(DMA_CRC_CRC16REV | DMA_CRC_REV | DMA_CRC_INV,
		Reverse16(~crc), DMA_TEMP_CHAN(), &data, 1) >> 16);
}
#endif // USE_DMA

// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - tabled version (requires 512 B of flash memory)
u16 Crc16BBufTab(u16 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;

	for (; len > 0; len--)
	{
		// Reflect formula
		crc = Crc16BTable[(crc ^ *s++) & 0xff] ^ (crc >> 8);
	}
	return ~crc;
}

// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - slow version
u16 Crc16BBufSlow(u16 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	crc = ~crc;
	int i;

	for (; len > 0; len--)
	{
		crc ^= *s++;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (u16)((crc >> 1) ^ CRC16B_POLY);
		}
	}
	return ~crc;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B), buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BBufDMA(u16 crc, const void* buf, int len)
{
	return (u16)(DMA_CRC(DMA_CRC_CRC16REV | DMA_CRC_REV | DMA_CRC_INV,
			Reverse16(~crc), DMA_TEMP_CHAN(), buf, len) >> 16);
}
#endif // USE_DMA

// Calculate CRC-16 CCITT Reversed (CRC16B) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 160 us per 1 KB
u16 Crc16BTab(const void* buf, int len)
{
	return Crc16BBufTab(CRC16B_INIT, buf, len);
}

// Calculate CRC-16 CCITT Reversed (CRC16B) - slow version
//   Calculation speed: 880 us per 1 KB
u16 Crc16BSlow(const void* buf, int len)
{
	return Crc16BBufSlow(CRC16B_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Reversed (CRC16B) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16BDMA(const void* buf, int len)
{
	return Crc16BBufDMA(CRC16B_INIT, buf, len);
}
#endif // USE_DMA

// Check CRC-16 CCITT Reversed (CRC16B) calculations (returns False on error)
Bool Crc16BCheck()
{
	u16 crc;
	int i;
	const u8* src;

	// Check CRC-16 table
	if (!Crc16BTableCheck()) return False;

	// Check CRC-16-CCITT (0xFFFF) pattern
#define CRC16B_RES1 0x906E
#define CRC16B_RES2 0x7CBD
#define CRC16B_RES3 0x0C42
	if (Crc16BTab(CrcPat1, CRCPAT1LEN) != CRC16B_RES1) return False;
	if (Crc16BTab(CrcPat2, CRCPAT2LEN) != CRC16B_RES2) return False;
	if (Crc16BTab(CrcPat3, CRCPAT3LEN) != CRC16B_RES3) return False;

	if (Crc16BSlow(CrcPat1, CRCPAT1LEN) != CRC16B_RES1) return False;
	if (Crc16BSlow(CrcPat2, CRCPAT2LEN) != CRC16B_RES2) return False;
	if (Crc16BSlow(CrcPat3, CRCPAT3LEN) != CRC16B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Crc16BDMA(CrcPat1, CRCPAT1LEN) != CRC16B_RES1) return False;
	if (Crc16BDMA(CrcPat2, CRCPAT2LEN) != CRC16B_RES2) return False;
	if (Crc16BDMA(CrcPat3, CRCPAT3LEN) != CRC16B_RES3) return False;
#endif // USE_DMA

	// CRC-16 buffer version
#define CRC16B_SPLIT 503
	crc = Crc16BBufTab(CRC16B_INIT, CrcPat3, CRC16B_SPLIT);
	crc = Crc16BBufTab(crc, (const u8*)CrcPat3+CRC16B_SPLIT, CRCPAT3LEN-CRC16B_SPLIT);
	if (crc != CRC16B_RES3) return False;

	crc = Crc16BBufSlow(CRC16B_INIT, CrcPat3, CRC16B_SPLIT);
	crc = Crc16BBufSlow(crc, (const u8*)CrcPat3+CRC16B_SPLIT, CRCPAT3LEN-CRC16B_SPLIT);
	if (crc != CRC16B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Crc16BBufDMA(CRC16B_INIT, CrcPat3, CRC16B_SPLIT);
	crc = Crc16BBufDMA(crc, (const u8*)CrcPat3+CRC16B_SPLIT, CRCPAT3LEN-CRC16B_SPLIT);
	if (crc != CRC16B_RES3) return False;
#endif // USE_DMA

	// CRC-16 byte version
	crc = CRC16B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16BByteTab(crc, *src++);
	if (crc != CRC16B_RES3) return False;

	crc = CRC16B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16BByteSlow(crc, *src++);
	if (crc != CRC16B_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = CRC16B_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16BByteDMA(crc, *src++);
	if (crc != CRC16B_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                 CRC-16 CCITT Normal Alternative (CRC16C)
// ============================================================================

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - tabled version (requires 512 B of flash memory)
//   Calculation speed: 170 us per 1 KB
u16 Crc16CTab(const void* buf, int len)
{
	return Crc16CBufTab(CRC16C_INIT, buf, len);
}

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - fast version
//   Calculation speed: 200 us per 1 KB
u16 Crc16CFast(const void* buf, int len)
{
	return Crc16CBufFast(CRC16C_INIT, buf, len);
}

// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - slow version
//   Calculation speed: 1000 us per 1 KB
u16 Crc16CSlow(const void* buf, int len)
{
	return Crc16CBufSlow(CRC16C_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate CRC-16 CCITT Normal Alternative (CRC16C) - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   Calculation speed: 2 us per 1 KB
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u16 Crc16CDMA(const void* buf, int len)
{
	return Crc16CBufDMA(CRC16C_INIT, buf, len);
}
#endif // USE_DMA

// Check CRC-16 CCITT Normal Alternative (CRC16C) calculations (returns False on error)
Bool Crc16CCheck()
{
	u16 crc;
	int i;
	const u8* src;

	// Check CRC-16-CCITT (0x1D0F) pattern
#define CRC16C_RES1 0xE5CC
#define CRC16C_RES2 0x9144
#define CRC16C_RES3 0xE351
	if (Crc16CTab(CrcPat1, CRCPAT1LEN) != CRC16C_RES1) return False;
	if (Crc16CTab(CrcPat2, CRCPAT2LEN) != CRC16C_RES2) return False;
	if (Crc16CTab(CrcPat3, CRCPAT3LEN) != CRC16C_RES3) return False;

	if (Crc16CFast(CrcPat1, CRCPAT1LEN) != CRC16C_RES1) return False;
	if (Crc16CFast(CrcPat2, CRCPAT2LEN) != CRC16C_RES2) return False;
	if (Crc16CFast(CrcPat3, CRCPAT3LEN) != CRC16C_RES3) return False;

	if (Crc16CSlow(CrcPat1, CRCPAT1LEN) != CRC16C_RES1) return False;
	if (Crc16CSlow(CrcPat2, CRCPAT2LEN) != CRC16C_RES2) return False;
	if (Crc16CSlow(CrcPat3, CRCPAT3LEN) != CRC16C_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Crc16CDMA(CrcPat1, CRCPAT1LEN) != CRC16C_RES1) return False;
	if (Crc16CDMA(CrcPat2, CRCPAT2LEN) != CRC16C_RES2) return False;
	if (Crc16CDMA(CrcPat3, CRCPAT3LEN) != CRC16C_RES3) return False;
#endif // USE_DMA

	// CRC-16 buffer version
#define CRC16C_SPLIT 503
	crc = Crc16CBufTab(CRC16C_INIT, CrcPat3, CRC16C_SPLIT);
	crc = Crc16CBufTab(crc, (const u8*)CrcPat3+CRC16C_SPLIT, CRCPAT3LEN-CRC16C_SPLIT);
	if (crc != CRC16C_RES3) return False;

	crc = Crc16CBufFast(CRC16C_INIT, CrcPat3, CRC16C_SPLIT);
	crc = Crc16CBufFast(crc, (const u8*)CrcPat3+CRC16C_SPLIT, CRCPAT3LEN-CRC16C_SPLIT);
	if (crc != CRC16C_RES3) return False;

	crc = Crc16CBufSlow(CRC16C_INIT, CrcPat3, CRC16C_SPLIT);
	crc = Crc16CBufSlow(crc, (const u8*)CrcPat3+CRC16C_SPLIT, CRCPAT3LEN-CRC16C_SPLIT);
	if (crc != CRC16C_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Crc16CBufDMA(CRC16C_INIT, CrcPat3, CRC16C_SPLIT);
	crc = Crc16CBufDMA(crc, (const u8*)CrcPat3+CRC16C_SPLIT, CRCPAT3LEN-CRC16C_SPLIT);
	if (crc != CRC16C_RES3) return False;
#endif // USE_DMA

	// CRC-16 byte version
	crc = CRC16C_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16CByteTab(crc, *src++);
	if (crc != CRC16C_RES3) return False;

	crc = CRC16C_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16CByteFast(crc, *src++);
	if (crc != CRC16C_RES3) return False;

	crc = CRC16C_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16CByteSlow(crc, *src++);
	if (crc != CRC16C_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = CRC16C_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc16CByteDMA(crc, *src++);
	if (crc != CRC16C_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                            CRC-8 Dallas
// ============================================================================

// CRC-8 table (256 B)
const u8 Crc8Table[256] = {
	0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
	0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
	0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
	0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
	0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
	0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
	0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
	0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
	0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
	0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
	0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
	0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
	0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
	0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
	0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
	0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
	0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
	0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
	0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
	0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
	0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
	0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
	0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
	0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
	0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
	0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
	0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
	0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
	0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
	0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
	0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
	0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};

// Check CRC-8 table (returns False on error; can be used to generate table)
Bool Crc8TableCheck()
{
	int i, j;
	u8 crc;

	for (i = 0; i < 256; i++)
	{
		crc = (u8)i;
		for (j = 8; j > 0; j--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (crc >> 1) ^ CRC8_POLY;
		}
		if (Crc8Table[i] != crc) return False;
	}
	return True;
}

// Calculate CRC-8, 1 byte - tabled version (requires 256 B of flash memory)
u8 Crc8ByteTab(u8 crc, u8 data)
{
	return Crc8Table[crc ^ data];
}	

// Calculate CRC-8, 1 byte - slow version
u8 Crc8ByteSlow(u8 crc, u8 data)
{
	int i;
	crc ^= data;
	for (i = 8; i > 0; i--)
	{
		if ((crc & 1) == 0)
			crc >>= 1;
		else
			crc = (crc >> 1) ^ CRC8_POLY;
	}
	return crc;
}

// Calculate CRC-8, buffer - tabled version (requires 256 B of flash memory)
u8 Crc8BufTab(u8 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	for (; len > 0; len--)
	{
		crc = Crc8Table[crc ^ *s++];
	}
	return crc;
}

// Calculate CRC-8, buffer - slow version
u8 Crc8BufSlow(u8 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	int i;
	for (; len > 0; len--)
	{
		crc ^= *s++;
		for (i = 8; i > 0; i--)
		{
			if ((crc & 1) == 0)
				crc >>= 1;
			else
				crc = (crc >> 1) ^ CRC8_POLY;
		}
	}
	return crc;
}

// Calculate CRC-8 - tabled version (requires 256 B of flash memory)
//   Calculation speed: 115 us per 1 KB
u8 Crc8Tab(const void* buf, int len)
{
	return Crc8BufTab(CRC8_INIT, buf, len);
}

// Calculate CRC-8 - slow version
//   Calculation speed: 820 us per 1 KB
u8 Crc8Slow(const void* buf, int len)
{
	return Crc8BufSlow(CRC8_INIT, buf, len);
}

// Check CRC-8 calculations (returns False on error)
Bool Crc8Check()
{
	u8 crc;
	int i;
	const u8* src;

	// Check CRC-8 table
	if (!Crc8TableCheck()) return False;

	// Check CRC-8 pattern
#define CRC8_RES1 0xA1
#define CRC8_RES2 0xF1
#define CRC8_RES3 0x1E
	if (Crc8Tab(CrcPat1, CRCPAT1LEN) != CRC8_RES1) return False;
	if (Crc8Tab(CrcPat2, CRCPAT2LEN) != CRC8_RES2) return False;
	if (Crc8Tab(CrcPat3, CRCPAT3LEN) != CRC8_RES3) return False;

	if (Crc8Slow(CrcPat1, CRCPAT1LEN) != CRC8_RES1) return False;
	if (Crc8Slow(CrcPat2, CRCPAT2LEN) != CRC8_RES2) return False;
	if (Crc8Slow(CrcPat3, CRCPAT3LEN) != CRC8_RES3) return False;

	// CRC-8 buffer version
#define CRC8_SPLIT 503
	crc = Crc8BufTab(CRC8_INIT, CrcPat3, CRC8_SPLIT);
	crc = Crc8BufTab(crc, (const u8*)CrcPat3+CRC8_SPLIT, CRCPAT3LEN-CRC8_SPLIT);
	if (crc != CRC8_RES3) return False;

	crc = Crc8BufSlow(CRC8_INIT, CrcPat3, CRC8_SPLIT);
	crc = Crc8BufSlow(crc, (const u8*)CrcPat3+CRC8_SPLIT, CRCPAT3LEN-CRC8_SPLIT);
	if (crc != CRC8_RES3) return False;
	
	// CRC-8 byte version
	crc = CRC8_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc8ByteTab(crc, *src++);
	if (crc != CRC8_RES3) return False;

	crc = CRC8_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Crc8ByteSlow(crc, *src++);
	if (crc != CRC8_RES3) return False;

	return True;
}

// ============================================================================
//                                   Parity
// ============================================================================

// Calculate parity, 1 byte - software version
u8 ParityByteSoft(u8 par, u8 data)
{
	int i;
	for (i = 8; i > 0; i--)
	{
		if ((data & 1) != 0) par ^= 1;
		data >>= 1;
	}
	return par;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity, 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u8 ParityByteDMA(u8 par, u8 data)
{
	return (u8)DMA_CRC(DMA_CRC_XOR, par, DMA_TEMP_CHAN(), &data, 1);
}
#endif // USE_DMA

// Calculate parity, buffer - software version
u8 ParityBufSoft(u8 par, const void* buf, int len)
{
	int i;
	u8 data = 0;
	const u8* s = (const u8*)buf;
	for (; len > 0; len--) data ^= *s++;
	for (i = 8; i > 0; i--)
	{
		if ((data & 1) != 0) par ^= 1;
		data >>= 1;
	}
	return par;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u8 ParityBufDMA(u8 par, const void* buf, int len)
{
	return (u8)DMA_CRC(DMA_CRC_XOR, par, DMA_TEMP_CHAN(), buf, len);
}
#endif // USE_DMA

// Calculate parity - software version
//   Calculation speed: 90 us per 1 KB
u8 ParitySoft(const void* buf, int len)
{
	return ParityBufSoft(PARITY_INIT, buf, len);
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate parity - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   Calculation speed: 2 us per 1 KB
u8 ParityDMA(const void* buf, int len)
{
	return ParityBufDMA(PARITY_INIT, buf, len);
}
#endif // USE_DMA

// check parity calculation (returns False on error)
Bool ParityCheck()
{
	u8 crc;
	int i;
	const u8* src;

	// Check parity pattern
#define CRCPAR_RES1 1
#define CRCPAR_RES2 1
#define CRCPAR_RES3 0
	if (ParitySoft(CrcPat1, CRCPAT1LEN) != CRCPAR_RES1) return False;
	if (ParitySoft(CrcPat2, CRCPAT2LEN) != CRCPAR_RES2) return False;
	if (ParitySoft(CrcPat3, CRCPAT3LEN) != CRCPAR_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (ParityDMA(CrcPat1, CRCPAT1LEN) != CRCPAR_RES1) return False;
	if (ParityDMA(CrcPat2, CRCPAT2LEN) != CRCPAR_RES2) return False;
	if (ParityDMA(CrcPat3, CRCPAT3LEN) != CRCPAR_RES3) return False;
#endif // USE_DMA

	// Parity buffer version
#define CRCPAR_SPLIT 503
	crc = ParityBufSoft(PARITY_INIT, CrcPat3, CRCPAR_SPLIT);
	crc = ParityBufSoft(crc, (const u8*)CrcPat3+CRCPAR_SPLIT, CRCPAT3LEN-CRCPAR_SPLIT);
	if (crc != CRCPAR_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = ParityBufDMA(PARITY_INIT, CrcPat3, CRCPAR_SPLIT);
	crc = ParityBufDMA(crc, (const u8*)CrcPat3+CRCPAR_SPLIT, CRCPAT3LEN-CRCPAR_SPLIT);
	if (crc != CRCPAR_RES3) return False;
#endif // USE_DMA

	// Parity byte version
	crc = PARITY_INIT;
	src = (const u8*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN; i++) crc = ParityByteSoft(crc, *src++);
	if (crc != CRCPAR_RES1) return False;

	crc = PARITY_INIT;
	src = (const u8*)CrcPat2;
	for (i = 0; i < CRCPAT2LEN; i++) crc = ParityByteSoft(crc, *src++);
	if (crc != CRCPAR_RES2) return False;

	crc = PARITY_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = ParityByteSoft(crc, *src++);
	if (crc != CRCPAR_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = PARITY_INIT;
	src = (const u8*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN; i++) crc = ParityByteDMA(crc, *src++);
	if (crc != CRCPAR_RES1) return False;

	crc = PARITY_INIT;
	src = (const u8*)CrcPat2;
	for (i = 0; i < CRCPAT2LEN; i++) crc = ParityByteDMA(crc, *src++);
	if (crc != CRCPAR_RES2) return False;

	crc = PARITY_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = ParityByteDMA(crc, *src++);
	if (crc != CRCPAR_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//            Simple checksum on 8-bit data with 32-bit result
// ============================================================================

// Calculate 8-bit checksum, 1 byte - software version
u32 Sum8ByteSoft(u32 sum, u8 data)
{
	return sum + data;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum, 1 byte - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum8ByteDMA(u32 sum, u8 data)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), &data, 1, DMA_SIZE_8);
}
#endif // USE_DMA

// Calculate 8-bit checksum, buffer - software version
u32 Sum8BufSoft(u32 sum, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	for (; len > 0; len--) sum += *s++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum8BufDMA(u32 sum, const void* buf, int len)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), buf, len, DMA_SIZE_8);
}
#endif // USE_DMA

// Calculate 8-bit checksum - software version
//   Calculation speed: 100 us per 1 KB
u32 Sum8Soft(const void* buf, int len)
{
	u32 sum = SUM8_INIT;
	const u8* s = (const u8*)buf;
	for (; len > 0; len--) sum += *s++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 8-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   Calculation speed: 8 us per 1 KB
u32 Sum8DMA(const void* buf, int len)
{
	return Sum8BufDMA(SUM8_INIT, buf, len);
}
#endif // USE_DMA

// check 8-bit checksum calculation (returns False on error)
Bool Sum8Check()
{
	u32 crc;
	int i;
	const u8* src;

	// Check sum8 pattern
#define SUM8_RES1 0x000001DD
#define SUM8_RES2 0x0000014B
#define SUM8_RES3 0x0001FE00
	if (Sum8Soft(CrcPat1, CRCPAT1LEN) != SUM8_RES1) return False;
	if (Sum8Soft(CrcPat2, CRCPAT2LEN) != SUM8_RES2) return False;
	if (Sum8Soft(CrcPat3, CRCPAT3LEN) != SUM8_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Sum8DMA(CrcPat1, CRCPAT1LEN) != SUM8_RES1) return False;
	if (Sum8DMA(CrcPat2, CRCPAT2LEN) != SUM8_RES2) return False;
	if (Sum8DMA(CrcPat3, CRCPAT3LEN) != SUM8_RES3) return False;
#endif // USE_DMA

	// Sum8 buffer version
#define SUM8_SPLIT 503
	crc = Sum8BufSoft(SUM8_INIT, CrcPat3, SUM8_SPLIT);
	crc = Sum8BufSoft(crc, (const u8*)CrcPat3+SUM8_SPLIT, CRCPAT3LEN-SUM8_SPLIT);
	if (crc != SUM8_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Sum8BufDMA(SUM8_INIT, CrcPat3, SUM8_SPLIT);
	crc = Sum8BufDMA(crc, (const u8*)CrcPat3+SUM8_SPLIT, CRCPAT3LEN-SUM8_SPLIT);
	if (crc != SUM8_RES3) return False;
#endif // USE_DMA

	// Sum8 byte version
	crc = SUM8_INIT;
	src = (const u8*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN; i++) crc = Sum8ByteSoft(crc, *src++);
	if (crc != SUM8_RES1) return False;

	crc = SUM8_INIT;
	src = (const u8*)CrcPat2;
	for (i = 0; i < CRCPAT2LEN; i++) crc = Sum8ByteSoft(crc, *src++);
	if (crc != SUM8_RES2) return False;

	crc = SUM8_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Sum8ByteSoft(crc, *src++);
	if (crc != SUM8_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = SUM8_INIT;
	src = (const u8*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN; i++) crc = Sum8ByteDMA(crc, *src++);
	if (crc != SUM8_RES1) return False;

	crc = SUM8_INIT;
	src = (const u8*)CrcPat2;
	for (i = 0; i < CRCPAT2LEN; i++) crc = Sum8ByteDMA(crc, *src++);
	if (crc != SUM8_RES2) return False;

	crc = SUM8_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = Sum8ByteDMA(crc, *src++);
	if (crc != SUM8_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//            Simple checksum on 16-bit data with 32-bit result
// ============================================================================

// Calculate 16-bit checksum, 1 word - software version
u32 Sum16WordSoft(u32 sum, u16 data)
{
	return sum + data;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum, 1 word - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum16WordDMA(u32 sum, u16 data)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), &data, 1, DMA_SIZE_16);
}
#endif // USE_DMA

// Calculate 16-bit checksum, buffer - software version
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
u32 Sum16BufSoft(u32 sum, const u16* buf, int num)
{
	for (; num > 0; num--) sum += *buf++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum16BufDMA(u32 sum, const u16* buf, int num)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), buf, num, DMA_SIZE_16);
}
#endif // USE_DMA

// Calculate 16-bit checksum - software version
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Calculation speed: 48 us per 1 KB
u32 Sum16Soft(const u16* buf, int num)
{
	u32 sum = SUM16_INIT;
	for (; num > 0; num--) sum += *buf++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 16-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   data = pointer to data (must be aligned to u16 entry)
//   num = number of u16 entries
// Calculation speed: 4 us per 1 KB
u32 Sum16DMA(const u16* buf, int num)
{
	return Sum16BufDMA(SUM16_INIT, buf, num);
}
#endif // USE_DMA

// check 16-bit checksum calculation (returns False on error)
Bool Sum16Check()
{
	u32 crc;
	int i;
	const u16* src;

	// Check sum16 pattern
#define SUM16_RES1 0x0000D4D0
#define SUM16_RES2 0x00004446
#define SUM16_RES3 0x00FFFF00
	if (Sum16Soft((const u16*)CrcPat1, CRCPAT1LEN/2) != SUM16_RES1) return False;
	if (Sum16Soft((const u16*)CrcPat2, (CRCPAT2LEN+1)/2) != SUM16_RES2) return False;
	if (Sum16Soft((const u16*)CrcPat3, (CRCPAT3LEN+1)/2) != SUM16_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Sum16DMA((const u16*)CrcPat1, CRCPAT1LEN/2) != SUM16_RES1) return False;
	if (Sum16DMA((const u16*)CrcPat2, (CRCPAT2LEN+1)/2) != SUM16_RES2) return False;
	if (Sum16DMA((const u16*)CrcPat3, (CRCPAT3LEN+1)/2) != SUM16_RES3) return False;
#endif // USE_DMA

	// Sum16 buffer version
#define SUM16_SPLIT ((503+1)/2)
	crc = Sum16BufSoft(SUM16_INIT, (const u16*)CrcPat3, SUM16_SPLIT);
	crc = Sum16BufSoft(crc, (const u16*)CrcPat3+SUM16_SPLIT, (CRCPAT3LEN+1)/2-SUM16_SPLIT);
	if (crc != SUM16_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Sum16BufDMA(SUM16_INIT, (const u16*)CrcPat3, SUM16_SPLIT);
	crc = Sum16BufDMA(crc, (const u16*)CrcPat3+SUM16_SPLIT, (CRCPAT3LEN+1)/2-SUM16_SPLIT);
	if (crc != SUM16_RES3) return False;
#endif // USE_DMA

	// Sum16 word version
	crc = SUM16_INIT;
	src = (const u16*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN/2; i++) crc = Sum16WordSoft(crc, *src++);
	if (crc != SUM16_RES1) return False;

	crc = SUM16_INIT;
	src = (const u16*)CrcPat2;
	for (i = 0; i < (CRCPAT2LEN+1)/2; i++) crc = Sum16WordSoft(crc, *src++);
	if (crc != SUM16_RES2) return False;

	crc = SUM16_INIT;
	src = (const u16*)CrcPat3;
	for (i = 0; i < (CRCPAT3LEN+1)/2; i++) crc = Sum16WordSoft(crc, *src++);
	if (crc != SUM16_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = SUM16_INIT;
	src = (const u16*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN/2; i++) crc = Sum16WordDMA(crc, *src++);
	if (crc != SUM16_RES1) return False;

	crc = SUM16_INIT;
	src = (const u16*)CrcPat2;
	for (i = 0; i < (CRCPAT2LEN+1)/2; i++) crc = Sum16WordDMA(crc, *src++);
	if (crc != SUM16_RES2) return False;

	crc = SUM16_INIT;
	src = (const u16*)CrcPat3;
	for (i = 0; i < (CRCPAT3LEN+1)/2; i++) crc = Sum16WordDMA(crc, *src++);
	if (crc != SUM16_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//            Simple checksum on 32-bit data with 32-bit result
// ============================================================================

// Calculate 32-bit checksum, 1 double word - software version
u32 Sum32DWordSoft(u32 sum, u32 data)
{
	return sum + data;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum, 1 double word - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum32DWordDMA(u32 sum, u32 data)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), &data, 1, DMA_SIZE_32);
}
#endif // USE_DMA

// Calculate 32-bit checksum, buffer - software version
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
u32 Sum32BufSoft(u32 sum, const u32* buf, int num)
{
	for (; num > 0; num--) sum += *buf++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum, buffer - DMA version (uses DMA_TEMP_CHAN() temporary channel)
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
u32 Sum32BufDMA(u32 sum, const u32* buf, int num)
{
	return DMA_SUM(DMA_CRC_SUM, sum, DMA_TEMP_CHAN(), buf, num, DMA_SIZE_32);
}
#endif // USE_DMA

// Calculate 32-bit checksum - software version
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Calculation speed: 22 us per 1 KB
u32 Sum32Soft(const u32* buf, int num)
{
	u32 sum = SUM32_INIT;
	for (; num > 0; num--) sum += *buf++;
	return sum;
}

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
// Calculate 32-bit checksum - DMA version (uses DMA_TEMP_CHAN() temporary channel)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
//   data = pointer to data (must be aligned to u32 entry)
//   num = number of u32 entries
// Calculation speed: 2 us per 1 KB
u32 Sum32DMA(const u32* buf, int num)
{
	return Sum32BufDMA(SUM32_INIT, buf, num);
}
#endif // USE_DMA

// check 32-bit checksum calculation (returns False on error)
Bool Sum32Check()
{
	u32 crc;
	int i;
	const u32* src;

	// Check sum32 pattern
#define SUM32_RES1 0x6C6A6866
#define SUM32_RES2 0x3E4A05FC
#define SUM32_RES3 0xFFFFFF80
	if (Sum32Soft((const u32*)CrcPat1, CRCPAT1LEN/4) != SUM32_RES1) return False;
	if (Sum32Soft((const u32*)CrcPat2, (CRCPAT2LEN+3)/4) != SUM32_RES2) return False;
	if (Sum32Soft((const u32*)CrcPat3, (CRCPAT3LEN+3)/4) != SUM32_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	if (Sum32DMA((const u32*)CrcPat1, CRCPAT1LEN/4) != SUM32_RES1) return False;
	if (Sum32DMA((const u32*)CrcPat2, (CRCPAT2LEN+3)/4) != SUM32_RES2) return False;
	if (Sum32DMA((const u32*)CrcPat3, (CRCPAT3LEN+3)/4) != SUM32_RES3) return False;
#endif // USE_DMA

	// Sum32 buffer version
#define SUM32_SPLIT ((503+3)/4)
	crc = Sum32BufSoft(SUM32_INIT, (const u32*)CrcPat3, SUM32_SPLIT);
	crc = Sum32BufSoft(crc, (const u32*)CrcPat3+SUM32_SPLIT, (CRCPAT3LEN+3)/4-SUM32_SPLIT);
	if (crc != SUM32_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = Sum32BufDMA(SUM32_INIT, (const u32*)CrcPat3, SUM32_SPLIT);
	crc = Sum32BufDMA(crc, (const u32*)CrcPat3+SUM32_SPLIT, (CRCPAT3LEN+3)/4-SUM32_SPLIT);
	if (crc != SUM32_RES3) return False;
#endif // USE_DMA

	// Sum32 dword version
	crc = SUM32_INIT;
	src = (const u32*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN/4; i++) crc = Sum32DWordSoft(crc, *src++);
	if (crc != SUM32_RES1) return False;

	crc = SUM32_INIT;
	src = (const u32*)CrcPat2;
	for (i = 0; i < (CRCPAT2LEN+3)/4; i++) crc = Sum32DWordSoft(crc, *src++);
	if (crc != SUM32_RES2) return False;

	crc = SUM32_INIT;
	src = (const u32*)CrcPat3;
	for (i = 0; i < (CRCPAT3LEN+3)/4; i++) crc = Sum32DWordSoft(crc, *src++);
	if (crc != SUM32_RES3) return False;

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
	crc = SUM32_INIT;
	src = (const u32*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN/4; i++) crc = Sum32DWordDMA(crc, *src++);
	if (crc != SUM32_RES1) return False;

	crc = SUM32_INIT;
	src = (const u32*)CrcPat2;
	for (i = 0; i < (CRCPAT2LEN+3)/4; i++) crc = Sum32DWordDMA(crc, *src++);
	if (crc != SUM32_RES2) return False;

	crc = SUM32_INIT;
	src = (const u32*)CrcPat3;
	for (i = 0; i < (CRCPAT3LEN+3)/4; i++) crc = Sum32DWordDMA(crc, *src++);
	if (crc != SUM32_RES3) return False;
#endif // USE_DMA

	return True;
}

// ============================================================================
//                                  CRC-XOR
// ============================================================================

// Calculate CRC-XOR, 1 byte
u16 CrcXorByte(u16 crc, u8 data)
{
	crc = (crc << 1) | (crc >> 15); // rotate 1 bit left with carry to lowest bit
	crc ^= data;
	return crc;
}

// Calculate CRC-XOR, buffer
u16 CrcXorBuf(u16 crc, const void* buf, int len)
{
	const u8* s = (const u8*)buf;
	for (; len > 0; len--)
	{
		crc = (crc << 1) | (crc >> 15); // rotate 1 bit left with carry to lowest bit
		crc ^= *s++;
	}
	return crc;
}

// Calculate CRC-XOR
//   Calculation speed: 160 us per 1 KB
u16 CrcXor(const void* buf, int len)
{
	return CrcXorBuf(CRCXOR_INIT, buf, len);
}

// check Xor checksum calculation (returns False on error)
Bool CrcXorCheck()
{
	u16 crc;
	int i;
	const u8* src;

	// Check sum8 pattern
#define CRCXOR_RES1 0x2035
#define CRCXOR_RES2 0x03B0
#define CRCXOR_RES3 0x0000
	if (CrcXor(CrcPat1, CRCPAT1LEN) != CRCXOR_RES1) return False;
	if (CrcXor(CrcPat2, CRCPAT2LEN) != CRCXOR_RES2) return False;
	if (CrcXor(CrcPat3, CRCPAT3LEN) != CRCXOR_RES3) return False;

	// CrcXor buffer version
#define CRCXOR_SPLIT 503
	crc = CrcXorBuf(CRCXOR_INIT, CrcPat3, CRCXOR_SPLIT);
	crc = CrcXorBuf(crc, (const u8*)CrcPat3+CRCXOR_SPLIT, CRCPAT3LEN-CRCXOR_SPLIT);
	if (crc != CRCXOR_RES3) return False;

	// CrcXor byte version
	crc = CRCXOR_INIT;
	src = (const u8*)CrcPat1;
	for (i = 0; i < CRCPAT1LEN; i++) crc = CrcXorByte(crc, *src++);
	if (crc != CRCXOR_RES1) return False;

	crc = CRCXOR_INIT;
	src = (const u8*)CrcPat2;
	for (i = 0; i < CRCPAT2LEN; i++) crc = CrcXorByte(crc, *src++);
	if (crc != CRCXOR_RES2) return False;

	crc = CRCXOR_INIT;
	src = (const u8*)CrcPat3;
	for (i = 0; i < CRCPAT3LEN; i++) crc = CrcXorByte(crc, *src++);
	if (crc != CRCXOR_RES3) return False;

	return True;
}

#endif // USE_CRC	// use CRC Check Sum (lib_crc.c, lib_crc.h)
