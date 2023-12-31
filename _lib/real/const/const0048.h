
// ****************************************************************************
//
//                           Constant Templates of real48
//
// ****************************************************************************
// PicoLibSDK, Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz

// real48: size 6 bytes, 48 bits, mantissa 37 bits, exponent 10 bits

// Normal precision:
const REAL REALNAME(Const0) =	{ { 0x0000, 0x0000, 0x0000 }, };	// 0  (0)
const REAL REALNAME(Const1) =	{ { 0x0000, 0x0000, 0x3fe0 }, };	// 1  (1)
const REAL REALNAME(ConstM1) =	{ { 0x0000, 0x0000, 0xbfe0 }, };	// -1  (-1)
const REAL REALNAME(Const2) =	{ { 0x0000, 0x0000, 0x4000 }, };	// 2  (2)
const REAL REALNAME(Const3) =	{ { 0x0000, 0x0000, 0x4010 }, };	// 3  (3)
const REAL REALNAME(Const05) =	{ { 0x0000, 0x0000, 0x3fc0 }, };	// 0.5  (0.5)
const REAL REALNAME(ConstM05) =	{ { 0x0000, 0x0000, 0xbfc0 }, };	// -0.5  (-0.5)
const REAL REALNAME(Const075) =	{ { 0x0000, 0x0000, 0x3fd0 }, };	// 0.75  (0.75)
const REAL REALNAME(Const83) =	{ { 0xaaab, 0xaaaa, 0x400a }, };	// 8/3  (2.66666666666667)
const REAL REALNAME(Const10) =	{ { 0x0000, 0x0000, 0x4048 }, };	// 10  (10)
const REAL REALNAME(Const100) =	{ { 0x0000, 0x0000, 0x40b2 }, };	// 100  (100)
const REAL REALNAME(ConstExpMax) =	{ { 0xbaf5, 0x6657, 0x40ec }, };	// exp(x) max  (355.198209266132)
const REAL REALNAME(ConstExpMin) =	{ { 0xbaf5, 0x6657, 0xc0ec }, };	// exp(x) min  (-355.198209266132)
const REAL REALNAME(Const01) =	{ { 0x3333, 0x3333, 0x3f73 }, };	// 0.1 (1e-1)  (0.1)
const REAL REALNAME(Const001) =	{ { 0x8f5c, 0xf5c2, 0x3f08 }, };	// 0.01 (1e-2)  (0.01)
const REAL REALNAME(Const1eM4) =	{ { 0xd639, 0x6dc5, 0x3e34 }, };	// 0.0001 (1e-4)  (0.0001)
const REAL REALNAME(Const1eM8) =	{ { 0xc461, 0xf31d, 0x3c8a }, };	// 0.00000001 (1e-8)  (1e-008)
const REAL REALNAME(ConstLn2) =	{ { 0xfdf4, 0x5c85, 0x3fcc }, };	// ln(2)  (0.693147180559945)
const REAL REALNAME(ConstRLn2) =	{ { 0xca57, 0x2a8e, 0x3fee }, };	// 1/ln(2)  (1.44269504088896)
const REAL REALNAME(ConstLn10) =	{ { 0x776b, 0xd763, 0x4004 }, };	// ln(10)  (2.30258509299405)
const REAL REALNAME(ConstRLn10) =	{ { 0x2a4e, 0x96f6, 0x3fb7 }, };	// 1/ln(10)  (0.434294481903252)
const REAL REALNAME(ConstLog2) =	{ { 0xa13f, 0x8826, 0x3fa6 }, };	// log(2)  (0.301029995663981)
const REAL REALNAME(ConstRLog2) =	{ { 0x12f3, 0x269e, 0x4015 }, };	// 1/log(2)  (3.32192809488736)
const REAL REALNAME(ConstEul) =	{ { 0x1629, 0x7e15, 0x400b }, };	// Eul  (2.71828182845905)
const REAL REALNAME(ConstPi05) =	{ { 0xa888, 0x43f6, 0x3ff2 }, };	// pi/2  (1.5707963267949)
const REAL REALNAME(ConstMPi05) =	{ { 0xa888, 0x43f6, 0xbff2 }, };	// -pi/2  (-1.5707963267949)
const REAL REALNAME(ConstPi) =	{ { 0xa888, 0x43f6, 0x4012 }, };	// pi  (3.14159265358979)
const REAL REALNAME(ConstMPi) =	{ { 0xa888, 0x43f6, 0xc012 }, };	// -pi  (-3.14159265358979)
const REAL REALNAME(ConstPi2) =	{ { 0xa888, 0x43f6, 0x4032 }, };	// pi*2  (6.28318530717959)
const REAL REALNAME(ConstMPi2) =	{ { 0xa888, 0x43f6, 0xc032 }, };	// -pi*2  (-6.28318530717959)
const REAL REALNAME(ConstRPi2) =	{ { 0xdb94, 0xbe60, 0x3f88 }, };	// 1/pi*2  (0.159154943091895)
const REAL REALNAME(ConstLnPi22) =	{ { 0x90c9, 0xcfe3, 0x3fda }, };	// ln(pi*2)/2  (0.918938533204673)
const REAL REALNAME(Const180Pi) =	{ { 0x34c8, 0x4bb8, 0x4099 }, };	// 180/pi  (57.2957795130823)
const REAL REALNAME(ConstPi180) =	{ { 0x44a5, 0xbe8d, 0x3f23 }, };	// pi/180  (0.0174532925199433)
const REAL REALNAME(Const200Pi) =	{ { 0x5717, 0xa977, 0x409f }, };	// 200/pi  (63.6619772367581)
const REAL REALNAME(ConstPi200) =	{ { 0x242e, 0x2b7f, 0x3f20 }, };	// pi/200  (0.015707963267949)
const REAL REALNAME(ConstPhi) =	{ { 0x3730, 0xc6ef, 0x3ff3 }, };	// phi (sectio aurea = (1 + sqrt(5))/2)  (1.61803398874989)
const REAL REALNAME(ConstSqrt2) =	{ { 0xccfe, 0x413c, 0x3fed }, };	// sqrt(2)  (1.4142135623731)
const REAL REALNAME(ConstRSqrt2) =	{ { 0xccfe, 0x413c, 0x3fcd }, };	// 1/sqrt(2)  (0.707106781186548)

