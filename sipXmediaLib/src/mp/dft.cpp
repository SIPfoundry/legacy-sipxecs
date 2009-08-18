//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef __pingtel_on_posix__ /* [ */

#define VX1_PC0 1		// Set to 1 for VXWORKS, 0 for PC simulation
#define USE_IMAGSQ 1

#include "mp/FilterBank.h"
#if (VX1_PC0 == 1) //////////////////////////////////////////////////////////
#include "mp/HandsetFilterBank.h"
#else		// #else for VX1_PC0

	#include <stdlib.h>
	#include <stdio.h>
	#include <math.h>

	int imagsq(icomplex *, int);

#endif		// #endif for VX1_PC0


   int IndexMapInc5Mod40[40];
	int IndexMapInc8Mod40[40];
	int IndexMapInc13Mod40[40];


//void ComputeFFT80RtoCCosSinTable();
int FFT80RtoCCosSinTable[38] = {
32667,  2571, 32365,  5126, 31863,  7650, 31164, 10126, 30274, 12540,
29197, 14876, 27939, 17121, 26510, 19261, 24917, 21281, 23170, 23170,
21281, 24917, 19261, 26510, 17121, 27939, 14876, 29197, 12540, 30274,
10126, 31164,  7650, 31863,  5126, 32365,  2571, 32667};

//void ComputeFFT16RtoCCosSinTable();
int FFT16RtoCCosSinTable[6] = {
30274, 12540, 23170, 23170, 12540, 30274};

/* Used once in simulation to fprintf constants used by FFT5CtoC() to dft5table */
//void compute_dft5_constants();
const int ic50 = 15582;
const int ic51 = -25212;
const int ic52 = -5952;
const int ic53 = 9159;
const int ic54 = -20480;

const int Dft8c = 11585;		// 0.707106781 * SCALE_DFT8

/* Used once in simulation to fprintf constants used by FFT16CtoC() to dft16table */
//void compute_dft16_constants();

const int ic80 = 5793;
const int ic161 = 3135;
const int ic162 = 10703;
const int ic163 = 4433;
const int ic164 = 7568;


int IndexMapInc5Mod80[80];
int IndexMapInc16Mod80[80];
int IndexMapInc21Mod80[80];

//void compute_dBTable();  // Used once in simulation to fprintf dBTable[] to file dBTable
const short int dBTable[256] = {   // used by int Get1000log10(int)
  0,   2,   3,   5,   7,   8,  10,  12,  13,  15,  17,  18,  20,  22,  23,  25,
 26,  28,  30,  31,  33,  34,  36,  37,  39,  40,  42,  44,  45,  47,  48,  50,
 51,  53,  54,  56,  57,  59,  60,  62,  63,  65,  66,  67,  69,  70,  72,  73,
 75,  76,  77,  79,  80,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  96,
 97,  98, 100, 101, 102, 104, 105, 106, 108, 109, 110, 112, 113, 114, 116, 117,
118, 119, 121, 122, 123, 125, 126, 127, 128, 130, 131, 132, 133, 135, 136, 137,
138, 140, 141, 142, 143, 144, 146, 147, 148, 149, 150, 152, 153, 154, 155, 156,
158, 159, 160, 161, 162, 163, 165, 166, 167, 168, 169, 170, 172, 173, 174, 175,
176, 177, 178, 179, 181, 182, 183, 184, 185, 186, 187, 188, 189, 191, 192, 193,
194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 205, 206, 207, 208, 209, 210,
211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 255, 256, 257,
258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 268, 269, 270, 271, 272,
273, 274, 275, 276, 277, 278, 278, 279, 280, 281, 282, 283, 284, 285, 285, 286,
287, 288, 289, 290, 291, 292, 292, 293, 294, 295, 296, 297, 298, 298, 299, 300
};

int rngseed;

void TestDft()
{
//	icomplex v[82];
//	icomplex w[82];

//	ComputeFFT80RtoCCosSinTable();
//	ComputeFFT16RtoCCosSinTable();

	rngseed = 1;		// Seed random number generator.

#if (0)
    printf("testing FFT80CtoC() as its own inverse (with conjugation)\n");

//	compute_dft5_constants();
//	compute_dft16_constants();

	getdft80tables();		// Initialize 3 tables used in fast 80-pt FFT.
//	compute_dBTable();

	total_err_db = 0;
#define NUM_TESTS 64
	for (k=0; k<NUM_TESTS; k++)
	{
		for (j=0; j<80; j++) {
			w[j].r = myrand();
			v[j].r = w[j].r;
		}
		for (j=0; j<80; j++) {
			w[j].i = myrand();
			v[j].i = w[j].i;
		}
// DEBUG!!! Also test with sine waves.

/*		for (j=0; j<80; j++) {
			printf("%6d %6d   ",w[j].r,w[j].i);
			if ((j%4) == 3) printf("\n");
		}
		getchar(); */

// FFT80CtoC() scales up by square root of 80.
// Here, we scale down the first output by 8 and the second output by 10.
		FFT80CtoC(v);
		for (i=0; i < 80; i++) {
			v[i].r  = (v[i].r + 4) >> 3;
			v[i].i  = (-v[i].i + 4) >> 3;
		}
		FFT80CtoC(v);
		for (i=0; i < 80; i++) {
			v[i].r  = (v[i].r * 3277 + 16384) >> 15;
			v[i].i  = (-v[i].i * 3277 + 16384) >> 15;
		}

/*		for (i=0; i < 80; i++) {
			printf("%6d %6d     %6d %6d\n",w[i].r, w[i].i,
			v[i].r, v[i].i);
			if (i%16 == 15) getchar();
		}
		getchar(); */

		total_err_db += FFTError((int *) v,(int *) w, 2*80);
//		if (k%4 == 3) printf("\n");
		printf("\n");
	}
	printf("\n");

	total_err_db /= NUM_TESTS;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);
//	printf("average error should be 66.17 db down.\n");

	getchar();
#endif

#if (0)
	printf("testing FFT40CtoC() as its own inverse (with conjugation)\n");

	GetDft40Tables();		// Initialize 3 tables used in fast 40-pt FFT.

	total_err_db = 0;
#define NUM_TESTS 64
	for (k=0; k<NUM_TESTS; k++)
	{
		for (j=0; j<40; j++) {
			w[j].r = myrand();
			v[j].r = w[j].r;
		}
		for (j=0; j<40; j++) {
			w[j].i = myrand();
			v[j].i = w[j].i;
		}
// DEBUG!!! Also test with sine waves.

/*		for (j=0; j<40; j++) {
			printf("%6d %6d   ",w[j].r,w[j].i);
			if ((j%4) == 3) printf("\n");
		}
		getchar(); */

// FFT40CtoC() scales up by square root of 40.
// Here, we scale down the first output by 4 and the second output by 10.
		FFT40CtoC(v);
		for (i=0; i < 40; i++) {
			v[i].r  = (v[i].r + 2) >> 2;
			v[i].i  = (-v[i].i + 2) >> 2;
		}
		FFT40CtoC(v);
		for (i=0; i < 40; i++) {
			v[i].r  = (v[i].r * 3277 + 16384) >> 15;
			v[i].i  = (-v[i].i * 3277 + 16384) >> 15;
		}

/*		for (i=0; i < 40; i++) {
			printf("%6d %6d     %6d %6d\n",w[i].r, w[i].i,
			v[i].r, v[i].i);
			if (i%16 == 15) getchar();
		}
		getchar(); */

		total_err_db += FFTError((int *) v,(int *) w, 2*40);
		if (k%8 == 7) printf("\n");
//		printf("\n");
	}
	printf("\n");

	total_err_db /= NUM_TESTS;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);
	printf("average error should be 68.54 db down.\n");

	getchar();
