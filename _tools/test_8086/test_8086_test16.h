
// test 16-bit addition
void Test_ADD16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ADD16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = ADD16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = ADD16_E(f, a, b);
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
					printf("ADD16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = ADD16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = ADD16_E(f, b, a);
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
					printf("ADD16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit C-addition
void Test_ADC16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ADC16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = ADC16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = ADC16_E(f, a, b);
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
					printf("ADC16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = ADC16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = ADC16_E(f, b, a);
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
					printf("ADC16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit subtraction
void Test_SUB16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SUB16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = SUB16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = SUB16_E(f, a, b);
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
					printf("SUB16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = SUB16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = SUB16_E(f, b, a);
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
					printf("SUB16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit subtraction
void Test_SBB16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SBB16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = SBB16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = SBB16_E(f, a, b);
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
					printf("SBB16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = SBB16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = SBB16_E(f, b, a);
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
					printf("SBB16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit AND
void Test_AND16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("AND16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = AND16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = AND16_E(f, a, b);
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
					printf("AND16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = AND16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = AND16_E(f, b, a);
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
					printf("AND16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit OR
void Test_OR16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("OR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = OR16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = OR16_E(f, a, b);
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
					printf("OR16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = OR16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = OR16_E(f, b, a);
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
					printf("OR16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit XOR
void Test_XOR16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("XOR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = XOR16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = XOR16_E(f, a, b);
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
					printf("XOR16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = XOR16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = XOR16_E(f, b, a);
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
					printf("XOR16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit compare
void Test_CMP16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("CMP16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = CMP16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = CMP16_E(f, a, b);
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
					printf("CMP16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = CMP16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = CMP16_E(f, b, a);
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
					printf("CMP16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit TEST
void Test_TEST16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("TEST16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = TEST16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = TEST16_E(f, a, b);
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
					printf("TEST16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = TEST16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = TEST16_E(f, b, a);
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
					printf("TEST16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit INC
void Test_INC16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("INC16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = INC16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = INC16_E(f, a);
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
				printf("INC16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit DEC
void Test_DEC16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DEC16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = DEC16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = DEC16_E(f, a);
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
				printf("DEC16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit ROL,1 instruction
void Test_ROL16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROL16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = ROL16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ROL16_E(f, a);
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
				printf("ROL16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit ROR,1 instruction
void Test_ROR16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = ROR16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = ROR16_E(f, a);
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
				printf("ROR16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit RCL,1 instruction
void Test_RCL16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCL16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = RCL16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = RCL16_E(f, a);
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
				printf("RCL16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit RCR,1 instruction
void Test_RCR16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = RCR16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = RCR16_E(f, a);
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
				printf("RCR16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SHL,1 instruction
void Test_SHL16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHL16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SHL16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SHL16_E(f, a);
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
				printf("SHL16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SHR,1 instruction
void Test_SHR16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SHR16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SHR16_E(f, a);
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
				printf("SHR16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SAR,1 instruction
void Test_SAR16()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SAR16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SAR16_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & I8086_FLAGALL;

			// check E code
			k = SAR16_E(f, a);
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
				printf("SAR16");
				PrintErr16B(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit ROL,CL instruction
void Test_ROL16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROL16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = ROL16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = ROL16_E(f_e, a_e);
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
					printf("ROL16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit ROR,CL instruction
void Test_ROR16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ROR16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = ROR16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = ROR16_E(f_e, a_e);
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
					printf("ROR16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit RCL,CL instruction
void Test_RCL16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCL16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = RCL16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = RCL16_E(f_e, a_e);
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
					printf("RCL16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit RCR,CL instruction
void Test_RCR16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("RCR16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = RCR16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = RCR16_E(f_e, a_e);
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
					printf("RCR16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SHL,CL instruction
void Test_SHL16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHL16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = SHL16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SHL16_E(f_e, a_e);
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
					printf("SHL16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SHR,CL instruction
void Test_SHR16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SHR16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = SHR16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SHR16_E(f_e, a_e);
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
					printf("SHR16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit SAR,CL instruction
void Test_SAR16N()
{
	u32 f, fi, a, c, cc, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SAR16N check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (c = 0; c < 32; c++) // new CPUs use only lower 5 bits of CL, 160166 uses all bits
		{
			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = SAR16N_C(f, a, c);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				f_e = f;
				a_e = a;
				for (cc = c; cc > 0; cc--)
				{
					k = SAR16_E(f_e, a_e);
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
					printf("SAR16N-%d", c);
					PrintErr16B(a, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit multiplication
void Test_MUL16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("MUL16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = MUL16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = MUL16_E(f, a, b);
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
					printf("MUL16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = MUL16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = MUL16_E(f, b, a);
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
					printf("MUL16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit signed multiplication
void Test_IMUL16()
{
	u32 f, fi, a, b, bi, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("IMUL16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (bi = 0; bi < SAMP16_NUM; bi++)
		{
			b = Samp16[bi];

			for (a = 0; a < 0x10000; a++)
			{
				// check C code
				k = IMUL16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = IMUL16_E(f, a, b);
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
					printf("IMUL16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}

				// check C code
				k = IMUL16_C(f, b, a);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = IMUL16_E(f, b, a);
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
					printf("IMUL16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit division
void Test_DIV16()
{
	u32 f, fi, a, b, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("DIV16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (b = 1; b < 0x10000; b++)
		{
			for (a = 0; (a < (b<<16)) && (a < 0x100); a++)
			{
				// check C code
				k = DIV16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;

				// check E code
				k = DIV16_E(f, a, b);
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
					printf("DIV16");
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test 16-bit signed division
void Test_IDIV16()
{
	u32 f, fi, b, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	s32 a;
	printf("IDIV16 check: ");

	// loop variants
	for (fi = 0; fi < FLAG_NUM; fi++)
	{
		f = FlagInx[fi];
		if ((fi & 3) == 2) printf(".");

		for (b = 1; b < 0x10000; b += 13)
		{
			for (a = -0x100000; a < 0x100000; a += 123)
			{
				s32 res = (s32)a / (s16)(u16)b;
				if ((res < -32768) || (res > 32767)) continue;

				// check C code
				k = IDIV16_C(f, a, b);
				a_c = k & 0xffff;
				f_c = (k >> 16) & I8086_FLAGALL;
				f_c &= ~I8086_CF; // CF undefined

				// check E code
				k = IDIV16_E(f, a, b);
				a_e = k & 0xffff;
				f_e = (k >> 16) & I8086_FLAGALL;
				f_e &= ~I8086_CF; // CF undefined

#if I8086_CPU_INTEL && I8086_CPU_80286_UP // flags undefined
				f_c = 0;
				f_e = 0;
#endif

				// compare result
#if !PRINT_ALL		// 1 = print all states, even if OK
				if ((a_c != a_e) || (f_c != f_e))
#endif
				{
					// error
					if (err == 0) printf("\n");
					err++;

					// print error
					printf("IDIV16 res=%d", res);
					PrintErr16(a, b, f, a_c, f_c, a_e, f_e);
				}
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}