// Extended precision:
const REALEXT REALNAME(Const0Ext) =	{ { 0x0000, 0x0000, 0x0000 }, 0x00000000};	// 0  (0)
const REALEXT REALNAME(Const1Ext) =	{ { 0x0000, 0x0000, 0x8000 }, 0x3fffffff};	// 1  (1)
const REALEXT REALNAME(ConstM1Ext) =	{ { 0x0000, 0x0000, 0x8000 }, 0xbfffffff};	// -1  (-1)
const REALEXT REALNAME(Const2Ext) =	{ { 0x0000, 0x0000, 0x8000 }, 0x40000000};	// 2  (2)
const REALEXT REALNAME(Const3Ext) =	{ { 0x0000, 0x0000, 0xc000 }, 0x40000000};	// 3  (3)
const REALEXT REALNAME(Const05Ext) =	{ { 0x0000, 0x0000, 0x8000 }, 0x3ffffffe};	// 0.5  (0.5)
const REALEXT REALNAME(ConstM05Ext) =	{ { 0x0000, 0x0000, 0x8000 }, 0xbffffffe};	// -0.5  (-0.5)
const REALEXT REALNAME(Const075Ext) =	{ { 0x0000, 0x0000, 0xc000 }, 0x3ffffffe};	// 0.75  (0.75)
const REALEXT REALNAME(Const83Ext) =	{ { 0xaaab, 0xaaaa, 0xaaaa }, 0x40000000};	// 8/3  (2.66666666666667)
const REALEXT REALNAME(Const10Ext) =	{ { 0x0000, 0x0000, 0xa000 }, 0x40000002};	// 10  (10)
const REALEXT REALNAME(Const100Ext) =	{ { 0x0000, 0x0000, 0xc800 }, 0x40000005};	// 100  (100)
const REALEXT REALNAME(ConstExpMaxExt) =	{ { 0x463f, 0x17fa, 0xb172 }, 0x4000001b};	// exp(x) max  (372130559.284299)
const REALEXT REALNAME(ConstExpMinExt) =	{ { 0x463f, 0x17fa, 0xb172 }, 0xc000001b};	// exp(x) min  (-372130559.284299)
const REALEXT REALNAME(Const01Ext) =	{ { 0xcccd, 0xcccc, 0xcccc }, 0x3ffffffb};	// 0.1 (1e-1)  (0.1)
const REALEXT REALNAME(Const001Ext) =	{ { 0x70a4, 0x0a3d, 0xa3d7 }, 0x3ffffff8};	// 0.01 (1e-2)  (0.01)
const REALEXT REALNAME(Const1eM4Ext) =	{ { 0xe219, 0x1758, 0xd1b7 }, 0x3ffffff1};	// 0.0001 (1e-4)  (0.0001)
const REALEXT REALNAME(Const1eM8Ext) =	{ { 0x8462, 0x7711, 0xabcc }, 0x3fffffe4};	// 0.00000001 (1e-8)  (1e-008)
const REALEXT REALNAME(ConstLn2Ext) =	{ { 0xd1cf, 0x17f7, 0xb172 }, 0x3ffffffe};	// ln(2)  (0.693147180559945)
const REALEXT REALNAME(ConstRLn2Ext) =	{ { 0x5c18, 0x3b29, 0xb8aa }, 0x3fffffff};	// 1/ln(2)  (1.44269504088896)
const REALEXT REALNAME(ConstLn10Ext) =	{ { 0xaaa9, 0x8ddd, 0x935d }, 0x40000000};	// ln(10)  (2.30258509299405)
const REALEXT REALNAME(ConstRLn10Ext) =	{ { 0x3728, 0xd8a9, 0xde5b }, 0x3ffffffd};	// 1/ln(10)  (0.434294481903252)
const REALEXT REALNAME(ConstLog2Ext) =	{ { 0xfbd0, 0x9a84, 0x9a20 }, 0x3ffffffd};	// log(2)  (0.301029995663981)
const REALEXT REALNAME(ConstRLog2Ext) =	{ { 0xcd1c, 0x784b, 0xd49a }, 0x40000000};	// 1/log(2)  (3.32192809488736)
const REALEXT REALNAME(ConstEulExt) =	{ { 0xa2bb, 0x5458, 0xadf8 }, 0x40000000};	// Eul  (2.71828182845905)
const REALEXT REALNAME(ConstPi05Ext) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0x3fffffff};	// pi/2  (1.5707963267949)
const REALEXT REALNAME(ConstMPi05Ext) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0xbfffffff};	// -pi/2  (-1.5707963267949)
const REALEXT REALNAME(ConstPiExt) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0x40000000};	// pi  (3.14159265358979)
const REALEXT REALNAME(ConstMPiExt) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0xc0000000};	// -pi  (-3.14159265358979)
const REALEXT REALNAME(ConstPi2Ext) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0x40000001};	// pi*2  (6.28318530717959)
const REALEXT REALNAME(ConstMPi2Ext) =	{ { 0x2169, 0xdaa2, 0xc90f }, 0xc0000001};	// -pi*2  (-6.28318530717959)
const REALEXT REALNAME(ConstRPi2Ext) =	{ { 0x4e44, 0x836e, 0xa2f9 }, 0x3ffffffc};	// 1/pi*2  (0.159154943091895)
const REALEXT REALNAME(ConstLnPi22Ext) =	{ { 0x25f6, 0x8e43, 0xeb3f }, 0x3ffffffe};	// ln(pi*2)/2  (0.918938533204673)
const REALEXT REALNAME(Const180PiExt) =	{ { 0x1e10, 0xe0d3, 0xe52e }, 0x40000004};	// 180/pi  (57.2957795130823)
const REALEXT REALNAME(ConstPi180Ext) =	{ { 0x94ea, 0x3512, 0x8efa }, 0x3ffffff9};	// pi/180  (0.0174532925199433)
const REALEXT REALNAME(Const200PiExt) =	{ { 0x5a4a, 0xdd5c, 0xfea5 }, 0x40000004};	// 200/pi  (63.6619772367581)
const REALEXT REALNAME(ConstPi200Ext) =	{ { 0xb939, 0xfc90, 0x80ad }, 0x3ffffff9};	// pi/200  (0.015707963267949)
const REALEXT REALNAME(ConstPhiExt) =	{ { 0xbfa5, 0xbcdc, 0xcf1b }, 0x3fffffff};	// phi (sectio aurea = (1 + sqrt(5))/2)  (1.61803398874989)
const REALEXT REALNAME(ConstSqrt2Ext) =	{ { 0xf9de, 0xf333, 0xb504 }, 0x3fffffff};	// sqrt(2)  (1.4142135623731)
const REALEXT REALNAME(ConstRSqrt2Ext) =	{ { 0xf9de, 0xf333, 0xb504 }, 0x3ffffffe};	// 1/sqrt(2)  (0.707106781186548)