#endif



#if (0)
	printf("testing FFT80RtoC() vs FFT80CtoC()\n");
	int x[82];

    getdft80tables();		// Initialize 3 tables used in fast 80-pt FFT
    total_err_db = 0;
	for (k=0; k<64; k++)
	{

		for (j=0; j < 80; j++)
		{
			x[j]= myrand();
			v[j].r = x[j];
			v[j].i = 0;
		}

		FFT80RtoC((icomplex *) x);
		for (i=0; i <= 40; i++) {
            w[i].r = x[2*i];
            w[i].i = x[2*i+1];
        }

		FFT80CtoC(v);

/*
		for (i=0; i <= 40; i++) {
			printf("%6d %6d     %6d %6d     %6d %6d\n",v[i].r, v[i].i,
			w[i].r, w[i].i, (v[i].r - w[i].r), (v[i].i - w[i].i));
			if (i%16 == 15) getchar();
		}
		getchar();
        */

		total_err_db += FFTError((int *) w,(int *) v, 2*41);
//		if (k%4 == 3) printf("\n");
		printf("\n");
	}
	printf("\n");

	total_err_db /= 64;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);

	getchar();

#endif


#if (0)
	printf("testing FFT80CtoR() vs FFT80CtoC()\n");

	icomplex u[41];
	int x[80];

	total_err_db = 0;
	for (k=0; k<64; k++)
	{

		for (j=0; j <= 40; j++)
		{
			u[j].r = myrand();
			u[j].i = myrand();
		}
		u[0].i = 0;
		u[40].i = 0;
		for (j=0; j <= 40; j++)
		{
			v[j].r = u[j].r;
			v[j].i = -u[j].i;
		}
		for (j=1; j < 40; j++)
		{
			v[80-j].r = u[j].r;
			v[80-j].i = u[j].i;
		}

		FFT80CtoR(x,u);
		FFT80CtoC(v);

		for (i=0; i < 80; i++) {
			printf("%6d %6d     %6d    %6d\n",v[i].r, v[i].i,
			x[i], (v[i].r - x[i]));
			if (i%16 == 15) getchar();
		}
		getchar();

		double pow,err,dif;
		pow = 0.0;
		err = 0.0;
		for (i=0; i<80; i++) {
			pow += (v[i].r * v[i].r);
			dif = (x[i] - v[i].r);
			err += dif*dif;
		}
		printf("%8.2f  ",10.0*log10(pow/err));
//		getchar();
	}

#endif



#if (0)
	printf("testing FFT80RtoC() and FFT80CtoR() as inverses\n");

	int x[80];
	int y[82];

	total_err_db = 0;
#define NUM_TESTS 64
	for (k=0; k<NUM_TESTS; k++)
	{

		for (j=0; j < 80; j++)
		{
			x[j]= myrand();
			y[j] = x[j];
		}

		FFT80RtoC((icomplex *) y);

// FFT80RtoC() and FFT80CtoR() each scale up by square root of 80.
// Here, we scale down the first output by 8 and the second output by 10.
// How we do the scaling here can have a huge effect on measured error.
		for (i=0; i < 82; i++) {
			y[i]  = (y[i] + 4) >> 3;
		}
		FFT80CtoR((icomplex *) y);
		for (i=0; i < 80; i++) {
			y[i]  = (y[i] * 3277 + 16384) >> 15;
		}
// xxx

/*		for (i=0; i < 80; i++) {
			printf("%6d %6d   ",x[i], y[i]);
			if (i%4 == 3) printf("\n");
		}
		getchar(); */

		total_err_db += FFTError(x, y, 80);
//		getchar();
		if (k%8 == 7) printf("\n");
//		printf("\n");
	}
//	printf("\n");

	total_err_db /= NUM_TESTS;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);
	printf("average error should be 64.08 db down.\n");

	getchar();
#endif




#if (0)
	printf("testing FFT16RtoC() vs FFT16CtoC()\n");
	int x[18];

	total_err_db = 0;
	for (k=0; k<64; k++)
	{

		for (j=0; j < 16; j++)
		{
			x[j]= myrand();
			v[j].r = x[j];
			v[j].i = 0;
		}
		FFT16RtoC((icomplex *) x);
		for (i=0; i <= 8; i++) {
            w[i].r = x[2*i];
            w[i].i = x[2*i+1];
        }

		FFT16CtoC(v);

		for (i=0; i <= 8; i++) {
			printf("%6d %6d     %6d %6d     %6d %6d\n",v[i].r, v[i].i,
			w[i].r, w[i].i, (v[i].r - w[i].r), (v[i].i - w[i].i));
		}
		getchar();

		total_err_db += FFTError((int *) w,(int *) v, 2*9);
//		if (k%4 == 3) printf("\n");
		printf("\n");
	}
	printf("\n");

	total_err_db /= 64;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);

	getchar();

#endif

#if (VX1_PC0 == 0)
#if (0)
	printf("testing FFT16RtoC() and FFT16CtoR() as inverses\n");

	int x[16];
	int y[18];

	total_err_db = 0;
#define NUM_TESTS 64
	for (k=0; k<NUM_TESTS; k++)
	{

		for (j=0; j < 16; j++)
		{
			x[j]= myrand();
			y[j] = x[j];
		}

		FFT16RtoC((icomplex *) y);

// FFT80RtoC() and FFT80CtoR() each scale up by square root of 16.
// Here, we scale down the first output by 2 and the second output by 2.
// How we do the scaling here can have a huge effect on measured error.
		for (i=0; i < 18; i++) {
			y[i]  = (y[i] + 2) >> 2;
		}
		FFT16CtoR((icomplex *) y);
		for (i=0; i < 16; i++) {
			y[i]  = (y[i] + 2) >> 2;
		}
// xxx

/*		for (i=0; i < 16; i++) {
			printf("%6d %6d   ",x[i], y[i]);
			if (i%4 == 3) printf("\n");
		}
		getchar(); */

		total_err_db += FFTError(x, y, 16);
//		getchar();
		if (k%8 == 7) printf("\n");
//		printf("\n");
	}
//	printf("\n");

	total_err_db /= NUM_TESTS;
	printf("average error is %2d.%2d dB down.\n",total_err_db/100,
		total_err_db - (total_err_db/100)*100);
//	printf("average error should be 64.08 db down.\n");

	getchar();
#endif
#endif





}


