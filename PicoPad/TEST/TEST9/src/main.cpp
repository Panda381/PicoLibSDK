
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

real16 Real16a;
real32 Real32a;
real48 Real48a;
real64 Real64a;
real80 Real80a;
real96 Real96a;
real128 Real128a;
real160 Real160a;
real192 Real192a;
real256 Real256a;
real384 Real384a;
real512 Real512a;
real768 Real768a;
real1024 Real1024a;
real1536 Real1536a;
real2048 Real2048a;
real3072 Real3072a;
real4096 Real4096a;
real6144 Real6144a;
real8192 Real8192a;
real12288 Real12288a;

int main()
{
	int n, n2;	

//	printf("Connect USB console and press a key...");
//	while (GetChar() == NOCHAR) {}

	printf("\nNormal precision:\n");

#define REPNAME "report.h"
#include "test_real.h"

	printf("\nExtended precision:\n");

#define REPNAME "reportx.h"
#include "test_real.h"

	printf("OK ");

	// reset to boot loader
	while (KeyGet() != KEY_Y) {}
#if USE_SCREENSHOT		// use screen shots
	ScreenShot();
#endif
	ResetToBootLoader();
}
