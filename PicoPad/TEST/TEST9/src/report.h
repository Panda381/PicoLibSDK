// Test9

	WaitMs(2000);

	printf("real%d:\n", REALNAME(RealBits)());
	n = REALNAME(Test9)(&REALNAME(a));
	printf(" %.2fdig, lost %.2fdig (total %.2f)\n", n*0.30103,
		REALNAME(MantDigDecD)()-n*0.30103, REALNAME(MantDigDecD)());

#undef REALNAME