// Decimal exponents (index REAL_DECEXP = number '1')
const REAL REALNAME(ConstExp)[REAL_DECEXP*2+1] = {	// total 17 numbers
	 { { 0x9f19, 0x7411, 0x0ab7 }, },	// 1e-128 (1e-0x80)  (1e-128)
	 { { 0x89e9, 0x1ffa, 0x254a }, },	// 1e-64 (1e-0x40)  (1e-064)
	 { { 0xab51, 0xec47, 0x3293 }, },	// 1e-32 (1e-0x20)  (1e-032)
	 { { 0x2fb1, 0xa565, 0x3939 }, },	// 1e-16 (1e-0x10)  (1e-016)
	 { { 0xc461, 0xf31d, 0x3c8a }, },	// 1e-8 (1e-0x8)  (1e-008)
	 { { 0xd639, 0x6dc5, 0x3e34 }, },	// 1e-4 (1e-0x4)  (0.0001)
	 { { 0x8f5c, 0xf5c2, 0x3f08 }, },	// 1e-2 (1e-0x2)  (0.01)
	 { { 0x3333, 0x3333, 0x3f73 }, },	// 1e-1 (1e-0x1)  (0.1)
	 { { 0x0000, 0x0000, 0x3fe0 }, },	// 1e+0 (1e+0x0)  (1)
	 { { 0x0000, 0x0000, 0x4048 }, },	// 1e+1 (1e+0x1)  (10)
	 { { 0x0000, 0x0000, 0x40b2 }, },	// 1e+2 (1e+0x2)  (100)
	 { { 0x0000, 0x1000, 0x4187 }, },	// 1e+4 (1e+0x4)  (10000)
	 { { 0x0000, 0xaf08, 0x432f }, },	// 1e+8 (1e+0x8)  (100000000)
	 { { 0x6fc1, 0x86f2, 0x4683 }, },	// 1e+16 (1e+0x10)  (1e+016)
	 { { 0x6a0b, 0x716b, 0x4d27 }, },	// 1e+32 (1e+0x20)  (1e+032)
	 { { 0xd280, 0x9e07, 0x5a70 }, },	// 1e+64 (1e+0x40)  (1e+064)
	 { { 0xf260, 0xee91, 0x7504 }, },	// 1e+128 (1e+0x80)  (1e+128)
};

