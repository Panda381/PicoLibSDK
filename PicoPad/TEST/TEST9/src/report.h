// Test9

	printf("real%d: ", REALNAME(RealBits)());
	n = REALNAME(Test9)(&REALNAME(a));
	printf("%.2f digits, lost %.2f digits (total %.2f)\n", n*0.30103,
		REALNAME(MantDigDecD)()-n*0.30103, REALNAME(MantDigDecD)());

#undef REALNAME