#if (0)
int FFTError(int v[], int w[], int length)
{
	int i;
	double pow,err,dif;
	pow = 0.0;
	err = 0.0;
	for (i=0; i<length; i++) {
		pow += (w[i] * w[i]);
		dif = (v[i] - w[i]);
		err += dif*dif;
	}

	if (err == 0.0) err = 0.5;		// Prevent division by 0.
//	printf("total error is %8.2f db down from total power\n",10.0*log10(pow/err));
	printf("pow=%5.2f dB  err=%5.2f dB   ",10.0*log10(pow),10.0*log10(err));
	printf("%5.2f dB  ",10.0*log10(pow/err));
	return((int) (0.5 + 1000.*log10(pow/err)) );
}

#else

/* Compute the sum of the squares of the differences of any 2 vectors
and the sum of the squares of the second vector in dB.
Return an integer which equals 100 times the dB measure of how far down the
error is relative to the power in the second vector. */
// DEBUG!!! Should separately normalize the diffs.

int FFTError(int v[], int w[], int length)
{
	int i,j;
	int pow,err,dif;
	int temp0;
	int preshift;
	int round;

// First normalize to avoid overflow.
	pow = 0;
	for (i=0; i<length; i++) {
		temp0 = w[i] >> 4;
		pow += temp0*temp0;
	}
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
	preshift = 22 - i;

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
	}

	pow = 0;
	err = 0;
	for (i=0; i<length; i++) {
		temp0 = (w[i]* w[i]);
		if (preshift < 0) {
			temp0 = (temp0 + round) >> j;
		}
		else if (preshift > 0) {
			temp0 <<= preshift;
		}
		pow += temp0;
		dif = (v[i] - w[i]);
		err += (dif*dif);
	}

	int powdbtimes100, errdbtimes100;

	powdbtimes100 = Get1000log10(pow) - 301*preshift;
	errdbtimes100 = Get1000log10(err);
	i = (powdbtimes100 - errdbtimes100)/100;
	j = (powdbtimes100 - errdbtimes100) - i*100;

//	printf("pow=%8xd preshift=%2d %5d   err=%8x %5d ",pow, preshift, powdbtimes100, err, errdbtimes100);
	printf("%2d.%2d  ",i,j);

	return(powdbtimes100 - errdbtimes100);

}
#endif

/*
void compute_dBTable()
{
	FILE *fp;
	int dBTable[256];
	int j;

	if ((fp = fopen("dBTable","w")) == NULL) {
		printf("cannot open file dBTable.\n");
		exit(1);
	}

	for (j=0; j<256; j++)
		dBTable[j] = (int) (0.5 + 100.0*10.0*log10((double)(j+256)/(double)256.));

	fprintf(fp,"const short int dBTable[256] = {   // used by int Get1000log10(int)\n");
	for (j=0; j<256; j++) {
		fprintf(fp,"%3d",dBTable[j]);
		if (j<255) fprintf(fp,", ");
		if (j%16 == 15) fprintf(fp,"\n");
	}
	fprintf(fp,"};\n");
	fclose(fp);

}
*/

int Get1000log10(int input)
{
	int i;
	int temp0;
#if (0)
	temp0 = input;
	i = 0;
	while (temp0 > 0) {
		temp0 >>= 1;
		i++;
	}
#else
	temp0 = input;
	i = 0;
	if (temp0 > 0xffff) {
		temp0 >>= 16;
		i += 16;
	}
	if (temp0 > 0xff) {
		temp0 >>= 8;
		i += 8;
	}
	if (temp0 > 0xf) {
		temp0 >>= 4;
		i += 4;
	}
	if (temp0 > 0x3) {
		temp0 >>= 2;
		i += 2;
	}
	if (temp0 > 0x1) {
		temp0 >>= 1;
		i += 1;
	}
	if (temp0 > 0) {
		i += 1;
	}
#endif
	if (i >= 9) {
		temp0 = (input >> (i-9)) & 0xff;
		i = 301*i + dBTable[temp0];
		return(i);
	}
	else {
		temp0 = (input << (9-i)) & 0xff;
		i = 301*i + dBTable[temp0];
		return(i);
	}

}

/* ***************************************************************************** */
/*
void compute_dft5_constants()
{
#define SCALE_DFT5 16384
#define SHIFT_DFT5 14
#define ROUND_DFT5 8192

	FILE *fp;

	if ((fp = fopen("dft5table","w")) == NULL) {
		printf("cannot open file dBTable.\n");
		exit(1);
	}

	fprintf(fp,"#define ic50 %6d\n",(int) (0.95105652*SCALE_DFT5 + 0.5));
	fprintf(fp,"#define ic51 %6d\n",(int) (-1.53884180*SCALE_DFT5 - 0.5));
	fprintf(fp,"#define ic52 %6d\n",(int) (-0.36327126*SCALE_DFT5 - 0.5));
	fprintf(fp,"#define ic53 %6d\n",(int) (0.55901699*SCALE_DFT5 + 0.5));
	fprintf(fp,"#define ic54 %6d\n",(int) (-1.25*SCALE_DFT5 - 0.5));

	fclose(fp);
}
*/

void FFT5CtoC(icomplex v[])
{
/*	const short SCALE_DFT5 = 16384;
	const short SHIFT_DFT5 = 14;
	const short ROUND_DFT5 = 8192; */
#define SCALE_DFT5 16384
#define SHIFT_DFT5 14
#define ROUND_DFT5 8192

/*
#define ic50  15582
#define ic51 -25212
#define ic52  -5952
#define ic53   9159
#define ic54 -20480
*/

	int regr0,regr1,regr2,regr3;
	int regt;
	int regs0,regs1,regs2,regs3;

	regr1 = v[1].r;
	regr2 = v[4].r;
	regr0 = regr1 + regr2;
	regr3 = regr1 - regr2;
	regt = v[2].r;
	regs0 = v[3].r;
	regr2 = regt + regs0;
	regr1 = regt - regs0;
	regt = ((regr0 - regr2) * ic53 + ROUND_DFT5) >> SHIFT_DFT5;
	regr0 += regr2;
	regs0 = v[0].r + regr0;
	v[0].r = regs0;
	regr0 = regs0 + ((regr0*ic54 + ROUND_DFT5) >> SHIFT_DFT5);
	regr2 = regr0 - regt;
	regr0 += regt;
	regt = ((regr3 + regr1) * ic50 + ROUND_DFT5) >> SHIFT_DFT5;
	regr3 = regt + ((regr3*ic51 + ROUND_DFT5) >> SHIFT_DFT5);
	regr1 = regt + ((regr1*ic52 + ROUND_DFT5) >> SHIFT_DFT5);

	regs0 = v[1].i + v[4].i;
	regs3 = v[1].i - v[4].i;
	regs2 = v[2].i + v[3].i;
	regs1 = v[2].i - v[3].i;
	regt = ((regs0 - regs2) * ic53 + ROUND_DFT5) >> SHIFT_DFT5;
	regs0 += regs2;
	v[0].i += regs0;
	regs0 = v[0].i + ((regs0*ic54 + ROUND_DFT5) >> SHIFT_DFT5);
	regs2 = regs0 - regt;
	regs0 += regt;
	regt = ((regs3 + regs1) * ic50 + ROUND_DFT5) >> SHIFT_DFT5;
	regs3 = regt + ((regs3*ic51 + ROUND_DFT5) >> SHIFT_DFT5);
	regs1 = regt + ((regs1*ic52 + ROUND_DFT5) >> SHIFT_DFT5);

	v[1].r = regr0 + regs1;
	v[4].r = regr0 - regs1;
	v[2].r = regr2 - regs3;
	v[3].r = regr2 + regs3;

	v[1].i = regs0 - regr1;
	v[4].i = regs0 + regr1;
	v[2].i = regs2 + regr3;
	v[3].i = regs2 - regr3;

}