// Factorial coefficients
#ifdef FACT_COEFF
const REALEXT REALNAME(ConstFact)[FACT_COEFF] = {	// FACT_COEFF = 6
	 { { 0xaaab, 0xaaaa, 0xaaaa }, 0x3ffffffb},	// a0  (0.0833333333333333)
	 { { 0x8889, 0x8888, 0x8888 }, 0x3ffffffa},	// a1  (0.0333333333333333)
	 { { 0x3814, 0x1381, 0x8138 }, 0x3ffffffd},	// a2  (0.252380952380952)
	 { { 0xeec0, 0x2542, 0x868e }, 0x3ffffffe},	// a3  (0.525606469002695)
	 { { 0x608f, 0x9680, 0x8179 }, 0x3fffffff},	// a4  (1.01152306812684)
	 { { 0xd3dc, 0x9397, 0xc23c }, 0x3fffffff},	// a5  (1.51747364915329)
};
#endif

// Chebyshev coefficients of Ln()
#ifdef CHEB_LN
const REALEXT REALNAME(ConstChebLn)[CHEB_LN] = {	// CHEB_LN = 19
	 { { 0xfcc2, 0x1f65, 0xcf99 }, 0x3ffffffd},	// 0  (0.405465108108164)
	 { { 0xaaae, 0xaaaa, 0xaaaa }, 0x3ffffffd},	// 1  (0.333333333333339)
	 { { 0x8e41, 0x38e3, 0xe38e }, 0xbffffffa},	// 2  (-0.0555555555555574)
	 { { 0xa08c, 0x87e6, 0xca45 }, 0x3ffffff8},	// 3  (0.0123456790120222)
	 { { 0x9a68, 0x87e6, 0xca45 }, 0xbffffff6},	// 4  (-0.00308641975298374)
	 { { 0x19e0, 0xa220, 0xd7c1 }, 0x3ffffff4},	// 5  (0.000823045273159795)
	 { { 0x9ef7, 0xb427, 0xefba }, 0xbffffff2},	// 6  (-0.000228623687213539)
	 { { 0x159d, 0xf2f8, 0x88fc }, 0x3ffffff1},	// 7  (6.53210079259573e-005)
	 { { 0x49b7, 0xc57e, 0x9fd1 }, 0xbfffffef},	// 8  (-1.90519594858639e-005)
	 { { 0x154a, 0x1760, 0xbd6c }, 0x3fffffed},	// 9  (5.64522241589877e-006)
	 { { 0xee03, 0x9ffb, 0xe34e }, 0xbfffffeb},	// 10  (-1.69357008236381e-006)
	 { { 0xf457, 0x8714, 0x89a0 }, 0x3fffffea},	// 11  (5.1270075584019e-007)
	 { { 0x8d39, 0x6896, 0xa834 }, 0xbfffffe8},	// 12  (-1.56652853719555e-007)
	 { { 0xfdfe, 0xe6e6, 0xd256 }, 0x3fffffe6},	// 13  (4.89734720414714e-008)
	 { { 0x9590, 0x3d8b, 0x8242 }, 0xbfffffe5},	// 14  (-1.51641144889874e-008)
	 { { 0x61bb, 0x770a, 0x89d5 }, 0x3fffffe3},	// 15  (4.01149293363268e-009)
	 { { 0xd16b, 0x75f3, 0xabe0 }, 0xbfffffe1},	// 16  (-1.25056831028638e-009)
	 { { 0xd7ad, 0x1550, 0xcce4 }, 0x3fffffe0},	// 17  (7.45388934785871e-010)
	 { { 0xdd60, 0x14a0, 0x8160 }, 0xbfffffdf},	// 18  (-2.35332326636248e-010)
};
#endif

