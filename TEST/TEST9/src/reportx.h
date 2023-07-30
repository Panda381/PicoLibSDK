// Test9ext

	printf("real%dext: ", REALNAME(RealBits)());
	n = REALNAME(Test9Ext)(&REALNAME(a));
	n2 = REALNAME(MantBits)()+1-n;
	if (n2<0) n2=0;
	printf("%.2f digits, lost %.2f digits, lost on normal %.2f digits (total %.2f)\n",
		n*0.30103, (REALNAME(RealBits)()-1-n)*0.30103, n2*0.30103, REALNAME(MantDigDecDExt)());

#undef REALNAME