/* ***************************************************************************** */


void FFT8CtoC(icomplex v[])

{
#define SCALE_DFT8 16384
#define SHIFT_DFT8 14
#define ROUND_DFT8 8192

//	static double c80 = 0.70710678;

	int r0,r1,r2,r3,r4,r5,r6,r7;
	int t0,t1,t2,t3;
	int s0,s1,s2;

	r0 = v[0].r + v[4].r;
	r1 = v[0].r - v[4].r;
	r2 = v[1].r + v[7].r;
	r3 = v[1].r - v[7].r;
	r4 = v[2].r + v[6].r;
	r5 = v[2].r - v[6].r;
	r6 = v[3].r + v[5].r;
	r7 = v[3].r - v[5].r;

	t0 = r0 + r4;
	t1 = r0 - r4;
	t2 = r2 + r6;
	r2 = ((r2 - r6) * Dft8c + ROUND_DFT8) >> SHIFT_DFT8;
	v[0].r = t0 + t2;
	v[4].r = t0 - t2;

	t0 = r1 + r2;
	t2 = r1 - r2;
	s0 = r3 - r7;
	r3 = ((r3 + r7) * Dft8c + ROUND_DFT8) >> SHIFT_DFT8;
	s1 = r3 + r5;
	s2 = r3 - r5;

	r0 = v[0].i + v[4].i;
	r1 = v[0].i - v[4].i;
	r2 = v[1].i + v[7].i;
	r3 = v[1].i - v[7].i;
	r4 = v[2].i + v[6].i;
	r5 = v[2].i - v[6].i;
	r6 = v[3].i + v[5].i;
	r7 = v[3].i - v[5].i;

	t3 = r0 + r4;
	r0 = r0 - r4;
	r4 = r2 + r6;
	r2 = ((r2 - r6) * Dft8c + ROUND_DFT8) >> SHIFT_DFT8;
	v[0].i = t3 + r4;
	v[4].i = t3 - r4;

	r4 = r1 + r2;
	r1 = r1 - r2;
	r2 = r3 - r7;
	r3 = ((r3 + r7) * Dft8c + ROUND_DFT8) >> SHIFT_DFT8;
	r6 = r3 + r5;
	r3 = r3 - r5;

	v[1].r = t0 + r6;
	v[7].r = t0 - r6;
	v[2].r = t1 + r2;
	v[6].r = t1 - r2;
	v[3].r = t2 + r3;
	v[5].r = t2 - r3;

	v[1].i = r4 - s1;
	v[7].i = r4 + s1;
	v[2].i = r0 - s0;
	v[6].i = r0 + s0;
	v[3].i = r1 - s2;
	v[5].i = r1 + s2;

}

/* ***************************************************************************** */

void GetDft40Tables()
{
	int *indexptr;
	int k;
	int j,i,l;

	indexptr = IndexMapInc5Mod40;
	*indexptr = 0;
	for (j=0; j<5; j++) {
		for (l=1; l<8; l++) {
			k = *indexptr++ + 5;
			*indexptr = k;
			if (*indexptr >= 40) *indexptr -= 40;
		}
		if (j < 4) {
			*indexptr++;
			*indexptr = *(indexptr-8) + 8;
		}
	}

	indexptr = IndexMapInc8Mod40;
	*indexptr = 0;
	for (j=0; j<8; j++) {
		for (l=1; l<5; l++) {
			k = *indexptr++ + 8;
			*indexptr = k;
			if (*indexptr >= 40) *indexptr -= 40;
		}
		if (j < 15) {
			*indexptr++;
			*indexptr = *(indexptr-5) + 5;
		}
	}

/*	int *CosSinPtr;
	CosSinPtr = FFT80RtoCCosSinTable;
	for (i=1; i < 20; i++) {
		*CosSinPtr++ = cos(2.0*PI*i/80.0);
		*CosSinPtr++ = sin(2.0*PI*i/80.0);
	} */

/* For unscrambling the fft output. */
	for (i=0; i<40; i++)
		IndexMapInc13Mod40[i] = (i*13)%40;

}


/* ***************************************************************************** */

void FFT40CtoC(icomplex x[])
{
	icomplex v[8];
	int *indexptr;
	int index;
	int i,j;
	icomplex Fftout[40];

// Optimize:Completely unroll all loops and function calls here!!!
// Or better, just pass index pointer to function ??
// Do 5 8-point complex dfts.
	indexptr = IndexMapInc5Mod40;
	for (j=0; j<5; j++) {
		for (i=0; i<8; i++) {
			index = *indexptr++;
			v[i].r = x[index].r;
			v[i].i = x[index].i;
		}
		FFT8CtoC(v);
		indexptr -= 8;
		for (i=0; i<8; i++) {
			index = *indexptr++;
			x[index].r = v[i].r;
			x[index].i = v[i].i;
		}
	}

// Do 8 5-point complex dfts.
	indexptr = IndexMapInc8Mod40;
	for (j=0; j<8; j++) {
		for (i=0; i<5; i++) {
			index = *indexptr++;
			v[i].r = x[index].r;
			v[i].i = x[index].i;
		}
		FFT5CtoC(v);
		indexptr -= 5;
		for (i=0; i<5; i++) {
			index = *indexptr++;
			x[index].r = v[i].r;
			x[index].i = v[i].i;
		}
	}

/* Unscramble the output. */
	for (i=0; i<40; i++) {
		j = IndexMapInc13Mod40[i];
		Fftout[i].r = x[j].r;
		Fftout[i].i = x[j].i;
	}
	for (i=0; i<40; i++) {
		x[i].r = Fftout[i].r;
		x[i].i = Fftout[i].i;
	}


}
/* ***************************************************************************** */
#define NORM_FFT80RtoC_CONSTANT 19		// It's possible we could go to 20 without saturation
#define NORM_FFT80CtoR_CONSTANT 19

/* This function computes the first 41 complex values of the (symmetric) DFT of a 80-point
real array. */