// Chebyshev coefficients of Exp()
#ifdef CHEB_EXP
const REALEXT REALNAME(ConstChebExp)[CHEB_EXP] = {	// CHEB_EXP = 15
	 { { 0x0000, 0x0000, 0x8000 }, 0x3fffffff},	// 0  (1)
	 { { 0xd1cf, 0x17f7, 0xb172 }, 0x3ffffffe},	// 1  (0.693147180559944)
	 { { 0x162c, 0xeffc, 0xf5fd }, 0x3ffffffc},	// 2  (0.2402265069591)
	 { { 0x2506, 0x46b8, 0xe358 }, 0x3ffffffa},	// 3  (0.0555041086648216)
	 { { 0xd274, 0x5b7d, 0x9d95 }, 0x3ffffff8},	// 4  (0.00961812910762849)
	 { { 0x53e1, 0xff3c, 0xaec3 }, 0x3ffffff5},	// 5  (0.00133335581464401)
	 { { 0x3676, 0x897c, 0xa184 }, 0x3ffffff2},	// 6  (0.000154035303933866)
	 { { 0xb645, 0xfe2a, 0xffe5 }, 0x3fffffee},	// 7  (1.52527337985191e-005)
	 { { 0xa3fb, 0x111c, 0xb160 }, 0x3fffffeb},	// 8  (1.32154867877456e-006)
	 { { 0x7dba, 0xa084, 0xda92 }, 0x3fffffe7},	// 9  (1.01780873631749e-007)
	 { { 0x42e9, 0xa9fe, 0xf267 }, 0x3fffffe3},	// 10  (7.05491220696451e-009)
	 { { 0x8fc8, 0xe56b, 0xf462 }, 0x3fffffdf},	// 11  (4.44536114034907e-010)
	 { { 0x5039, 0xf86f, 0xe1dc }, 0x3fffffdb},	// 12  (2.56776690792414e-011)
	 { { 0x1e1d, 0xb0d0, 0xc259 }, 0x3fffffd7},	// 13  (1.38094232370387e-012)
	 { { 0x9e8d, 0x1660, 0x99cb }, 0x3fffffd3},	// 14  (6.82979496079742e-014)
};
#endif

// Chebyshev coefficients of Sin()
#ifdef CHEB_SIN
const REALEXT REALNAME(ConstChebSin)[CHEB_SIN] = {	// CHEB_SIN = 14
	 { { 0xf9de, 0xf333, 0xb504 }, 0x3ffffffe},	// 0  (0.707106781186546)
	 { { 0xeb17, 0x18d6, 0x8e2c }, 0x3ffffffe},	// 1  (0.555360367269795)
	 { { 0x958f, 0xdb03, 0xdf52 }, 0xbffffffc},	// 2  (-0.218089506238711)
	 { { 0x2fa3, 0x2e0e, 0xe9dd }, 0xbffffffa},	// 3  (-0.0570956992187142)
	 { { 0x4843, 0x24d9, 0xb7ad }, 0x3ffffff8},	// 4  (0.0112107143260156)
	 { { 0x0ee6, 0x831c, 0xe6d0 }, 0x3ffffff5},	// 5  (0.00176097488841069)
	 { { 0x16e9, 0x585b, 0xf1b5 }, 0xbffffff2},	// 6  (-0.000230511073544306)
	 { { 0x03e6, 0xf9a4, 0xd8f4 }, 0xbfffffef},	// 7  (-2.58632819908316e-005)
	 { { 0x275a, 0xc3b6, 0xaa65 }, 0x3fffffec},	// 8  (2.53912087635807e-006)
	 { { 0x5483, 0x7b5c, 0xedeb }, 0x3fffffe8},	// 9  (2.21580128264016e-007)
	 { { 0x137a, 0x6965, 0x957a }, 0xbfffffe5},	// 10  (-1.74015493478458e-008)
	 { { 0x4458, 0x539f, 0xaac4 }, 0xbfffffe1},	// 11  (-1.24249273338168e-009)
	 { { 0xe6cb, 0x09b4, 0xb0ba }, 0x3fffffdd},	// 12  (8.03660034815372e-011)
	 { { 0xefae, 0x43cd, 0xab1a }, 0x3fffffd9},	// 13  (4.86302829795091e-012)
};
#endif

