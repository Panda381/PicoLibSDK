
// test 8-bit addition
void Test_ADD8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ADD8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = ADD8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ADD8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("ADD8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit C-addition
void Test_ADC8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ADC8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = ADC8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ADC8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("ADC8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit subtraction
void Test_SUB8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SUB8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SUB8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SUB8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SUB8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit C-subtraction
void Test_SBB8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SBB8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SBB8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SBB8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SBB8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit AND
void Test_AND8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AND8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = AND8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AND8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AND8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit OR
void Test_OR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("OR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = OR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = OR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("OR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit XOR
void Test_XOR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("XOR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = XOR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = XOR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("XOR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit compare
void Test_CMP8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("CMP8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = CMP8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = CMP8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("CMP8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit TEST
void Test_TEST8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("TEST8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = TEST8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = TEST8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("TEST8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit INC
void Test_INC8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("INC8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = INC8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = INC8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("INC8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit DEC
void Test_DEC8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DEC8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = DEC8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = DEC8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("DEC8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test jumps
void Test_JMP()
{
	u32 f, fi, a_c, a_e;
	u32 err = 0;
	Bool err0;
	printf("JMP check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		err0 = False;

		a_c = JO_C(f) & 1;
		a_e = JO_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JO"); }

		a_c = JNO_C(f) & 1;
		a_e = JNO_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNO"); }

		a_c = JB_C(f) & 1;
		a_e = JB_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JB"); }

		a_c = JNB_C(f) & 1;
		a_e = JNB_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNB"); }

		a_c = JE_C(f) & 1;
		a_e = JE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JE"); }

		a_c = JNE_C(f) & 1;
		a_e = JNE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNE"); }

		a_c = JBE_C(f) & 1;
		a_e = JBE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JBE"); }

		a_c = JNBE_C(f) & 1;
		a_e = JNBE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNBE"); }

		a_c = JS_C(f) & 1;
		a_e = JS_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JS"); }

		a_c = JNS_C(f) & 1;
		a_e = JNS_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNS"); }

		a_c = JP_C(f) & 1;
		a_e = JP_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JP"); }

		a_c = JNP_C(f) & 1;
		a_e = JNP_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNP"); }

		a_c = JL_C(f) & 1;
		a_e = JL_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JL"); }

		a_c = JNL_C(f) & 1;
		a_e = JNL_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNL"); }

		a_c = JLE_C(f) & 1;
		a_e = JLE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JLE"); }

		a_c = JNLE_C(f) & 1;
		a_e = JNLE_E(f) & 1;
		if (a_c != a_e) { err0 = True; printf(" JNLE"); }

#if !PRINT_ALL		// 1 = print all states, even if OK
		if (err0)
#endif
		{
			// error
			if (err == 0) printf("\n");
			err++;

			// print error
			printf(" JMP: ");
			PrintFlags(f);
			printf(" ERROR\n");
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test DAA instruction
void Test_DAA()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DAA check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = DAA_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = DAA_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("DAA");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test DAS instruction
void Test_DAS()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DAS check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = DAS_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = DAS_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("DAS");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test AAA instruction
void Test_AAA()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AAA check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = AAA_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAA_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAA");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test AAS instruction
void Test_AAS()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AAS check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = AAS_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAS_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAS");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test AAD instruction
void Test_AAD()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AAD check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{

		// === 1

			// check C code
			k = AAD1_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAD_E(f, a, 1);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAD1");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 2

			// check C code
			k = AAD2_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAD_E(f, a, 2);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAD2");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 10

			// check C code
			k = AAD10_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAD_E(f, a, 10);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAD10");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 25

			// check C code
			k = AAD25_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAD_E(f, a, 25);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAD25");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 255

			// check C code
			k = AAD255_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAD_E(f, a, 255);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAD255");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test AAM instruction
void Test_AAM()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AAM check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{

		// === 1

			// check C code
			k = AAM1_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAM_E(f, a, 1);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAM1");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 2

			// check C code
			k = AAM2_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAM_E(f, a, 2);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAM2");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 10

			// check C code
			k = AAM10_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAM_E(f, a, 10);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAM10");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 25

			// check C code
			k = AAM25_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAM_E(f, a, 25);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAM25");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}

		// === 255

			// check C code
			k = AAM255_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = AAM_E(f, a, 255);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("AAM255");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit ROL,1 instruction
void Test_ROL8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROL8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = ROL8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ROL8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("ROL8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit ROR,1 instruction
void Test_ROR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = ROR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ROR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("ROR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit RCL,1 instruction
void Test_RCL8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCL8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = RCL8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = RCL8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("RCL8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit RCR,1 instruction
void Test_RCR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = RCR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = RCR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("RCR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SHL,1 instruction
void Test_SHL8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHL8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = SHL8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SHL8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SHL8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SHR,1 instruction
void Test_SHR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = SHR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SHR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SHR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SAR,1 instruction
void Test_SAR8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SAR8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x200; a++)
		{
			// check C code
			k = SAR8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SAR8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SAR8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit ROL,CL instruction
void Test_ROL8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROL8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = ROL8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = ROL8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("ROL8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit ROR,CL instruction
void Test_ROR8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROR8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = ROR8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = ROR8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("ROR8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit RCL,CL instruction
void Test_RCL8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCL8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = RCL8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = RCL8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("RCL8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit RCR,CL instruction
void Test_RCR8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCR8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = RCR8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = RCR8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("RCR8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SHL,CL instruction
void Test_SHL8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHL8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = SHL8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SHL8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("SHL8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SHR,CL instruction
void Test_SHR8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHR8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = SHR8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SHR8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("SHR8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit SAR,CL instruction
void Test_SAR8N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SAR8N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 8086 uses all bits
		{
			for (a = 0; a < 0x200; a++)
			{
				// check C code
				k = SAR8N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SAR8_E(f_e, a_e);
					a_e = k & 0xffff;
					f_e = (k >> 16) & I8086_FLAGALL;
				}

				// Overflow flag is undefined on shifts > 1 (and it varies for different processors ... AMD leaves last OF for shift)
				if (c > 1)
				{
					f_e &= ~I8086_OF;
					f_c &= ~I8086_OF;
				}

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("SAR8N-%d", c);
					PrintErr8(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit multiplication
void Test_MUL8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("MUL8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = MUL8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = MUL8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("MUL8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit signed multiplication
void Test_IMUL8()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("IMUL8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = IMUL8_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = IMUL8_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & I8086_FLAGALL;

			// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
			if ((a_c != a_e) || (f_c != f_e))
#endif
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("IMUL8");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit unsigned division
void Test_DIV8()
{
	u32 f, fi, a, b, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DIV8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (b = 1; b < 0x100; b++)
		{
			for (a = 0; a < b*256; a++)
			{
				// check C code
				k = DIV8_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = DIV8_E(f, a, b);
				a_e = k & 0xffff;
				f_e = (k >> 16) & I8086_FLAGALL;

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("DIV8");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 8-bit signed division
void Test_IDIV8()
{
	u32 f, fi, a, b, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("IDIV8 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (b = 1; b < 0x100; b++)
		{
			for (a = 0; a < 0x10000; a++)
			{
				s32 res = (s32)(s16)(u16)a / (s16)(s8)b;
				if ((res < -128) || (res > 127)) continue;

				// check C code
				k = IDIV8_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = IDIV8_E(f, a, b);
				a_e = k & 0xffff;
				f_e = (k >> 16) & I8086_FLAGALL;

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("IDIV8");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}