void FFT80RtoC(icomplex w[])
{
	int i;

/* Conceptually, place the even-indexed input values into the real part of w[] and the
odd-indexed values into the imaginary part of w[]. The casting of the (int *) time-domain
function argument to (icomplex *) makes this happen automatically. */

#if (1)
// pre-normalization
	int j;
	int pow;
//	int temp0,temp1;
	int preshift;
	int round;

	pow = 0;
	for (i=0; i<40; i ++) {
#if USE_IMAGSQ
		pow += imagsq(&(w[i]), 4);
#else
		temp0 = w[i].r >> 4;
		temp1 = w[i].i >> 4;
		pow += temp0*temp0 + temp1*temp1;
#endif
	}
//	printf("pow = %d ",pow);
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
//	printf("%2d ",i);
	preshift = (NORM_FFT80RtoC_CONSTANT-i) >> 1;

//	printf("%1d ",preshift);

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
		for (i=0; i<40; i++) {
			w[i].r = (w[i].r + round) >> j;
			w[i].i = (w[i].i + round) >> j;
		}
	}
	else if (preshift > 0) {
		for (i=0; i<40; i++) {
			w[i].r = w[i].r << preshift;
			w[i].i = w[i].i << preshift;
		}
	}
#endif


/* Compute a 40-point complex DFT in place. */
	FFT40CtoC(w);

	int h1r,h1i,h2r,h2i;

/* Put the 40-point real-input 21-point complex output DFT of the even samples
(real part of y[]-input) into u[], and the DFT of the odd samples (imaginary part
of y[]-input) into v[].
Construct the output w[i] = u[i] + (v[i] * e**(-j*2*PI*i/80)).
(The modulation of v[] compensates for the 1-sample time-shift of the odd samples.)
Since we do this all in-place, there are no actual u[] and v[] arrays. */

	h1r = w[0].r;
	h1i = w[0].i;
	w[0].r = h1r + h1i;
	w[40].r = h1r - h1i;
	w[0].i = 0;
	w[40].i = 0;
//	w[20].r = w[20].r;
	w[20].i = -w[20].i;

	int *CosSinPtr;
	int rcos,rsin,icos,isin;
	CosSinPtr = FFT80RtoCCosSinTable;

	for (i=1; i < 20; i++) {
		h1r = (w[i].r + w[40-i].r) >> 1;
		h1i = (w[i].i - w[40-i].i) >> 1;
		h2r = (w[i].i + w[40-i].i) >> 1;
		h2i = (w[40-i].r - w[i].r) >> 1;

		icos = *CosSinPtr++;		// cos(2.0*PI*i/80.0)
		isin = *CosSinPtr++;		// sin(2.0*PI*i/80.0)

		rcos = (h2r * icos + 16384) >> 15;
		rsin = (h2r * isin + 16384) >> 15;
		icos = (icos * h2i + 16384) >> 15;
		isin = (isin * h2i + 16384) >> 15;

		rcos += isin;
		icos -= rsin;

		w[i].r = h1r + rcos;
		w[i].i = h1i + icos;
		w[40-i].r = h1r - rcos;
		w[40-i].i = icos - h1i;
	}


#if (1)
	if (preshift < 0) {		// shift output up
		j = -preshift;
		for (i=0; i <= 40; i++) {
			w[i].r = w[i].r << j;
			w[i].i = w[i].i << j;
		}
	}
	else if (preshift > 0) {
		round = 1 << (preshift-1);
		for (i=0; i <= 40; i++) {
			w[i].r = (w[i].r + round) >> preshift;
			w[i].i = (w[i].i + round) >> preshift;
		}
	}
#endif

}
/* ***************************************************************************** */
/* This function computes the 80 real time-domain samples from the first
41 complex DFT values of an 80-point spectrum whose real part is even and
imaginary part is odd. */

void FFT80CtoR(icomplex w[])
{
	int i;
	int h1r,h1i,h2r,h2i;


#if (1)
// pre-normalization
	int j;
	int pow;
//	int temp0,temp1;
	int preshift;
	int round;

	pow = 0;
	for (i=0; i <= 40; i ++) {
#if USE_IMAGSQ
		pow += imagsq(&(w[i]), 4);
#else
		temp0 = w[i].r >> 4;
		temp1 = w[i].i >> 4;;
		pow += temp0*temp0 + temp1*temp1;
#endif
	}
//	printf("pow = %d ",pow);
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
//	printf("%2d ",i);
	preshift = (NORM_FFT80CtoR_CONSTANT-i) >> 1;

//	printf("%1d ",preshift);

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
		for (i=0; i <= 40; i++) {
			w[i].r = (w[i].r + round) >> j;
			w[i].i = (w[i].i + round) >> j;
		}
	}
	else if (preshift > 0) {
		for (i=0; i <= 40; i++) {
			w[i].r = w[i].r << preshift;
			w[i].i = w[i].i << preshift;
		}
	}
#endif



	h1r = w[0].r;
	h1i = w[40].r;
	w[0].r = h1r + h1i;
	w[0].i = h1r - h1i;

	w[20].r = w[20].r << 1;
	w[20].i = (-w[20].i) << 1;

	int *CosSinPtr;
	int rcos,rsin,icos,isin;
	CosSinPtr = FFT80RtoCCosSinTable;

	for (i=1; i < 20; i++) {

		h1r = (w[i].r + w[40-i].r);
		h1i = (w[40-i].i - w[i].i);
		h2r = (w[i].i + w[40-i].i);
		h2i = (w[i].r - w[40-i].r);

		icos = *CosSinPtr++;		// cos(2.0*PI*i/80.0)
		isin = *CosSinPtr++;		// sin(2.0*PI*i/80.0)

		rcos = (h2r * icos + 16384) >> 15;
		rsin = (h2r * isin + 16384) >> 15;
		icos = (icos * h2i + 16384) >> 15;
		isin = (isin * h2i + 16384) >> 15;

		rcos += isin;
		icos -= rsin;

		w[i].r = h1r + rcos;
		w[i].i = h1i + icos;
		w[40-i].r = h1r - rcos;
		w[40-i].i = icos - h1i;
	}

/* Compute a 40-point complex DFT in place. */
	FFT40CtoC(w);


#if (1)
	if (preshift < 0) {		// shift output up
		j = -preshift;
		for (i=0; i < 40; i++) {
			w[i].r = w[i].r << j;
			w[i].i = w[i].i << j;
		}
	}
	else if (preshift > 0) {
		round = 1 << (preshift-1);
		for (i=0; i < 40; i++) {
			w[i].r = (w[i].r + round) >> preshift;
			w[i].i = (w[i].i + round) >> preshift;
		}
	}
#endif

/* Conceptually, place  the real part of w[] into the even-indexed time-domain output
values and the imaginary part of w[] into the odd-indexed time-domain output values.
The casting of the (int *) time-domain function argument to (icomplex *) makes
this happen automatically. */

}

/* ***************************************************************************** */
/*
void compute_dft16_constants()
{
#define SCALE_DFT16 8192
#define SHIFT_DFT16 13
#define ROUND_DFT16 4096

	FILE *fp;

	if ((fp = fopen("dft16table","w")) == NULL) {
		printf("cannot open file dBTable.\n");
		exit(1);
	}

	fprintf(fp,"#define ic80 %6d\n",(int) (0.70710678*SCALE_DFT16 + 0.5));
	fprintf(fp,"#define ic161 %6d\n",(int) (0.38268343*SCALE_DFT16 + 0.5));
	fprintf(fp,"#define ic162 %6d\n",(int) (1.30656297*SCALE_DFT16 + 0.5));
	fprintf(fp,"#define ic163 %6d\n",(int) (0.54119610*SCALE_DFT16 + 0.5));
	fprintf(fp,"#define ic164 %6d\n",(int) (0.92387953*SCALE_DFT16 + 0.5));

	fclose(fp);
}
*/