// Chebyshev coefficients of ASin()
#ifdef CHEB_ASIN
const REALEXT REALNAME(ConstChebASin)[CHEB_ASIN] = {	// CHEB_ASIN = 27
	 { { 0x4b17, 0xa76e, 0xc4cf }, 0x3ffffffd},	// 0  (0.38439677449564)
	 { { 0x4c01, 0x3c10, 0xcf1d }, 0x3ffffffd},	// 1  (0.404519917477957)
	 { { 0xc6cc, 0xcee0, 0x8790 }, 0x3ffffffa},	// 2  (0.0330970841572933)
	 { { 0x759d, 0x79d7, 0x86be }, 0x3ffffff9},	// 3  (0.0164482478827953)
	 { { 0xe5a6, 0xab57, 0xa966 }, 0x3ffffff7},	// 4  (0.00516970983862314)
	 { { 0xbbc6, 0x33d3, 0x9cfe }, 0x3ffffff6},	// 5  (0.00239552274045626)
	 { { 0xb3e2, 0xa575, 0x8834 }, 0x3ffffff5},	// 6  (0.00103916663906724)
	 { { 0x7d1f, 0xe8c5, 0x8336 }, 0x3ffffff4},	// 7  (0.000500543553582419)
	 { { 0x0198, 0x7202, 0xfe30 }, 0x3ffffff2},	// 8  (0.00024241374933389)
	 { { 0xf97f, 0xdf64, 0xff90 }, 0x3ffffff1},	// 9  (0.000121863321651705)
	 { { 0xb0fc, 0x6cb9, 0x8241 }, 0x3ffffff1},	// 10  (6.21106935821941e-005)
	 { { 0x24a8, 0x4cca, 0x8705 }, 0x3ffffff0},	// 11  (3.21914441506008e-005)
	 { { 0x705f, 0x1f84, 0x8dce }, 0x3fffffef},	// 12  (1.69044933815524e-005)
	 { { 0xb7a2, 0x0f5f, 0x99c8 }, 0x3fffffee},	// 13  (9.16609076184228e-006)
	 { { 0xc83b, 0xa951, 0xa50d }, 0x3fffffed},	// 14  (4.91897359073006e-006)
	 { { 0x7e2a, 0xe65f, 0x90f1 }, 0x3fffffec},	// 15  (2.15984763895423e-006)
	 { { 0x8614, 0xd471, 0x9bf3 }, 0x3fffffeb},	// 16  (1.16193637534845e-006)
	 { { 0x12c7, 0xe1a7, 0xd107 }, 0x3fffffeb},	// 17  (1.55740072530424e-006)
	 { { 0xe795, 0xb600, 0xeac8 }, 0x3fffffea},	// 18  (8.74638658588859e-007)
	 { { 0xced2, 0x8f99, 0xbc49 }, 0xbfffffea},	// 19  (-7.0142502868362e-007)
	 { { 0x46e8, 0xe0f8, 0xd9f4 }, 0xbfffffe9},	// 20  (-4.05975725068173e-007)
	 { { 0x18a3, 0x3d8c, 0xd9cf }, 0x3fffffea},	// 21  (8.11403739774671e-007)
	 { { 0xc985, 0xe4e0, 0xf8f1 }, 0x3fffffe9},	// 22  (4.63696007900437e-007)
	 { { 0x095d, 0x3a5c, 0xb143 }, 0xbfffffe9},	// 23  (-3.30177339251289e-007)
	 { { 0xa42f, 0x52f4, 0xcb30 }, 0xbfffffe8},	// 24  (-1.89234284500092e-007)
	 { { 0xf152, 0x58dd, 0xc11f }, 0x3fffffe7},	// 25  (8.99296485596573e-008)
	 { { 0xbce0, 0x04f9, 0xdcb7 }, 0x3fffffe6},	// 26  (5.13891968109386e-008)
};
#endif