void FFT16CtoC(icomplex v[])
{
/*	const short SCALE_DFT16 = 8192;
	const short SHIFT_DFT16 = 13;
	const short ROUND_DFT16 = 4096; */
#define SCALE_DFT16 8192
#define SHIFT_DFT16 13
#define ROUND_DFT16 4096

/*
#define ic80   5793
#define ic161   3135
#define ic162  10703
#define ic163   4433
#define ic164   7568
*/

	int r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15;
	int t0,t1,t2,t3,t4,t5,t6,t7;
	int s1,s2,s3,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15;

	r0 = v[0].r + v[8].r;
	r1 = v[0].r - v[8].r;
	r2 = v[1].r + v[9].r;
	r3 = v[1].r - v[9].r;
	r4 = v[2].r + v[10].r;
	r5 = v[2].r - v[10].r;
	r6 = v[3].r + v[11].r;
	r7 = v[3].r - v[11].r;
	r8 = v[4].r + v[12].r;
	r9 = v[4].r - v[12].r;
	r10 = v[5].r + v[13].r;
	r11 = v[5].r - v[13].r;
	r12 = v[6].r + v[14].r;
	r13 = v[6].r - v[14].r;
	r14 = v[7].r + v[15].r;
	r15 = v[7].r - v[15].r;

	t0 = r0 + r8;
	t1 = r0 - r8;
	t2 = r2 + r10;
	t3 = r2 - r10;
	t4 = r4 + r12;
	t5 = r4 - r12;
	t6 = r6 + r14;
	t7 = r6 - r14;

	r0 = t0 + t4;
	r2 = t0 - t4;
	r4 = t2 + t6;
	r6 = t2 - t6;

	v[0].r = r0 + r4;
	v[8].r = r0 - r4;
	t0 = (ic80 * (t3 + t7) + ROUND_DFT16) >> SHIFT_DFT16;
	t4 = (ic80 * (t3 - t7) + ROUND_DFT16) >> SHIFT_DFT16;
	r8 = t1 + t4;
	r10 = t1 - t4;
	r12 = t5 + t0;
	r14 = t5 - t0;
	t0 = r3 + r15;
	t1 = r3 - r15;
	t2 = (ic80 * (r5 + r13) + ROUND_DFT16) >> SHIFT_DFT16;
	t3 = (ic80 * (r5 - r13) + ROUND_DFT16) >> SHIFT_DFT16;
	t4 = r7 + r11;
	t5 = r7 - r11;
	t6 = (ic161 * (t1 - t5) + ROUND_DFT16) >> SHIFT_DFT16;
	t1 = (((ic162 * t1) + ROUND_DFT16) >> SHIFT_DFT16) - t6;

	t5 = (((ic163 * t5) + ROUND_DFT16) >> SHIFT_DFT16) - t6;
	t6 = r1 + t3;
	t7 = r1 - t3;
	r1 = t6 + t1;
	r3 = t6 - t1;
	r5 = t7 + t5;
	r7 = t7 - t5;
	t6 = ((ic164 * (t0 + t4)) + ROUND_DFT16) >> SHIFT_DFT16;
	t1 = t6 - (((ic163 * t0) + ROUND_DFT16) >> SHIFT_DFT16);
	t3 = t6 - (((ic162 * t4) + ROUND_DFT16) >> SHIFT_DFT16);
	t5 = r9 + t2;
	t7 = r9 - t2;
	r9 = t5 + t1;
	r11 = t5 - t1;
	r13 = t7 + t3;
	r15 = t7 - t3;

	r0 = v[0].i + v[8].i;
	s1 = v[0].i - v[8].i;
	s2 = v[1].i + v[9].i;
	s3 = v[1].i - v[9].i;
	r4 = v[2].i + v[10].i;
	s5 = v[2].i - v[10].i;
	s6 = v[3].i + v[11].i;
	s7 = v[3].i - v[11].i;
	s8 = v[4].i + v[12].i;
	s9 = v[4].i - v[12].i;
	s10 = v[5].i + v[13].i;
	s11 = v[5].i - v[13].i;
	s12 = v[6].i + v[14].i;
	s13 = v[6].i - v[14].i;
	s14 = v[7].i + v[15].i;
	s15 = v[7].i - v[15].i;

	t0 = r0 + s8;
	t1 = r0 - s8;
	t2 = s2 + s10;
	t3 = s2 - s10;
	t4 = r4 + s12;
	t5 = r4 - s12;
	t6 = s6 + s14;
	t7 = s6 - s14;

	r0 = t0 + t4;
	s2 = t0 - t4;
	r4 = t2 + t6;
	s6 = t2 - t6;

	v[0].i = r0 + r4;
	v[8].i = r0 - r4;
	v[4].r = r2 + s6;
	v[12].r = r2 - s6;
	v[4].i = s2 - r6;
	v[12].i = s2 + r6;

	t0 = (ic80 * (t3 + t7) + ROUND_DFT16) >> SHIFT_DFT16;
	t4 = (ic80 * (t3 - t7) + ROUND_DFT16) >> SHIFT_DFT16;
	s8 = t1 + t4;
	s10 = t1 - t4;
	s12 = t5 + t0;
	s14 = t5 - t0;
	t0 = s3 + s15;
	t1 = s3 - s15;
	t2 = (ic80 * (s5 + s13) + ROUND_DFT16) >> SHIFT_DFT16;
	t3 = (ic80 * (s5 - s13) + ROUND_DFT16) >> SHIFT_DFT16;
	t4 = s7 + s11;
	t5 = s7 - s11;
	t6 = (ic161 * (t1 - t5) + ROUND_DFT16) >> SHIFT_DFT16;
	t1 = (((ic162 * t1) + ROUND_DFT16) >> SHIFT_DFT16) - t6;
	t5 = (((ic163 * t5) + ROUND_DFT16) >> SHIFT_DFT16) - t6;
	t6 = s1 + t3;
	t7 = s1 - t3;
	s1 = t6 + t1;
	s3 = t6 - t1;
	s5 = t7 + t5;
	s7 = t7 - t5;
	t6 = (ic164 * (t0 + t4) + ROUND_DFT16) >> SHIFT_DFT16;
	t1 = t6 - (((ic163 * t0) + ROUND_DFT16) >> SHIFT_DFT16);
	t3 = t6 - (((ic162 * t4) + ROUND_DFT16) >> SHIFT_DFT16);
	t5 = s9 + t2;
	t7 = s9 - t2;
	s9 = t5 + t1;
	s11 = t5 - t1;
	s13 = t7 + t3;
	s15 = t7 - t3;

	v[1].r = r1 + s9;
	v[15].r = r1 - s9;
	v[1].i = s1 - r9;
	v[15].i = s1 + r9;

	v[2].r = r8 + s12;
	v[14].r = r8 - s12;
	v[2].i = s8 - r12;
	v[14].i = s8 + r12;

	v[3].r = r7 - s15;
	v[13].r = r7 + s15;
	v[3].i = s7 + r15;
	v[13].i = s7 - r15;

	v[5].r = r5 + s13;
	v[11].r = r5 - s13;
	v[5].i = s5 - r13;
	v[11].i = s5 + r13;

	v[6].r = r10 - s14;
	v[10].r = r10 + s14;
	v[6].i = s10 + r14;
	v[10].i = s10 - r14;

	v[7].r = r3 - s11;
	v[9].r = r3 + s11;
	v[7].i = s3 + r11;
	v[9].i = s3 - r11;
}

/* ***************************************************************************** */

void getdft80tables()
{
	int *indexptr;
	int k;
	int j,i,l;

	indexptr = IndexMapInc5Mod80;
	*indexptr = 0;
	for (j=0; j<5; j++) {
		for (l=1; l<16; l++) {
			k = *indexptr++ + 5;
			*indexptr = k;
			if (*indexptr >= 80) *indexptr -= 80;
		}
		if (j < 4) {
			*indexptr++;
			*indexptr = *(indexptr-16) + 16;
		}
	}

	indexptr = IndexMapInc16Mod80;
	*indexptr = 0;
	for (j=0; j<16; j++) {
		for (l=1; l<5; l++) {
			k = *indexptr++ + 16;
			*indexptr = k;
			if (*indexptr >= 80) *indexptr -= 80;
		}
		if (j < 15) {
			*indexptr++;
			*indexptr = *(indexptr-5) + 5;
		}
	}


/* For unscrambling the fft output. */
	for (i=0; i<80; i++)
		IndexMapInc21Mod80[i] = (i*21)%80;

/*	for (j=0; j<80; j++) {
		printf("%2d ",IndexMapInc5Mod80[j]);
		if (j%16 == 15) printf("\n");
	}
	getchar();
	for (j=0; j<80; j++) {
		printf("%2d ",IndexMapInc16Mod80[j]);
		if (j%5 == 4) printf("\n");
	}
	getchar();
	for (j=0; j<80; j++) {
		printf("%2d ",IndexMapInc21Mod80[j]);
		if (j%20 == 19) printf("\n");
	}
	getchar(); */

	printf("getdft80tables(): done\n");
}



void FFT80CtoC(icomplex x[])
{
	icomplex v[16];
	int *indexptr;
	int i,j;
	icomplex fftout[80];

// pre-normalization
	int pow;
	int temp0,temp1;
	int preshift;
	int round;
	pow = 0;
// Unroll this loop.
	for (i=0; i<80; i++) {
		temp0 = x[i].r >> 4;
		temp1 = x[i].i >> 4;;
		pow += temp0*temp0 + temp1*temp1;
	}
//	printf("pow = %d ",pow);
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
//	printf("%2d ",i);
	preshift = (21-i) >> 1;

//	printf("%1d ",preshift);

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
		for (i=0; i<80; i++) {
			x[i].r = (x[i].r + round) >> j;
			x[i].i = (x[i].i + round) >> j;
		}
	}
	else if (preshift > 0) {
		for (i=0; i<80; i++ ) {
			x[i].r = x[i].r << preshift;
			x[i].i = x[i].i << preshift;
		}
	}
// Combine above shifting with below reordering into v[].

// Do 5 16-point complex dfts.
	indexptr = IndexMapInc5Mod80;
	for (j=0; j<5; j++) {
		for (i=0; i<16; i++) {
			v[i].r = x[*indexptr].r;
			v[i].i = x[*indexptr++].i;
		}
		FFT16CtoC(v);
		indexptr -= 16;
		for (i=0; i<16; i++) {
			x[*indexptr].r = v[i].r;
			x[*indexptr++].i = v[i].i;
		}
	}

// Do 16 5-point complex dfts.
	indexptr = IndexMapInc16Mod80;
	for (j=0; j<16; j++) {
		for (i=0; i<5; i++) {
			v[i].r = x[*indexptr].r;
			v[i].i = x[*indexptr++].i;
		}
		FFT5CtoC(v);
		indexptr -= 5;
		for (i=0; i<5; i++) {
			x[*indexptr].r = v[i].r;
			x[*indexptr++].i = v[i].i;
		}
	}

// DEBUG!!! Combine this with unscramble.
	if (preshift < 0) {		// shift output up
		j = -preshift;
		for (i=0; i<80; i++) {
			x[i].r = x[i].r << j;
			x[i].i = x[i].i << j;
		}
	}
	else if (preshift > 0) {
//		round = 0;
		round = 1 << (preshift-1);
		for (i=0; i<80; i++) {
			x[i].r = (x[i].r + round) >> preshift;
			x[i].i = (x[i].i + round) >> preshift;
		}
	}



/* Unscramble the output. */
/* This can be done without a whole array of work space. */
	for (i=0; i<80; i++) {
		j = IndexMapInc21Mod80[i];
		fftout[i].r = x[j].r;
		fftout[i].i = x[j].i;
	}
	for (i=0; i<80; i++) {
		x[i].r = fftout[i].r;
		x[i].i = fftout[i].i;
	}

}








/* ***************************************************************************** */
/*
void ComputeFFT16RtoCCosSinTable()
{
	FILE *fp;
	int FFT16RtoCCosSinTable[38];
	int i,j;

	if ((fp = fopen("FFT16RtoCCosSinTable","w")) == NULL) {
		printf("cannot open file FFT16RtoCCosSinTable.\n");
		exit(1);
	}

#define PI (double) 3.1415926539
	int *CosSinPtr;
	CosSinPtr = FFT16RtoCCosSinTable;
	for (i=1; i < 4; i++) {
		*CosSinPtr++ = (int) (0.5 + 32768.0 * cos(2.0*PI*i/16.0));
		*CosSinPtr++ = (int) (0.5 + 32768.0 * sin(2.0*PI*i/16.0));
	}

	fprintf(fp,"const int FFT16RtoCCosSinTable[6] = {\n");
	for (j=0; j<6; j++) {
		fprintf(fp,"%5d",FFT16RtoCCosSinTable[j]);
		if (j<37) fprintf(fp,", ");
		if (j%10 == 9) fprintf(fp,"\n");
	}
	fprintf(fp,"};\n");
	fclose(fp);

}

*/

/* ***************************************************************************** */
#if (1)
#define NORM_FFT16RtoC_CONSTANT 19		// It's possible we could go to 20 without saturation
#define NORM_FFT16CtoR_CONSTANT 19

/* This function computes the first 9 complex values of the (symmetric) DFT of a 16-point
real array. */