// Chebyshev coefficients of ATan()
#ifdef CHEB_ATAN
const REALEXT REALNAME(ConstChebATan)[CHEB_ATAN] = {	// CHEB_ATAN = 19
	 { { 0xea06, 0xd42d, 0xded3 }, 0x3ffffffe},	// 0  (0.870419751367102)
	 { { 0xfd75, 0xa60c, 0xd0a4 }, 0xbffffffb},	// 1  (-0.101876542350221)
	 { { 0xdbe2, 0x805f, 0xaad1 }, 0x3ffffff9},	// 2  (0.020851851207109)
	 { { 0x02f1, 0xf172, 0xa4d9 }, 0xbffffff7},	// 3  (-0.00503086366008845)
	 { { 0xcc98, 0xbd41, 0xac6f }, 0x3ffffff5},	// 4  (0.00131558594957787)
	 { { 0x31c6, 0x322a, 0xbd42 }, 0xbffffff3},	// 5  (-0.000360982089909433)
	 { { 0x861e, 0x4fa6, 0xd67d }, 0x3ffffff1},	// 6  (0.000102276562032668)
	 { { 0x1107, 0xedac, 0xf8b8 }, 0xbfffffef},	// 7  (-2.96500178085531e-005)
	 { { 0x003a, 0x31ba, 0x92b7 }, 0x3fffffee},	// 8  (8.74493137104396e-006)
	 { { 0xf8c0, 0x231e, 0xaf6f }, 0xbfffffec},	// 9  (-2.61417224487175e-006)
	 { { 0xf953, 0x3d05, 0xd409 }, 0x3fffffea},	// 10  (7.89895979286009e-007)
	 { { 0x305a, 0x5696, 0x8124 }, 0xbfffffe9},	// 11  (-2.4054561966623e-007)
	 { { 0xeff4, 0x354f, 0x9ea0 }, 0x3fffffe7},	// 12  (7.38659005055463e-008)
	 { { 0x541e, 0x91b2, 0xc744 }, 0xbfffffe5},	// 13  (-2.31978306725651e-008)
	 { { 0x99f2, 0xd0bc, 0xf7b5 }, 0x3fffffe3},	// 14  (7.20931613816767e-009)
	 { { 0xc253, 0xe6af, 0x8337 }, 0xbfffffe2},	// 15  (-1.90947850565924e-009)
	 { { 0xfc55, 0xc965, 0xa41d }, 0x3fffffe0},	// 16  (5.97051819005797e-010)
	 { { 0x46a5, 0x87e8, 0xc4da }, 0xbfffffdf},	// 17  (-3.58074678445063e-010)
	 { { 0x6be0, 0x70b1, 0xf91c }, 0x3fffffdd},	// 18  (1.13282610327366e-010)
};
#endif