void FFT16RtoC(icomplex w[])
{
	int i;

/* Conceptually, place the even-indexed input values into the real part of w[] and the
odd-indexed values into the imaginary part of w[]. The casting of the (int *) time-domain
function argument to (icomplex *) makes this happen automatically. */

#if (1)
// pre-normalization
	int j;
	int pow;
//	int temp0,temp1;
	int preshift;
	int round;

	pow = 0;
	for (i=0; i<8; i ++) {
#if USE_IMAGSQ
		pow += imagsq(&(w[i]), 4);
#else
		temp0 = w[i].r >> 4;
		temp1 = w[i].i >> 4;
		pow += temp0*temp0 + temp1*temp1;
#endif
	}
//	printf("pow = %d ",pow);
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
//	printf("%2d ",i);
	preshift = (NORM_FFT16RtoC_CONSTANT-i) >> 1;

//	printf("%1d ",preshift);

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
		for (i=0; i<8; i++) {
			w[i].r = (w[i].r + round) >> j;
			w[i].i = (w[i].i + round) >> j;
		}
	}
	else if (preshift > 0) {
		for (i=0; i<8; i++) {
			w[i].r = w[i].r << preshift;
			w[i].i = w[i].i << preshift;
		}
	}
#endif


/* Compute a 8-point complex DFT in place. */
	FFT8CtoC(w);

	int h1r,h1i,h2r,h2i;

/* Put the 8-point real-input 5-point complex output DFT of the even samples
(real part of y[]-input) into u[], and the DFT of the odd samples (imaginary part
of y[]-input) into v[].
Construct the output w[i] = u[i] + (v[i] * e**(-j*2*PI*i/16)).
(The modulation of v[] compensates for the 1-sample time-shift of the odd samples.)
Since we do this all in-place, there are no actual u[] and v[] arrays. */

	h1r = w[0].r;
	h1i = w[0].i;
	w[0].r = h1r + h1i;
	w[8].r = h1r - h1i;
	w[0].i = 0;
	w[8].i = 0;
//	w[4].r = w[4].r;
	w[4].i = -w[4].i;

	int *CosSinPtr;
	int rcos,rsin,icos,isin;
	CosSinPtr = FFT16RtoCCosSinTable;

	for (i=1; i < 4; i++) {
		h1r = (w[i].r + w[8-i].r) >> 1;
		h1i = (w[i].i - w[8-i].i) >> 1;
		h2r = (w[i].i + w[8-i].i) >> 1;
		h2i = (w[8-i].r - w[i].r) >> 1;

		icos = *CosSinPtr++;		// cos(2.0*PI*i/16.0)
		isin = *CosSinPtr++;		// sin(2.0*PI*i/16.0)

		rcos = (h2r * icos + 16384) >> 15;
		rsin = (h2r * isin + 16384) >> 15;
		icos = (icos * h2i + 16384) >> 15;
		isin = (isin * h2i + 16384) >> 15;

		rcos += isin;
		icos -= rsin;

		w[i].r = h1r + rcos;
		w[i].i = h1i + icos;
		w[8-i].r = h1r - rcos;
		w[8-i].i = icos - h1i;
	}


#if (1)
	if (preshift < 0) {		// shift output up
		j = -preshift;
		for (i=0; i <= 8; i++) {
			w[i].r = w[i].r << j;
			w[i].i = w[i].i << j;
		}
	}
	else if (preshift > 0) {
		round = 1 << (preshift-1);
		for (i=0; i <= 8; i++) {
			w[i].r = (w[i].r + round) >> preshift;
			w[i].i = (w[i].i + round) >> preshift;
		}
	}
#endif

}
#endif
/* ***************************************************************************** */
/* This function computes the 16 real time-domain samples from the first
9 complex DFT values of a 16-point spectrum whose real part is even and
imaginary part is odd. */

void FFT16CtoR(icomplex w[])
{
	int i;
	int h1r,h1i,h2r,h2i;


#if (1)
// pre-normalization
	int j;
	int pow;
//	int temp0,temp1;
	int preshift;
	int round;

	pow = 0;
	for (i=0; i <= 8; i ++) {
#if USE_IMAGSQ
		pow += imagsq(&(w[i]), 4);
#else
		temp0 = w[i].r >> 4;
		temp1 = w[i].i >> 4;;
		pow += temp0*temp0 + temp1*temp1;
#endif
	}
//	printf("pow = %d ",pow);
	i = 0;
	while (pow > 0) {
		pow >>= 1;
		i++;
	}
//	printf("%2d ",i);
	preshift = (NORM_FFT16CtoR_CONSTANT-i) >> 1;

//	printf("%1d ",preshift);

	if (preshift < 0) {		// shift input down
		j = -preshift;
		round = 1 << (j-1);
		for (i=0; i <= 8; i++) {
			w[i].r = (w[i].r + round) >> j;
			w[i].i = (w[i].i + round) >> j;
		}
	}
	else if (preshift > 0) {
		for (i=0; i <= 8; i++) {
			w[i].r = w[i].r << preshift;
			w[i].i = w[i].i << preshift;
		}
	}
#endif



	h1r = w[0].r;
	h1i = w[8].r;
	w[0].r = h1r + h1i;
	w[0].i = h1r - h1i;

	w[4].r = w[4].r << 1;
	w[4].i = (-w[4].i) << 1;

	int *CosSinPtr;
	int rcos,rsin,icos,isin;
	CosSinPtr = FFT16RtoCCosSinTable;

	for (i=1; i < 4; i++) {

		h1r = (w[i].r + w[8-i].r);
		h1i = (w[8-i].i - w[i].i);
		h2r = (w[i].i + w[8-i].i);
		h2i = (w[i].r - w[8-i].r);

		icos = *CosSinPtr++;		// cos(2.0*PI*i/16.0)
		isin = *CosSinPtr++;		// sin(2.0*PI*i/16.0)

		rcos = (h2r * icos + 16384) >> 15;
		rsin = (h2r * isin + 16384) >> 15;
		icos = (icos * h2i + 16384) >> 15;
		isin = (isin * h2i + 16384) >> 15;

		rcos += isin;
		icos -= rsin;

		w[i].r = h1r + rcos;
		w[i].i = h1i + icos;
		w[8-i].r = h1r - rcos;
		w[8-i].i = icos - h1i;
	}

/* Compute a 8-point complex DFT in place. */
	FFT8CtoC(w);


#if (1)
	if (preshift < 0) {		// shift output up
		j = -preshift;
		for (i=0; i < 8; i++) {
			w[i].r = w[i].r << j;
			w[i].i = w[i].i << j;
		}
	}
	else if (preshift > 0) {
		round = 1 << (preshift-1);
		for (i=0; i < 8; i++) {
			w[i].r = (w[i].r + round) >> preshift;
			w[i].i = (w[i].i + round) >> preshift;
		}
	}
#endif

/* Conceptually, place  the real part of w[] into the even-indexed time-domain output
values and the imaginary part of w[] into the odd-indexed time-domain output values.
The casting of the (int *) time-domain function argument to (icomplex *) makes
this happen automatically. */

}

/* ***************************************************************************** */


/* ***************************************************************************** */
// pseudo-random number generator: returns integer ranging from 0 to 32767
int nrand()		// From Kernighan and Ritchie 2nd edition page 46
{
	rngseed = rngseed * 1103515245 + 12345;
	return ((unsigned int) (rngseed/65536) % 32768);
}

int myrand()
{
	int sum;
	sum = 2*(nrand() - 16384);
	sum += ((nrand() - 16384) >> 2);
	sum >>= 3;
	return(sum);
}

#endif /* ] __pingtel_on_posix__ */