// Chebyshev coefficients of Sqrt()
#ifdef CHEB_SQRT
const REALEXT REALNAME(ConstChebSqrt)[CHEB_SQRT] = {	// CHEB_SQRT = 26
	 { { 0xd2da, 0xc1d6, 0xca62 }, 0x3fffffff},	// 0  (1.58113883008419)
	 { { 0x636d, 0xe89b, 0xf2dc }, 0x3ffffffd},	// 1  (0.474341649025257)
	 { { 0x9c30, 0xbec3, 0x91b7 }, 0xbffffffb},	// 2  (-0.0711512473531215)
	 { { 0x4ea3, 0x7e84, 0xaedc }, 0x3ffffff9},	// 3  (0.0213453742057549)
	 { { 0x7d16, 0x5eed, 0x8325 }, 0xbffffff8},	// 4  (-0.00800451536447733)
	 { { 0xc226, 0x4357, 0xdc53 }, 0x3ffffff6},	// 5  (0.00336189645873528)
	 { { 0xa0f7, 0xe8c3, 0xc64a }, 0xbffffff5},	// 6  (-0.00151285258636052)
	 { { 0xd1f4, 0x2a84, 0xbaf6 }, 0x3ffffff4},	// 7  (0.000713201851981496)
	 { { 0x4e0e, 0xde59, 0xb64a }, 0xbffffff3},	// 8  (-0.000347695265339201)
	 { { 0x209b, 0x0b99, 0xb64b }, 0x3ffffff2},	// 9  (0.000173848291133673)
	 { { 0x93b2, 0x3cff, 0xb9ce }, 0xbffffff1},	// 10  (-8.85990229897099e-005)
	 { { 0xa165, 0x654a, 0xc08c }, 0x3ffffff0},	// 11  (4.59071208443681e-005)
	 { { 0x6ea0, 0x8c61, 0xcc80 }, 0xbfffffef},	// 12  (-2.43785550639482e-005)
	 { { 0x2306, 0x1ae1, 0xd944 }, 0x3fffffee},	// 13  (1.29500648467371e-005)
	 { { 0x9f9c, 0x29eb, 0xcd9b }, 0xbfffffed},	// 14  (-6.12753952755347e-006)
	 { { 0xcd1b, 0x1cbe, 0xdc7d }, 0x3fffffec},	// 15  (3.28553795618908e-006)
	 { { 0x7710, 0xbf16, 0xe487 }, 0xbfffffec},	// 16  (-3.4053662347456e-006)
	 { { 0x7921, 0xc42e, 0xfeab }, 0x3fffffeb},	// 17  (1.89744652988518e-006)
	 { { 0xbffd, 0x3199, 0x9df7 }, 0x3fffffeb},	// 18  (1.1769354387736e-006)
	 { { 0x339a, 0x1733, 0xb7be }, 0xbfffffea},	// 19  (-6.84494307279586e-007)
	 { { 0x5f91, 0x31be, 0xe0ec }, 0xbfffffeb},	// 20  (-1.67580421289651e-006)
	 { { 0xa7a1, 0x0290, 0x8034 }, 0x3fffffeb},	// 21  (9.55188007203312e-007)
	 { { 0x257e, 0x49cd, 0xba20 }, 0x3fffffea},	// 22  (6.93373851922464e-007)
	 { { 0xb07d, 0x7781, 0xd501 }, 0xbfffffe9},	// 23  (-3.96754089325647e-007)
	 { { 0x626e, 0x9723, 0xda8b }, 0xbfffffe8},	// 24  (-2.03536148123656e-007)
	 { { 0xe3a0, 0x338b, 0xf93b }, 0x3fffffe7},	// 25  (1.16057347173936e-007)
};
#endif

// Cordic atan coefficients
#ifdef CORD_ATAN
const REAL REALNAME(ConstAtanInit) =	{ { 0x21af, 0x76d4, 0x4dba }, };	// init value of atan table (=1/expansion factor)  (0.607252935008881)
const REAL REALNAME(ConstAtan)[CORD_ATAN] = {	// CORD_ATAN = 18
	 { { 0x10b5, 0xed51, 0x6487 }, },	// 0  (0.785398163397448)
	 { { 0xc377, 0xce0a, 0x3b58 }, },	// 1  (0.463647609000806)
	 { { 0x2c81, 0x75f9, 0x1f5b }, },	// 2  (0.244978663126864)
	 { { 0x5618, 0xdd4d, 0x0fea }, },	// 3  (0.124354994546761)
	 { { 0xcb3f, 0x56ed, 0x07fd }, },	// 4  (0.0624188099959574)
	 { { 0x752f, 0xaab7, 0x03ff }, },	// 5  (0.0312398334302683)
	 { { 0xbbb7, 0xf555, 0x01ff }, },	// 6  (0.0156237286204768)
	 { { 0xadde, 0xfeaa, 0x00ff }, },	// 7  (0.00781234106010111)
	 { { 0x556f, 0xffd5, 0x007f }, },	// 8  (0.00390623013196697)
	 { { 0xaaab, 0xfffa, 0x003f }, },	// 9  (0.00195312251647882)
	 { { 0x5555, 0xffff, 0x001f }, },	// 10  (0.000976562189559319)
	 { { 0xeaab, 0xffff, 0x000f }, },	// 11  (0.000488281211194898)
	 { { 0xfd55, 0xffff, 0x0007 }, },	// 12  (0.000244140620149362)
	 { { 0xffab, 0xffff, 0x0003 }, },	// 13  (0.00012207031189367)
	 { { 0xfff5, 0xffff, 0x0001 }, },	// 14  (6.10351561742088e-005)
	 { { 0xffff, 0xffff, 0x0000 }, },	// 15  (3.05175781155261e-005)
	 { { 0x0000, 0x8000, 0x0000 }, },	// 16  (1.52587890613158e-005)
	 { { 0x0000, 0x4000, 0x0000 }, },	// 17  (7.62939453110197e-006)
};
#endif
