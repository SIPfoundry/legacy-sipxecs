//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef __pingtel_on_posix__ /* [ */

#define VX1_PC0 1    // Set to 1 for VXWORKS, 0 for PC simulation

////////////////////////////////// common defines ////////////////////

#define TWOM (2*M)
#define WINDOWSIZE (M*6)

//#define EXTRA_MIC_DELAY 160      // Probably, this would be 0 in the real-time product
static int EXTRA_LDSPKR_DELAY = 0;
#define MAX_EXTRA_LDSPKR_DELAY 3000
#define MAX_EXTRA_MIC_DELAY 3000
#define EXTRA_MIC_DELAY 0    // 224 Probably, this would be 0 in the real-time product
//#define EXTRA_LDSPKR_DELAY 0

#define COMPUTE_BAND_ERL 1

/////////////////////////////////////////////////////////////////////

int iLoudspeakerFadeDB;
int iDoubleTalkIndicatorFlag;

static int iEchoSup = 0;
int ESup(int iFlag )
{

   int save = iEchoSup;
   iEchoSup = iFlag;
   return (save);
}

static int EchoSuppressionX0 = 25;
static int EchoSuppressionX1 = 55;
static int EchoSuppressionY1 = 12;
static int EchoSuppressionX2 = 66;
static int EchoSuppressionY2 = 37;

int SupTweak(int X0, int X1, int Y1, int X2, int Y2 )
{
   EchoSuppressionX0 = X0;
   EchoSuppressionX1 = X1;
   EchoSuppressionY1 = Y1;
   EchoSuppressionX2 = X2;
   EchoSuppressionY2 = Y2;

   return(1);
}

static int EchoSuppressionDIThresh = 1000;
static int EchoSuppressionDIAvgConst = 60;
static int EchoSuppressionDISubConst = 32;

static int EchoSuppressionHoldTime = 24;
static int EchoSuppressionMaxDecay = -4096;

int TimeTweak(int Hold, int Decay)
{
   EchoSuppressionHoldTime = Hold;
   EchoSuppressionMaxDecay = Decay;

   return(1);
}


static int UpdateControl = 1;

int Upd(int iFlag )
{

   int save = UpdateControl;
   UpdateControl = iFlag;
   return (save);
}

#define USE_IMAGSQ 1

#include "mp/FilterBank.h"

#if (VX1_PC0 == 1) //////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mp/dsplib.h"

#else    ////////////////////////////////////// #else for VX1_PC0
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define MAX_NUM_TAPS 40
#endif      ///////////////////////  #endif for VX1_PC0


#define ECLEN1 22
#define ECLEN2 (ECLEN1)
#define ECLEN3 (ECLEN1)
#define ECLEN4 (ECLEN1)
/* Until EXTRA_LDSPKR_DELAY is determined make these all 1.
static int GradingShift[MAX_NUM_TAPS] = {
    3,2,1,0,0,0,0,0,1,1,
    1,1,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2};
*/
static int GradingShift[MAX_NUM_TAPS] = {
    2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2};

#if (VX1_PC0 == 1) //////////////////////////////////////////////////////////


// Constructor
FilterBank::FilterBank()
{
   FilterBankInit();
   FilterBankReinit();
}

// Destructor
FilterBank::~FilterBank()
{
}


#ifdef _VXWORKS /* [ */
         extern volatile int* pOsTC; // SA1110 timer

         int before;
         int after0;
         int after1;
         int after2;
         int after3;
         int after4mic;
         int after5mic;
         int after6mic;
         int after7mic;
         int after8mic;
         int after4spkr;
         int after5spkr;
         int after6spkr;
         int after7spkr;
         int after8spkr;
         int after9;
         int after10;

#endif /* _VXWORKS ] */
#define ARITHMETIC_TYPE 3

       static int ECNumTaps[NUM_BANDS_PROCESSED] = {
        ECLEN1,ECLEN1,ECLEN1,ECLEN1,ECLEN1,
        ECLEN2,ECLEN2,ECLEN2,ECLEN2,ECLEN2,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4,ECLEN4,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4};


#else    ////////////////////////////////////// #else for VX1_PC0


#define M 40
#define DEBUG_NUM_BANDS 39
   #if (DEBUG_NUM_BANDS == 41)
       #define LOW_BAND 0
       #define HIGH_BAND 40
   #else
       #define LOW_BAND 1
       #define HIGH_BAND 39
   #endif

#define NUM_BANDS_PROCESSED ((HIGH_BAND - LOW_BAND) + 1)

#if (DEBUG_NUM_BANDS == 41)
       static int ECNumTaps[NUM_BANDS_PROCESSED] = {
        ECLEN1,ECLEN1,ECLEN1,ECLEN1,ECLEN1,
        ECLEN2,ECLEN2,ECLEN2,ECLEN2,ECLEN2,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4,ECLEN4,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4,ECLEN4,ECLEN4};
   #else
       static int ECNumTaps[NUM_BANDS_PROCESSED] = {
        ECLEN1,ECLEN1,ECLEN1,ECLEN1,ECLEN1,
        ECLEN2,ECLEN2,ECLEN2,ECLEN2,ECLEN2,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN3,ECLEN3,ECLEN3,ECLEN3,ECLEN3,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4,ECLEN4,
        ECLEN4,ECLEN4,ECLEN4,ECLEN4};
   #endif

   int imagsq(icomplex *, int);
   void FilterBankInit();
   void FilterBankReinit();
   void FilterBankReport();
   void FilterBankFinalReport(int);
   void FilterBank(short *, short *, short *, int);
   void TwoFrameFilterBankAnalysis(icomplex [][M+1], int *, int *, int *);
   int DoubletalkDetection(icomplex [][M+1], icomplex [][M+1], int [], int []);
   void ComputeLoudspeakerFade();
   void SubbandECLoop(icomplex [][M+1], icomplex [][M+1], int [], int [], int []);
   void EchoSuppress(icomplex [][M+1], int[], int [], int [], int);


   void TestDft_float();

   static icomplex EchoCancellerCoef[NUM_BANDS_PROCESSED][MAX_NUM_TAPS];
   #define ECDL_SIZE (NUM_BANDS_PROCESSED*MAX_NUM_TAPS)     // Unnecessarily large
   static icomplex ECDL[ECDL_SIZE];          // DEBUG!!! Determine size later.

#define ARITHMETIC_TYPE 1
#if (ARITHMETIC_TYPE == 3)
   void ComplexInnerProduct(icomplex *, icomplex *, icomplex *, int);
#endif
   void ComplexCoefUpdate(icomplex *, icomplex *, icomplex *, int);

   extern FILE *fp_stats;

#endif      ///////////////////////  #endif for VX1_PC0

/* ****** Echo suppression table ************************** */
/* This table allows a maximum loss for echo suppression of 42 dB. */
   static int EchoSupMultiplierTable[43] = {
      4096, 3649, 3251, 2896, 2580, 2299,
      2048, 1825, 1625, 1448, 1290, 1149,
      1024,  912,  813,  724,  645,  575,
      512,  456,  406,  362,  323,  287,
      256,  228,  203,  181,  161,  144,
      128,  114,  102,   91,   81,   72,
       64,   57,   51,   45,   40,   36,
       32};

#define MAX_TARGET_ECHO_SUP_DB (40*1024-1)


static int ECDLHighestAddressBand;
static int ECDLHighestAddressBandNewestIndex;

static int CoefExamineBand;
static int BandLowDelayPower[NUM_BANDS_PROCESSED];
static int BandTailPower[NUM_BANDS_PROCESSED];
static int EchoDecaydB;                 // Echo canceller main lobe to tail power ratio (100 x dB)


static int DoubletalkIndicator;     // This should be available for loudspeaker fade
static int LdspkrDLPow[NUM_BANDS_PROCESSED];
static int WeightedLdspkrDLPow[NUM_BANDS_PROCESSED];
static int SumWeightedLdspkrPowDBMax;
static int DelayedMic[TWOM + MAX_EXTRA_MIC_DELAY];
static int DelayedLdspkr[TWOM + MAX_EXTRA_LDSPKR_DELAY];

static int TwoFrameMicAnalysisDL[WINDOWSIZE+M];
static int TwoFrameLdspkrAnalysisDL[WINDOWSIZE+M];

static int SynthesisDL[WINDOWSIZE];

static int LdspkrMicGain[NUM_BANDS_PROCESSED];     // For doubletalk detection
static int LMGThresh0[NUM_BANDS_PROCESSED];     // For doubletalk detection

#if COMPUTE_BAND_ERL
static int SumFBMicPow[NUM_BANDS_PROCESSED];
static int SumFBResPow[NUM_BANDS_PROCESSED];
#endif

static int Frame10msCount;


static int ThresholdLdspkrDLPow[NUM_BANDS_PROCESSED];
static int DebugUpdateCount[NUM_BANDS_PROCESSED];

static int EchoSupDB;

#define DEBUG_ECHO_ESTIMATE_POW 0
#if DEBUG_ECHO_ESTIMATE_POW
static int EchoEstimatePow[NUM_BANDS_PROCESSED];      // new Aug 8, 2002, DEBUG!!!
#endif

static int MicPow[NUM_BANDS_PROCESSED];      // Now used in LoudspeakerFade()
static int SumMicPow;       // used in LoudspeakerFade() and EchoSuppress()


#if (VX1_PC0 == 1)
void FilterBank::FilterBankReinit()
#else    // #else for VX1_PC0
void FilterBankReinit()
#endif
{
   int i,j;
   int Band;


   for (j=0; j < (TWOM + EXTRA_MIC_DELAY); j++)
      DelayedMic[j] = 0;

   for (j=0; j < (TWOM + EXTRA_LDSPKR_DELAY); j++)
      DelayedLdspkr[j] = 0;

   for (j=0; j<WINDOWSIZE+M; j++)
       TwoFrameMicAnalysisDL[j] = 0;

   for (j=0; j<WINDOWSIZE+M; j++)
       TwoFrameLdspkrAnalysisDL[j] = 0;

   for (j=0; j<WINDOWSIZE; j++)
       SynthesisDL[j] = 0;

   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      LdspkrDLPow[Band] = 0;
   }

   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      WeightedLdspkrDLPow[Band] = 0;
   }

#define THRESHOLD_SCALE 8
   ThresholdLdspkrDLPow[0] = THRESHOLD_SCALE*8192; // higher means less frequent update
   ThresholdLdspkrDLPow[1] = THRESHOLD_SCALE*2048; // higher means less frequent update
   ThresholdLdspkrDLPow[2] = THRESHOLD_SCALE*2048; // higher means less frequent update
   for (Band=3; Band < 8; Band++)
   {
      ThresholdLdspkrDLPow[Band] = ThresholdLdspkrDLPow[Band-1];
   }
   for (Band=8; Band < 11; Band++)
   {
      ThresholdLdspkrDLPow[Band] = (26*ThresholdLdspkrDLPow[Band-1]) >> 5;
   }
   for (Band=11; Band < 14; Band++)
   {
      ThresholdLdspkrDLPow[Band] = (33*ThresholdLdspkrDLPow[Band-1]) >> 5;
   }
   for (Band=14; Band < 20; Band++)
   {
      ThresholdLdspkrDLPow[Band] = (28*ThresholdLdspkrDLPow[Band-1]) >> 5;
   }
   for (Band=20; Band < NUM_BANDS_PROCESSED; Band++)
   {
      ThresholdLdspkrDLPow[Band] = (30*ThresholdLdspkrDLPow[Band-1]) >> 5;
   }


#if (0)
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      ThresholdLdspkrDLPow[Band] = 0x40;
   }
#endif


   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      DebugUpdateCount[Band] = 0;
   }


   ECDLHighestAddressBand = NUM_BANDS_PROCESSED-1;
   ECDLHighestAddressBandNewestIndex = -2;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      ECDLHighestAddressBandNewestIndex += (ECNumTaps[Band]-1);
   }

   for (i=0; i < ECDL_SIZE; i++)
   {
      ECDL[i].r = 0;
      ECDL[i].i = 0;
   }


   for (i=0; i < NUM_BANDS_PROCESSED; i++) {
      for (j=0; j<MAX_NUM_TAPS; j++) {
         EchoCancellerCoef[i][j].r = 0;
         EchoCancellerCoef[i][j].i = 0;
      }
   }
   CoefExamineBand = 0;

// Initialize loudspeaker to mic gain estimates (log scale).
   for (i = 0; i < NUM_BANDS_PROCESSED; i++) {
      LdspkrMicGain[i] = 0;
      LdspkrMicGain[i] = -12000;            // Was 0 until 7/22/02
   }

// Set threshold for how big LdspkrDB[] must be, before LdspkrMicGain[i] can be adjusted.
   for (i = 0; i < NUM_BANDS_PROCESSED; i++) {
      LMGThresh0[i] = 38000 - i*250;
   }

if (iEchoSup)
      EchoSupDB = 0;



#if COMPUTE_BAND_ERL
   for (i=0; i < NUM_BANDS_PROCESSED; i++) {
      SumFBMicPow[i] = 0;
      SumFBResPow[i] = 0;
   }
#endif

   Frame10msCount = 0;

}



#if (VX1_PC0 == 1)
static int TDSummic=0;     // Move to  VX1
static int TDSumres=0;     // Move to  VX1

void FilterBank::FilterBankInit()
{
   GetDft40Tables();
}

#else    // #else for VX1_PC0

static double dTDSummic, dTDSumres;
static double BigTDSummic;
static double BigTDSumres;
static double BigTDSumerl;

static int MaxLdspkrDLPow[NUM_BANDS_PROCESSED];

#if COMPUTE_BAND_ERL
static int MaxSumFBMicPow[NUM_BANDS_PROCESSED];
static int MaxSumFBResPow[NUM_BANDS_PROCESSED];
#endif

void FilterBankInit()
{
   GetDft40Tables();

   BigTDSummic = 0.0;
   BigTDSumres = 0.0;
   BigTDSumerl = 0.0;

   int Band;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      MaxLdspkrDLPow[Band] = 0;
#if COMPUTE_BAND_ERL
      MaxSumFBMicPow[Band] = 0;
      MaxSumFBResPow[Band] = 0;
#endif
   }
}

void FilterBankReport()
{
   double erldb;
   erldb = 10.0*log10(dTDSummic/dTDSumres);
   printf("fb td ERL = %6.2f\n",erldb);

   BigTDSummic += dTDSummic;
   BigTDSumres += dTDSumres;
   BigTDSumerl += erldb;

#if (1)
   int Band;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
      printf("%4d ",DebugUpdateCount[Band]);
      if (Band%14 == 13) printf("\n");
   }
   printf("\n");
#endif
}

void FilterBankFinalReport(int total_simulations)
{
   printf("FilterBank total td ERL = %6.2f avg ERL = %8.2f\n",
      10.0*log10(BigTDSummic/BigTDSumres),
      BigTDSumerl/total_simulations);

#if (0)
   int Band;
   printf("Band: MaxLdspkrDLPow[Band],MaxSumFBMicPow[Band],MaxSumFBResPow[Band]\n");
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      printf("%8x %8x %8x   ",MaxLdspkrDLPow[Band],MaxSumFBMicPow[Band],MaxSumFBResPow[Band]);
      if (Band%2 == 1) printf("\n");
   }
#endif

}
#endif      // #endif for VX1_PC0


/* ********************************************************************* */

#if 1    // DEBUG!!!!!! feb 1, 2005
void FilterBank::DoFilterBank(short mic[], short ldspkr[])
{
#define MIC_GAIN_SHIFT 3

	int i,flag;
	static int stage = 0;   // DEBUG!!!!! Feb 1, 2005
	static int micbuf[40000];
	long sum;
	int DCoffset;

	for (i=0; i<TWOM; i++)
		ldspkr[i] = 0;

	if (stage == 0) {
		if (Frame10msCount == 499) {
			stage = 1;
			osPrintf("stage = %d\n",stage);
			Frame10msCount = -1;
		}
	}


	else if (stage == 1) {

#if (0)
#define PULSE0 0
#define PULSE1 31767
#define PULSE2 0
		if (Frame10msCount == 100) {
			ldspkr[0] = PULSE0;
			ldspkr[1] = PULSE1;
			ldspkr[2] = PULSE2;
		}
		if (Frame10msCount == 200) {
			ldspkr[0] = PULSE0;
			ldspkr[1] = PULSE1;
			ldspkr[2] = PULSE2;
		}
		if (Frame10msCount == 300) {
			ldspkr[0] = PULSE0;
			ldspkr[1] = PULSE1;
			ldspkr[2] = PULSE2;
		}
		if (Frame10msCount == 400) {
			ldspkr[0] = PULSE0;
			ldspkr[1] = PULSE1;
			ldspkr[2] = PULSE2;
		}
#else

		static double Angle = 0.0;
		double dTemp0;
		int j;
		for (i=0; i<TWOM; i++) {
			dTemp0 = 8000.0 * sin(6.2831853*Angle);
			Angle += 800./8000.;
			if (dTemp0 >= 0.0) j = (int) (dTemp0 + 0.5);
			else j = (int) (dTemp0 - 0.5);
			ldspkr[i] = j;
		}

#endif
/*
		for (i=0; i<TWOM; i++)
			micbuf[TWOM*Frame10msCount + i] = mic[i];

		if (Frame10msCount == 499) {
			sum = 0;
			for (i=0; i<40000; i++)
				sum += micbuf[i];
			DCoffset = sum/40000;
			for (i=0; i<40000; i++)
				micbuf[i] -= DCoffset;

			stage = 2;
			osPrintf("stage = %d\n",stage);
			Frame10msCount = -1;
		}
*/
	}

	else if (stage == 2) {
		int n;
		n = Frame10msCount*80;
		flag = 0;
		for (i=n; i<n+80; i++) {
			if (abs(micbuf[i]) > 500) flag = 1;
		}
		if (flag > 0) {
			osPrintf("sample %5d   %5d ms\n",n,n/8);
			for (i=n; i<n+80; i++) {
				osPrintf("%6d ",micbuf[i]);
				if (i%10 == 9) osPrintf("\n");
			}
		}
		if (Frame10msCount == 499) {
			stage = 3;
			osPrintf("stage = %d\n",stage);
			Frame10msCount = -1;
		}

	}

	else if (stage == 3) {
	}

	Frame10msCount++;


}


#else  // DEBUG!!!!! Feb 1, 2005

#if (VX1_PC0 == 1)

void FilterBank::DoFilterBank(short mic[], short ldspkr[])
{

#ifdef _VXWORKS /* [ */
   /* 1000/3.6864 = 271.26736111111111 */
   /* 1000 nanoSec/microSec */
   /* 3.6864 MHz counter */
   /* 3 place accuracy */

   before = *pOsTC;
#endif /* _VXWORKS ] */

#else    // #else for VX1_PC0

void FilterBank(short fbecmic[], short mic[], short ldspkr[], int TotalSamples)
{

#endif      // #endif for VX1_PC0


#if 0 /* [ */
 0.01000   1.00000   0.10000
iflat =  8
static double weighting_template[128] = {
 0.01080, 0.01679, 0.02383, 0.03196, 0.04124, 0.05174, 0.06347, 0.07648, 0.09080, 0.10642,
 0.12336, 0.14160, 0.16113, 0.18191, 0.20390, 0.22704, 0.25127, 0.27651, 0.30267, 0.32966,
 0.35736, 0.38567, 0.41447, 0.44364, 0.47303, 0.50253, 0.53199, 0.56129, 0.59029, 0.61887,
 0.64689, 0.67425, 0.70082, 0.72651, 0.75121, 0.77485, 0.79736, 0.81866, 0.83870, 0.85746,
 0.87491, 0.89103, 0.90582, 0.91931, 0.93150, 0.94244, 0.95217, 0.96075, 0.96824, 0.97471,
 0.98023, 0.98489, 0.98876, 0.99193, 0.99449, 0.99651, 0.99808, 0.99927, 1.00015, 1.00078,
 1.00122, 1.00151, 1.00169, 1.00178, 1.00182, 1.00178, 1.00169, 1.00151, 1.00122, 1.00078,
 1.00015, 0.99927, 0.99808, 0.99651, 0.99449, 0.99193, 0.98876, 0.98489, 0.98023, 0.97471,
 0.96824, 0.96075, 0.95217, 0.94244, 0.93150, 0.91931, 0.90582, 0.89103, 0.87491, 0.85746,
 0.83870, 0.81866, 0.79736, 0.77485, 0.75121, 0.72651, 0.70082, 0.67425, 0.64689, 0.61887,
 0.59029, 0.56129, 0.53199, 0.50253, 0.47303, 0.44364, 0.41447, 0.38567, 0.35736, 0.32966,
 0.30267, 0.27651, 0.25127, 0.22704, 0.20390, 0.18191, 0.16113, 0.14160, 0.12336, 0.10642,
 0.09080, 0.07648, 0.06347, 0.05174, 0.04124, 0.03196, 0.02383, 0.01679};
static double ldspkr_weighting_pow[128] = {
 1.00363, 1.00362, 1.00357, 1.00349, 1.00338, 1.00322, 1.00302, 1.00276, 1.00243, 1.00204,
 1.00156, 1.00098, 1.00030, 0.99949, 0.99854, 0.99744, 0.99617, 0.99470, 0.99304, 0.99114,
 0.98901, 0.98661, 0.98393, 0.98095, 0.97764, 0.97400, 0.97000, 0.96562, 0.96085, 0.95566,
 0.95005, 0.94400, 0.93748, 0.93050, 0.92304, 0.91508, 0.90663, 0.89767, 0.88819, 0.87820,
 0.86769, 0.85666, 0.84512, 0.83307, 0.82052, 0.80747, 0.79393, 0.77993, 0.76547, 0.75057,
 0.73524, 0.71952, 0.70342, 0.68697, 0.67020, 0.65312, 0.63578, 0.61819, 0.60040, 0.58243,
 0.56432, 0.54610, 0.52781, 0.50948, 0.49115, 0.47284, 0.45461, 0.43647, 0.41847, 0.40063,
 0.38300, 0.36559, 0.34844, 0.33159, 0.31505, 0.29885, 0.28301, 0.26757, 0.25253, 0.23792,
 0.22376, 0.21005, 0.19681, 0.18406, 0.17179, 0.16002, 0.14874, 0.13797, 0.12771, 0.11794,
 0.10867, 0.09990, 0.09161, 0.08380, 0.07646, 0.06957, 0.06314, 0.05713, 0.05155, 0.04637,
 0.04158, 0.03716, 0.03309, 0.02937, 0.02596, 0.02286, 0.02005, 0.01751, 0.01522, 0.01316,
 0.01133, 0.00969, 0.00824, 0.00697, 0.00585, 0.00487, 0.00403, 0.00330, 0.00268, 0.00215,
 0.00170, 0.00133, 0.00102, 0.00077, 0.00057, 0.00041, 0.00028, 0.00019};
  0     0.00    0.02    0.02  -54.87    0.02     -58.23  -58.23  -70.09  -70.09
  1             0.02    0.02  -54.88    0.02     -57.92  -58.61  -69.06  -71.33
  2     0.00    0.02    0.02  -54.91    0.02     -57.68  -59.07  -68.18  -72.83
  3             0.02    0.02  -54.96    0.02     -57.51  -59.61  -67.43  -74.71
  4     0.00    0.02    0.01  -55.03    0.01     -57.40  -60.25  -66.79  -77.17
  5             0.02    0.01  -55.13    0.01     -57.35  -61.00  -66.24  -80.69
  6     0.00    0.02    0.01  -55.24    0.01     -57.37  -61.88  -65.78  -86.80
  7             0.02    0.01  -55.37    0.01     -57.44  -62.92  -65.39 -120.02
  8     0.00    0.02    0.01  -55.51    0.01     -57.59  -64.16  -65.07  -86.45
  9             0.02    0.01  -55.67    0.01     -57.80  -65.66  -64.82  -80.56
 10     0.00    0.02    0.01  -55.84    0.01     -58.09  -67.54  -64.63  -77.13
 11             0.02    0.00  -56.02    0.00     -58.45  -69.99  -64.49  -74.73
 12     0.00    0.02    0.00  -56.21    0.00     -58.90  -73.48  -64.41  -72.90
 13             0.02   -0.00  -56.39   -0.00     -59.46  -79.44  -64.39  -71.44
 14     0.00    0.02   -0.01  -56.57   -0.01     -60.12 -116.14  -64.43  -70.24
 15             0.02   -0.01  -56.74   -0.01     -60.92  -79.77  -64.52  -69.24
 16     0.00    0.02   -0.02  -56.89   -0.02     -61.89  -73.75  -64.67  -68.39
 17             0.02   -0.02  -57.02   -0.02     -63.07  -70.29  -64.88  -67.68
 18    -0.00    0.02   -0.03  -57.11   -0.03     -64.52  -67.89  -65.16  -67.07
 19             0.02   -0.04  -57.16   -0.04     -66.38  -66.07  -65.50  -66.55
 20    -0.02    0.02   -0.05  -57.17   -0.05     -68.88  -64.63  -65.93  -66.12
 21             0.02   -0.06  -57.13   -0.06     -72.55  -63.45  -66.43  -65.76
 22    -0.04    0.02   -0.07  -57.04   -0.07     -79.32  -62.48  -67.04  -65.48
 23             0.02   -0.08  -56.91   -0.08     -93.04  -61.67  -67.75  -65.25
 24    -0.07    0.02   -0.10  -56.74   -0.10     -76.22  -60.99  -68.60  -65.09
 25             0.01   -0.11  -56.54   -0.11     -70.83  -60.41  -69.60  -64.99
 26    -0.12    0.01   -0.13  -56.31   -0.13     -67.53  -59.94  -70.82  -64.94
 27             0.01   -0.15  -56.07   -0.15     -65.16  -59.55  -72.31  -64.95
 28    -0.17    0.01   -0.17  -55.82   -0.17     -63.32  -59.24  -74.20  -65.02
 29             0.01   -0.20  -55.56   -0.20     -61.84  -59.00  -76.71  -65.15
 30    -0.23    0.01   -0.22  -55.30   -0.22     -60.62  -58.82  -80.37  -65.33
 31             0.00   -0.25  -55.06   -0.25     -59.59  -58.71  -87.05  -65.58
 32    -0.30    0.00   -0.28  -54.83   -0.28     -58.71  -58.67 -102.33  -65.90
 33             0.00   -0.31  -54.62   -0.31     -57.96  -58.68  -84.42  -66.29
 34    -0.38   -0.00   -0.35  -54.42   -0.35     -57.33  -58.76  -78.97  -66.75
 35            -0.00   -0.39  -54.26   -0.39     -56.79  -58.90  -75.66  -67.31
 36    -0.47   -0.01   -0.43  -54.11   -0.43     -56.33  -59.11  -73.29  -67.96
 37            -0.01   -0.47  -54.00   -0.47     -55.96  -59.38  -71.46  -68.74
 38    -0.58   -0.01   -0.51  -53.92   -0.51     -55.66  -59.73  -69.99  -69.65
 39            -0.02   -0.56  -53.86   -0.56     -55.43  -60.15  -68.77  -70.74
 40    -0.69   -0.02   -0.62  -53.84   -0.62     -55.27  -60.66  -67.74  -72.06
 41            -0.02   -0.67  -53.85   -0.67     -55.18  -61.26  -66.86  -73.69
 42    -0.81   -0.03   -0.73  -53.90   -0.73     -55.16  -61.97  -66.11  -75.78
 43            -0.03   -0.79  -53.98   -0.79     -55.21  -62.82  -65.47  -78.63
 44    -0.95   -0.03   -0.86  -54.10   -0.86     -55.33  -63.82  -64.91  -83.01
 45            -0.04   -0.93  -54.25   -0.93     -55.53  -65.03  -64.44  -92.56
 46    -1.09   -0.04   -1.00  -54.43   -1.00     -55.81  -66.51  -64.05  -92.40
 47            -0.04   -1.08  -54.65   -1.08     -56.18  -68.36  -63.72  -82.90
 48    -1.25   -0.05   -1.16  -54.90   -1.16     -56.65  -70.81  -63.46  -78.48
 49            -0.05   -1.25  -55.16   -1.25     -57.23  -74.32  -63.25  -75.60
 50    -1.42   -0.05   -1.34  -55.45   -1.34     -57.96  -80.45  -63.11  -73.47
 51            -0.06   -1.43  -55.74   -1.43     -58.84 -109.48  -63.03  -71.79
 52    -1.60   -0.06   -1.53  -56.03   -1.53     -59.94  -79.84  -63.00  -70.43
 53            -0.06   -1.63  -56.28   -1.63     -61.32  -73.98  -63.03  -69.30
 54    -1.80   -0.06   -1.74  -56.48   -1.74     -63.10  -70.55  -63.12  -68.34
 55            -0.07   -1.85  -56.61   -1.85     -65.51  -68.13  -63.27  -67.52
 56    -2.01   -0.07   -1.97  -56.64   -1.97     -69.08  -66.28  -63.49  -66.81
 57            -0.07   -2.09  -56.56   -2.09     -75.66  -64.81  -63.78  -66.21
 58    -2.24   -0.07   -2.22  -56.36   -2.22     -90.83  -63.59  -64.14  -65.70
 59            -0.07   -2.35  -56.05   -2.35     -72.85  -62.58  -64.59  -65.26
 60    -2.48   -0.08   -2.48  -55.65   -2.48     -67.31  -61.72  -65.13  -64.90
 61            -0.08   -2.63  -55.19   -2.63     -63.92  -60.98  -65.78  -64.60
 62    -2.74   -0.08   -2.78  -54.69   -2.78     -61.48  -60.36  -66.55  -64.37
 63            -0.08   -2.93  -54.16   -2.93     -59.58  -59.83  -67.49  -64.19
 64    -3.01   -0.08   -3.09  -53.64   -3.09     -58.04  -59.39  -68.62  -64.08
 65            -0.08   -3.25  -53.13   -3.25     -56.76  -59.02  -70.03  -64.02
 66    -3.30   -0.08   -3.42  -52.65   -3.42     -55.68  -58.72  -71.80  -64.01
 67            -0.08   -3.60  -52.20   -3.60     -54.75  -58.49  -74.17  -64.07
 68    -3.62   -0.08   -3.78  -51.78   -3.78     -53.95  -58.33  -77.60  -64.18
 69            -0.07   -3.97  -51.41   -3.97     -53.27  -58.22  -83.63  -64.36
 70    -3.95   -0.07   -4.17  -51.08   -4.17     -52.69  -58.17 -113.91  -64.60
 71            -0.07   -4.37  -50.80   -4.37     -52.20  -58.19  -82.94  -64.91
 72    -4.31   -0.07   -4.58  -50.56   -4.58     -51.80  -58.26  -76.97  -65.30
 73            -0.07   -4.79  -50.38   -4.79     -51.47  -58.40  -73.45  -65.77
 74    -4.69   -0.06   -5.02  -50.25   -5.02     -51.22  -58.61  -70.94  -66.35
 75            -0.06   -5.25  -50.17   -5.25     -51.05  -58.88  -69.00  -67.03
 76    -5.11   -0.06   -5.48  -50.14   -5.48     -50.96  -59.23  -67.43  -67.86
 77            -0.06   -5.73  -50.17   -5.73     -50.94  -59.66  -66.12  -68.85
 78    -5.55   -0.05   -5.98  -50.27   -5.98     -51.01  -60.19  -65.00  -70.06
 79            -0.05   -6.24  -50.43   -6.24     -51.17  -60.83  -64.04  -71.58
 80    -6.02   -0.05   -6.50  -50.65   -6.50     -51.42  -61.60  -63.20  -73.53
 81            -0.04   -6.78  -50.95   -6.78     -51.78  -62.52  -62.47  -76.19
 82    -6.53   -0.04   -7.06  -51.32   -7.06     -52.26  -63.65  -61.84  -80.27
 83            -0.04   -7.35  -51.76   -7.35     -52.89  -65.04  -61.28  -88.71
 84    -7.09   -0.03   -7.65  -52.28   -7.65     -53.70  -66.81  -60.79  -91.90
 85            -0.03   -7.96  -52.86   -7.96     -54.74  -69.17  -60.37  -81.01
 86    -7.69   -0.03   -8.28  -53.48   -8.28     -56.10  -72.61  -60.01  -76.25
 87            -0.02   -8.60  -54.08   -8.60     -57.92  -78.72  -59.71  -73.16
 88    -8.34   -0.02   -8.94  -54.57   -8.94     -60.49 -104.17  -59.46  -70.86
 89            -0.02   -9.28  -54.83   -9.28     -64.58  -77.62  -59.27  -69.04
 90    -9.06   -0.01   -9.64  -54.73   -9.64     -73.65  -71.71  -59.13  -67.53
 91            -0.01  -10.00  -54.22  -10.00     -73.93  -68.18  -59.03  -66.26
 92    -9.86   -0.01  -10.38  -53.38  -10.38     -63.96  -65.66  -58.99  -65.15
 93            -0.00  -10.77  -52.33  -10.77     -59.26  -63.69  -59.00  -64.19
 94   -10.74   -0.00  -11.17  -51.19  -11.17     -56.12  -62.09  -59.06  -63.33
 95             0.00  -11.58  -50.04  -11.57     -53.74  -60.74  -59.18  -62.57
 96   -11.74    0.00  -12.00  -48.95  -12.00     -51.84  -59.58  -59.36  -61.90
 97             0.00  -12.43  -47.92  -12.43     -50.25  -58.57  -59.59  -61.29
 98   -12.88    0.01  -12.88  -46.96  -12.88     -48.89  -57.68  -59.90  -60.75
 99             0.01  -13.34  -46.08  -13.34     -47.72  -56.90  -60.27  -60.26
100   -14.20    0.01  -13.81  -45.28  -13.81     -46.69  -56.19  -60.73  -59.82
101             0.01  -14.30  -44.54  -14.30     -45.78  -55.57  -61.28  -59.43
102   -15.76    0.01  -14.80  -43.88  -14.80     -44.98  -55.00  -61.95  -59.09
103             0.01  -15.32  -43.29  -15.31     -44.28  -54.50  -62.74  -58.78
104   -17.69    0.02  -15.86  -42.76  -15.85     -43.66  -54.05  -63.70  -58.52
105             0.02  -16.41  -42.29  -16.40     -43.12  -53.65  -64.87  -58.29
106   -20.17    0.02  -16.98  -41.89  -16.96     -42.66  -53.29  -66.32  -58.10
107             0.02  -17.57  -41.55  -17.55     -42.28  -52.97  -68.19  -57.93
108   -23.69    0.02  -18.18  -41.27  -18.16     -41.96  -52.69  -70.72  -57.81
109             0.02  -18.81  -41.06  -18.78     -41.73  -52.45  -74.51  -57.71
110   -29.70    0.02  -19.46  -40.92  -19.43     -41.57  -52.25  -81.87  -57.65
111             0.02  -20.14  -40.85  -20.10     -41.49  -52.07  -90.27  -57.62
112    -1.#J    0.02  -20.84  -40.86  -20.80     -41.50  -51.93  -76.75  -57.61
113             0.02  -21.57  -40.95  -21.52     -41.61  -51.83  -71.60  -57.64
114    -1.#J    0.02  -22.33  -41.14  -22.27     -41.83  -51.75  -68.34  -57.70
115             0.02  -23.12  -41.44  -23.06     -42.17  -51.70  -65.94  -57.79
116    -1.#J    0.02  -23.95  -41.87  -23.88     -42.68  -51.69  -64.04  -57.92
117             0.02  -24.81  -42.45  -24.74     -43.38  -51.70  -62.47  -58.08
118    -1.#J    0.02  -25.72  -43.22  -25.65     -44.34  -51.74  -61.14  -58.27
119             0.02  -26.68  -44.23  -26.61     -45.66  -51.82  -60.00  -58.49
120    -1.#J    0.02  -27.69  -45.52  -27.62     -47.55  -51.92  -58.99  -58.76
121             0.02  -28.77  -47.13  -28.70     -50.43  -52.06  -58.09  -59.06
122    -1.#J    0.02  -29.91  -48.89  -29.85     -55.67  -52.22  -57.30  -59.40
123             0.02  -31.13  -49.97  -31.08     -80.31  -52.42  -56.58  -59.79
124    -1.#J    0.02  -32.46  -48.97  -32.36     -55.64  -52.65  -55.94  -60.22
125             0.02  -33.90  -46.41  -33.67     -48.86  -52.92  -55.36  -60.71
126    -1.#J    0.02  -35.50  -43.65  -34.88     -44.77  -53.22  -54.84  -61.25
127             0.02  -37.28  -41.16  -35.79     -41.76  -53.56  -54.37  -61.86
128    -1.#J    0.02  -39.33  -38.98  -36.14     -39.33  -53.94  -53.94  -62.54
in band to aliased ratio =   44.95199
in band to distant aliased ratio =   63.12118
overlapped passband max deviation from flat =    0.09669
in band to weighted aliased ratio =   54.36052
analysis filter reconstruction errors by phase:
phase =  0/ 40   0.9998   0.0053   0.0021
phase =  1/ 40   0.9998   0.0052   0.0016
phase =  2/ 40   0.9998   0.0051   0.0012
phase =  3/ 40   0.9998   0.0051   0.0007
phase =  4/ 40   0.9998   0.0051   0.0003
phase =  5/ 40   0.9998   0.0051  -0.0002
phase =  6/ 40   0.9998   0.0051  -0.0007
phase =  7/ 40   0.9999   0.0052  -0.0012
phase =  8/ 40   0.9999   0.0052  -0.0016
phase =  9/ 40   0.9999   0.0053  -0.0021
phase = 10/ 40   1.0000   0.0054  -0.0025
phase = 11/ 40   1.0000   0.0054  -0.0029
phase = 12/ 40   1.0001   0.0055  -0.0032
phase = 13/ 40   1.0001   0.0056  -0.0036
phase = 14/ 40   1.0002   0.0057  -0.0038
phase = 15/ 40   1.0002   0.0057  -0.0041
phase = 16/ 40   1.0002   0.0058  -0.0043
phase = 17/ 40   1.0003   0.0058  -0.0044
phase = 18/ 40   1.0003   0.0058  -0.0045
phase = 19/ 40   1.0003   0.0058  -0.0046
phase = 20/ 40   1.0003   0.0058  -0.0046
phase = 21/ 40   1.0003   0.0058  -0.0045
phase = 22/ 40   1.0003   0.0058  -0.0044
phase = 23/ 40   1.0002   0.0058  -0.0043
phase = 24/ 40   1.0002   0.0057  -0.0041
phase = 25/ 40   1.0002   0.0057  -0.0038
phase = 26/ 40   1.0001   0.0056  -0.0036
phase = 27/ 40   1.0001   0.0055  -0.0032
phase = 28/ 40   1.0000   0.0054  -0.0029
phase = 29/ 40   1.0000   0.0054  -0.0025
phase = 30/ 40   0.9999   0.0053  -0.0021
phase = 31/ 40   0.9999   0.0052  -0.0016
phase = 32/ 40   0.9999   0.0052  -0.0012
phase = 33/ 40   0.9998   0.0051  -0.0007
phase = 34/ 40   0.9998   0.0051  -0.0002
phase = 35/ 40   0.9998   0.0051   0.0003
phase = 36/ 40   0.9998   0.0051   0.0007
phase = 37/ 40   0.9998   0.0051   0.0012
phase = 38/ 40   0.9998   0.0052   0.0016
phase = 39/ 40   0.9998   0.0053   0.0021
#endif /* ] */

static int AnalysisWindow[240] = {        // crp06 .01 1 .1 iflat=8
  -250,  -297,  -345,  -395,  -448,  -504,  -562,  -622,  -684,  -749,
  -815,  -884,  -954, -1025, -1097, -1171, -1245, -1319, -1394, -1469,
 -1543, -1616, -1688, -1759, -1827, -1894, -1957, -2017, -2074, -2126,
 -2174, -2217, -2254, -2286, -2310, -2328, -2338, -2340, -2333, -2320,
 -2176, -2109, -2035, -1946, -1846, -1732, -1604, -1462, -1306, -1134,
  -947,  -744,  -526,  -291,   -40,   228,   513,   814,  1132,  1468,
  1820,  2189,  2575,  2977,  3396,  3831,  4281,  4748,  5228,  5724,
  6234,  6758,  7294,  7843,  8403,  8975,  9556, 10148, 10746, 11358,
 11908, 12498, 13098, 13699, 14304, 14908, 15514, 16116, 16718, 17314,
 17907, 18493, 19072, 19642, 20204, 20754, 21292, 21817, 22329, 22824,
 23304, 23766, 24210, 24635, 25039, 25422, 25783, 26121, 26435, 26725,
 26990, 27229, 27442, 27627, 27786, 27916, 28020, 28093, 28140, 28153,
 28153, 28140, 28093, 28020, 27916, 27786, 27627, 27442, 27229, 26990,
 26725, 26435, 26121, 25783, 25422, 25039, 24635, 24210, 23766, 23304,
 22824, 22329, 21817, 21292, 20754, 20204, 19642, 19072, 18493, 17907,
 17314, 16718, 16116, 15514, 14908, 14304, 13699, 13098, 12498, 11908,
 11358, 10746, 10148,  9556,  8975,  8403,  7843,  7294,  6758,  6234,
  5724,  5228,  4748,  4281,  3831,  3396,  2977,  2575,  2189,  1820,
  1468,  1132,   814,   513,   228,   -40,  -291,  -526,  -744,  -947,
 -1134, -1306, -1462, -1604, -1732, -1846, -1946, -2035, -2109, -2176,
 -2320, -2333, -2340, -2338, -2328, -2310, -2286, -2254, -2217, -2174,
 -2126, -2074, -2017, -1957, -1894, -1827, -1759, -1688, -1616, -1543,
 -1469, -1394, -1319, -1245, -1171, -1097, -1025,  -954,  -884,  -815,
  -749,  -684,  -622,  -562,  -504,  -448,  -395,  -345,  -297,  -250};


#if 0 /* [ */
 0.02000   1.00000   0.10000
iflat =  0
  0     0.00   -0.22   -0.23  -71.42   -0.23     -76.03  -76.03  -80.88  -80.88
  1            -0.21   -0.23  -70.21   -0.23     -85.17  -71.77  -84.32  -78.47
  2    -0.00   -0.21   -0.23  -67.85   -0.23     -86.07  -69.01  -90.25  -76.65
  3            -0.21   -0.23  -65.58   -0.23     -76.13  -66.99 -131.18  -75.19
  4    -0.01   -0.21   -0.23  -63.65   -0.23     -71.59  -65.43  -90.35  -73.99
  5            -0.20   -0.23  -62.05   -0.23     -68.62  -64.17  -84.29  -73.00
  6    -0.02   -0.20   -0.23  -60.70   -0.23     -66.42  -63.14  -80.77  -72.15
  7            -0.20   -0.24  -59.56   -0.24     -64.68  -62.28  -78.30  -71.44
  8    -0.04   -0.20   -0.24  -58.58   -0.24     -63.26  -61.56  -76.41  -70.83
  9            -0.19   -0.24  -57.74   -0.24     -62.06  -60.96  -74.90  -70.32
 10    -0.07   -0.19   -0.25  -57.02   -0.25     -61.05  -60.46  -73.65  -69.88
 11            -0.19   -0.25  -56.39   -0.25     -60.17  -60.04  -72.60  -69.52
 12    -0.09   -0.18   -0.26  -55.85   -0.26     -59.42  -59.71  -71.70  -69.23
 13            -0.18   -0.26  -55.38   -0.26     -58.76  -59.44  -70.93  -69.00
 14    -0.13   -0.17   -0.27  -54.98   -0.27     -58.20  -59.24  -70.27  -68.83
 15            -0.17   -0.28  -54.65   -0.28     -57.71  -59.11  -69.70  -68.71
 16    -0.17   -0.16   -0.28  -54.38   -0.28     -57.30  -59.03  -69.21  -68.65
 17            -0.16   -0.29  -54.16   -0.29     -56.95  -59.01  -68.80  -68.65
 18    -0.21   -0.15   -0.30  -54.00   -0.30     -56.67  -59.05  -68.45  -68.70
 19            -0.15   -0.31  -53.90   -0.31     -56.45  -59.14  -68.17  -68.80
 20    -0.26   -0.14   -0.32  -53.84   -0.32     -56.28  -59.29  -67.95  -68.96
 21            -0.13   -0.34  -53.84   -0.34     -56.18  -59.49  -67.78  -69.17
 22    -0.32   -0.13   -0.35  -53.89   -0.35     -56.13  -59.76  -67.67  -69.45
 23            -0.12   -0.36  -54.00   -0.36     -56.14  -60.09  -67.62  -69.79
 24    -0.38   -0.11   -0.38  -54.16   -0.38     -56.21  -60.49  -67.61  -70.19
 25            -0.10   -0.40  -54.38   -0.40     -56.35  -60.95  -67.67  -70.68
 26    -0.45   -0.09   -0.41  -54.67   -0.41     -56.55  -61.50  -67.77  -71.25
 27            -0.09   -0.43  -55.03   -0.43     -56.83  -62.14  -67.94  -71.91
 28    -0.52   -0.08   -0.46  -55.46   -0.46     -57.18  -62.88  -68.17  -72.69
 29            -0.07   -0.48  -55.98   -0.48     -57.62  -63.75  -68.46  -73.60
 30    -0.60   -0.06   -0.50  -56.60   -0.50     -58.16  -64.76  -68.82  -74.68
 31            -0.05   -0.53  -57.34   -0.53     -58.82  -65.96  -69.26  -75.97
 32    -0.69   -0.04   -0.56  -58.22   -0.56     -59.63  -67.39  -69.78  -77.55
 33            -0.03   -0.59  -59.27   -0.59     -60.60  -69.17  -70.41  -79.55
 34    -0.78   -0.02   -0.62  -60.54   -0.62     -61.80  -71.45  -71.15  -82.22
 35            -0.01   -0.65  -62.10   -0.65     -63.31  -74.60  -72.02  -86.19
 36    -0.88   -0.00   -0.69  -64.07   -0.69     -65.27  -79.64  -73.08  -93.82
 37             0.01   -0.72  -66.61   -0.72     -67.98  -93.01  -74.35 -101.50
 38    -0.98    0.02   -0.76  -69.84   -0.76     -72.18  -84.59  -75.93  -88.60
 39             0.03   -0.81  -72.61   -0.81     -81.22  -77.12  -77.96  -83.65
 40    -1.09    0.04   -0.85  -71.12   -0.85     -82.20  -73.23  -80.74  -80.55
 41             0.05   -0.90  -67.56   -0.90     -72.16  -70.60  -84.99  -78.32
 42    -1.21    0.06   -0.95  -64.55   -0.95     -67.56  -68.65  -94.01  -76.58
 43             0.07   -1.00  -62.18   -1.00     -64.53  -67.11  -95.35  -75.18
 44    -1.33    0.08   -1.05  -60.28   -1.05     -62.28  -65.86  -85.27  -74.02
 45             0.09   -1.11  -58.72   -1.11     -60.49  -64.82  -80.71  -73.04
 46    -1.46    0.10   -1.17  -57.41   -1.17     -59.02  -63.95  -77.74  -72.21
 47             0.11   -1.23  -56.29   -1.23     -57.78  -63.22  -75.55  -71.50
 48    -1.60    0.12   -1.30  -55.32   -1.30     -56.72  -62.59  -73.82  -70.89
 49             0.12   -1.37  -54.48   -1.37     -55.80  -62.06  -72.40  -70.37
 50    -1.75    0.13   -1.44  -53.74   -1.44     -55.00  -61.62  -71.22  -69.93
 51             0.14   -1.52  -53.10   -1.52     -54.30  -61.25  -70.21  -69.57
 52    -1.90    0.15   -1.59  -52.55   -1.59     -53.70  -60.96  -69.34  -69.26
 53             0.15   -1.68  -52.07   -1.68     -53.17  -60.72  -68.59  -69.02
 54    -2.07    0.16   -1.76  -51.66   -1.76     -52.72  -60.55  -67.93  -68.84
 55             0.17   -1.85  -51.31   -1.85     -52.34  -60.44  -67.37  -68.72
 56    -2.24    0.17   -1.94  -51.03   -1.94     -52.02  -60.38  -66.88  -68.65
 57             0.17   -2.04  -50.81   -2.04     -51.77  -60.38  -66.46  -68.63
 58    -2.42    0.18   -2.14  -50.65   -2.14     -51.59  -60.44  -66.11  -68.67
 59             0.18   -2.24  -50.55   -2.24     -51.46  -60.55  -65.82  -68.77
 60    -2.60    0.18   -2.35  -50.51   -2.35     -51.40  -60.72  -65.59  -68.92
 61             0.19   -2.46  -50.53   -2.46     -51.41  -60.95  -65.41  -69.13
 62    -2.80    0.19   -2.58  -50.62   -2.58     -51.48  -61.24  -65.29  -69.40
 63             0.19   -2.70  -50.78   -2.70     -51.63  -61.60  -65.22  -69.74
 64    -3.01    0.19   -2.82  -51.01   -2.82     -51.86  -62.04  -65.20  -70.16
 65             0.19   -2.95  -51.33   -2.95     -52.17  -62.55  -65.24  -70.66
 66    -3.23    0.19   -3.08  -51.74   -3.08     -52.59  -63.16  -65.34  -71.25
 67             0.19   -3.22  -52.27   -3.22     -53.12  -63.88  -65.50  -71.95
 68    -3.46    0.18   -3.36  -52.92   -3.36     -53.80  -64.73  -65.72  -72.78
 69             0.18   -3.51  -53.72   -3.51     -54.64  -65.73  -66.00  -73.77
 70    -3.70    0.18   -3.66  -54.73   -3.66     -55.70  -66.94  -66.37  -74.96
 71             0.17   -3.81  -55.99   -3.81     -57.07  -68.41  -66.81  -76.43
 72    -3.95    0.17   -3.97  -57.60   -3.97     -58.87  -70.27  -67.35  -78.29
 73             0.17   -4.14  -59.72   -4.14     -61.38  -72.72  -68.00  -80.76
 74    -4.22    0.16   -4.31  -62.63   -4.31     -65.28  -76.26  -68.78  -84.38
 75             0.15   -4.48  -66.53   -4.48     -73.38  -82.52  -69.72  -90.94
 76    -4.50    0.15   -4.66  -68.46   -4.66     -77.30 -105.43  -70.87 -107.34
 77             0.14   -4.85  -64.56   -4.85     -65.91  -81.28  -72.30  -88.52
 78    -4.79    0.13   -5.04  -60.52   -5.04     -60.97  -75.52  -74.14  -82.97
 79             0.12   -5.24  -57.47   -5.24     -57.74  -72.09  -76.62  -79.60
 80    -5.11    0.12   -5.44  -55.10   -5.44     -55.32  -69.65  -80.32  -77.18
 81             0.11   -5.65  -53.17   -5.65     -53.38  -67.77  -87.32  -75.30
 82    -5.43    0.10   -5.86  -51.56   -5.86     -51.77  -66.25  -98.29  -73.77
 83             0.09   -6.09  -50.18   -6.08     -50.39  -64.98  -83.21  -72.49
 84    -5.78    0.08   -6.31  -48.98   -6.31     -49.19  -63.91  -77.89  -71.39
 85             0.07   -6.54  -47.93   -6.54     -48.14  -62.98  -74.56  -70.45
 86    -6.14    0.06   -6.78  -46.99   -6.78     -47.21  -62.18  -72.13  -69.63
 87             0.05   -7.03  -46.16   -7.03     -46.38  -61.48  -70.22  -68.90
 88    -6.53    0.04   -7.28  -45.42   -7.28     -45.64  -60.87  -68.64  -68.26
 89             0.03   -7.54  -44.76   -7.54     -44.98  -60.33  -67.31  -67.70
 90    -6.94    0.02   -7.80  -44.18   -7.80     -44.40  -59.86  -66.16  -67.21
 91             0.01   -8.08  -43.66   -8.08     -43.88  -59.45  -65.15  -66.77
 92    -7.38   -0.00   -8.36  -43.21   -8.35     -43.44  -59.10  -64.26  -66.39
 93            -0.01   -8.64  -42.82   -8.64     -43.05  -58.80  -63.47  -66.06
 94    -7.85   -0.02   -8.94  -42.49   -8.94     -42.73  -58.55  -62.76  -65.78
 95            -0.03   -9.24  -42.23   -9.24     -42.47  -58.34  -62.13  -65.55
 96    -8.34   -0.04   -9.55  -42.03   -9.55     -42.27  -58.18  -61.56  -65.36
 97            -0.05   -9.87  -41.89   -9.87     -42.14  -58.06  -61.05  -65.20
 98    -8.88   -0.06  -10.19  -41.82  -10.19     -42.08  -57.98  -60.59  -65.09
 99            -0.07  -10.53  -41.82  -10.53     -42.09  -57.94  -60.19  -65.02
100    -9.45   -0.08  -10.87  -41.90  -10.87     -42.19  -57.93  -59.82  -64.99
101            -0.09  -11.23  -42.07  -11.22     -42.37  -57.97  -59.50  -65.00
102   -10.07   -0.09  -11.59  -42.33  -11.58     -42.67  -58.05  -59.23  -65.04
103            -0.10  -11.96  -42.71  -11.96     -43.09  -58.17  -58.98  -65.13
104   -10.74   -0.11  -12.34  -43.23  -12.34     -43.66  -58.34  -58.78  -65.26
105            -0.12  -12.73  -43.92  -12.73     -44.42  -58.54  -58.61  -65.43
106   -11.48   -0.13  -13.13  -44.82  -13.13     -45.44  -58.79  -58.48  -65.64
107            -0.13  -13.55  -46.01  -13.54     -46.82  -59.09  -58.39  -65.90
108   -12.29   -0.14  -13.97  -47.58  -13.97     -48.76  -59.44  -58.33  -66.21
109            -0.15  -14.40  -49.68  -14.40     -51.70  -59.85  -58.30  -66.57
110   -13.19   -0.15  -14.85  -52.35  -14.85     -57.00  -60.31  -58.31  -66.99
111            -0.16  -15.31  -54.39  -15.31     -82.22  -60.85  -58.35  -67.48
112   -14.20   -0.16  -15.78  -52.67  -15.78     -57.00  -61.46  -58.43  -68.03
113            -0.17  -16.27  -49.01  -16.27     -50.27  -62.15  -58.55  -68.67
114   -15.34   -0.17  -16.77  -45.72  -16.77     -46.22  -62.95  -58.71  -69.40
115            -0.18  -17.29  -43.01  -17.27     -43.25  -63.88  -58.91  -70.24
116   -16.67   -0.18  -17.82  -40.74  -17.79     -40.86  -64.95  -59.15  -71.22
117            -0.19  -18.36  -38.78  -18.32     -38.85  -66.21  -59.43  -72.38
118   -18.24   -0.19  -18.93  -37.05  -18.86     -37.09  -67.74  -59.77  -73.75
119            -0.19  -19.51  -35.50  -19.40     -35.53  -69.64  -60.16  -75.44
120   -20.17   -0.20  -20.11  -34.10  -19.94     -34.11  -72.11  -60.60  -77.59
121            -0.20  -20.73  -32.81  -20.47     -32.82  -75.62  -61.12  -80.51
122   -22.67   -0.20  -21.37  -31.61  -20.98     -31.62  -81.68  -61.71  -85.01
123            -0.20  -22.04  -30.49  -21.46     -30.50 -121.78  -62.38  -95.05
124   -26.18   -0.21  -22.73  -29.44  -21.89     -29.45  -81.54  -63.16  -93.59
125            -0.21  -23.45  -28.45  -22.25     -28.46  -75.59  -64.05  -84.51
126   -32.20   -0.21  -24.19  -27.52  -22.53     -27.52  -72.13  -65.10  -80.19
127            -0.21  -24.97  -26.63  -22.71     -26.63  -69.70  -66.34  -77.35
128  -324.26   -0.22  -25.78  -25.78  -22.77     -25.78  -67.83  -67.83  -75.23
in band to aliased ratio =   36.66418
in band to distant aliased ratio =   68.98246
overlapped passband max deviation from flat =    0.40542
in band to weighted aliased ratio =   53.69209
#endif /* ] */

static int LdspkrAnalysisWindow[240] = {  // crp08 .02 1 .1 iflat=0
   111,    95,    76,    54,    28,    -1,   -33,   -69,  -109,  -152,
  -198,  -248,  -301,  -358,  -418,  -480,  -546,  -615,  -686,  -760,
  -836,  -913,  -992, -1072, -1153, -1234, -1315, -1396, -1476, -1554,
 -1630, -1704, -1775, -1842, -1905, -1963, -2016, -2063, -2103, -2135,
 -2206, -2213, -2210, -2196, -2170, -2130, -2077, -2010, -1928, -1830,
 -1716, -1586, -1438, -1273, -1089,  -887,  -665,  -424,  -164,   117,
   417,   738,  1079,  1440,  1822,  2224,  2646,  3087,  3549,  4029,
  4528,  5045,  5580,  6132,  6700,  7283,  7882,  8494,  9120,  9757,
 10427, 11079, 11739, 12407, 13079, 13756, 14436, 15117, 15798, 16478,
 17154, 17827, 18493, 19152, 19803, 20442, 21071, 21685, 22285, 22869,
 23436, 23983, 24510, 25016, 25498, 25957, 26391, 26798, 27178, 27530,
 27852, 28145, 28407, 28637, 28835, 29001, 29134, 29233, 29299, 29331,
 29331, 29299, 29233, 29134, 29001, 28835, 28637, 28407, 28145, 27852,
 27530, 27178, 26798, 26391, 25957, 25498, 25016, 24510, 23983, 23436,
 22869, 22285, 21685, 21071, 20442, 19803, 19152, 18493, 17827, 17154,
 16478, 15798, 15117, 14436, 13756, 13079, 12407, 11739, 11079, 10427,
  9757,  9120,  8494,  7882,  7283,  6700,  6132,  5580,  5045,  4528,
  4029,  3549,  3087,  2646,  2224,  1822,  1440,  1079,   738,   417,
   117,  -164,  -424,  -665,  -887, -1089, -1273, -1438, -1586, -1716,
 -1830, -1928, -2010, -2077, -2130, -2170, -2196, -2210, -2213, -2206,
 -2135, -2103, -2063, -2016, -1963, -1905, -1842, -1775, -1704, -1630,
 -1554, -1476, -1396, -1315, -1234, -1153, -1072,  -992,  -913,  -836,
  -760,  -686,  -615,  -546,  -480,  -418,  -358,  -301,  -248,  -198,
  -152,  -109,   -69,   -33,    -1,    28,    54,    76,    95,   111};

    int i,j;
    icomplex TwoFrameMicFFTArray[2][M+1];
    icomplex TwoFrameLdspkrFFTArray[2][M+1];

    int MicDB[NUM_BANDS_PROCESSED];
    int LdspkrDB[NUM_BANDS_PROCESSED];
    int ResDB[NUM_BANDS_PROCESSED];

#if 0 /* [ */
   int max;
   max = 0;
   for (i=0; i<TWOM; i++) {
      if (abs(mic[i]) > max) max = abs(mic[i]);
   }
   printf("mic max = %10.0f ",max);

   max = 0;
   for (i=0; i<TWOM; i++) {
      if (abs(ldspkr[i]) > max) max = abs(ldspkr[i]);
   }
   printf("ldspkr max = %10.0f ",max);

   getchar();
#endif /* ] */


#if (0)
/* DEBUG!!!!! Feb 1, 2005 Overwrite mic signal with loudspeaker signal. */
    for (i=0; i<TWOM; i++)
       mic[i] =  ldspkr[i] >> 2;
#endif


/* DEBUG!!!!This is only for monitoring time domain ERL.
   This can be eliminated in final release. */
    static short micdl[5*M+TWOM+MAX_EXTRA_MIC_DELAY];  // For monitoring time domain ERL
    for (i=0; i<5*M+EXTRA_MIC_DELAY; i++)
       micdl[i] = micdl[i+TWOM];
    for (i=0; i<TWOM; i++)
       micdl[i+5*M+EXTRA_MIC_DELAY] =  mic[i];


// If needed, delay the mic signal.
// DEBUG!!! With memory to spare, let buffer creep so that the
// copying only takes place rarely.
    for (i=0; i < EXTRA_MIC_DELAY; i++)
      DelayedMic[i] = DelayedMic[i+TWOM];
    for (i=0; i<TWOM; i++)
       DelayedMic[EXTRA_MIC_DELAY+i] = mic[i];

//DEBUG!!!
#define TEST_SINUSOID_DEBUG 0
#if (TEST_SINUSOID_DEBUG)
    for (i=0; i<TWOM; i++) {
       static double Angle;
       double dTemp0;
       dTemp0 = 1000.0 * sin(6.2831853*Angle);
       Angle += 200./8000.;
       if (dTemp0 >= 0.0) j = (int) (dTemp0 + 0.5);
       else j = (int) (dTemp0 - 0.5);
       DelayedMic[EXTRA_MIC_DELAY+i] = j;
    }
#endif

// If needed, delay the loudspeaker signal.
// This also copies the samples into an array of 32-bit integers.
    for (i=0; i < EXTRA_LDSPKR_DELAY; i++)
      DelayedLdspkr[i] = DelayedLdspkr[i+TWOM];
    for (i=0; i<TWOM; i++)
       DelayedLdspkr[i+EXTRA_LDSPKR_DELAY] = ldspkr[i];


#if (VX1_PC0 == 1)
    TwoFrameFilterBankAnalysis(TwoFrameMicFFTArray,
       DelayedMic,TwoFrameMicAnalysisDL,AnalysisWindow, 0); // mic data

    TwoFrameFilterBankAnalysis(TwoFrameLdspkrFFTArray,
       DelayedLdspkr,TwoFrameLdspkrAnalysisDL,LdspkrAnalysisWindow, 1); // spkr data
#else
    TwoFrameFilterBankAnalysis(TwoFrameMicFFTArray,
       DelayedMic,TwoFrameMicAnalysisDL,AnalysisWindow); // mic data

    TwoFrameFilterBankAnalysis(TwoFrameLdspkrFFTArray,
       DelayedLdspkr,TwoFrameLdspkrAnalysisDL,LdspkrAnalysisWindow); // spkr data
#endif



//DEBUG!!!
#if (TEST_SINUSOID_DEBUG)
    for (FrameNum = 0; FrameNum < 2; FrameNum++) {
       for (i=0; i<6; i++) {
          printf("%5d,%5d  ",TwoFrameMicFFTArray[FrameNum][i].r,TwoFrameMicFFTArray[FrameNum][i].i);
       }
       printf("\n");
    }
    getchar();
#endif
// END OF DEBUG!!!!!!

/* ************************************************************************** */
/* ************** Functions that operate on the bands *********************** */
/* ************************************************************************** */
#define MIC_GAIN_SHIFT 3

//DEBUG!!!! To minimize cache hits, we should
    DoubletalkIndicator = DoubletalkDetection(TwoFrameMicFFTArray,
       TwoFrameLdspkrFFTArray,
       MicDB,
       LdspkrDB);

    ComputeLoudspeakerFade();

/* Main echo canceller loop for subbands. */
    SubbandECLoop(TwoFrameMicFFTArray,
       TwoFrameLdspkrFFTArray,
       MicDB,
       LdspkrDB,
       ResDB);


if (iEchoSup)
    EchoSuppress(TwoFrameMicFFTArray,
              LdspkrDB,
              ResDB,
              MicDB,
              DoubletalkIndicator);



/* Zero the bands that aren't processed. */
    for (i=0; i < LOW_BAND; i++) {
       TwoFrameMicFFTArray[0][i].r = 0;
       TwoFrameMicFFTArray[0][i].i = 0;
       TwoFrameMicFFTArray[1][i].r = 0;
       TwoFrameMicFFTArray[1][i].i = 0;
    }
    for (i=HIGH_BAND+1; i <= M; i++) {
       TwoFrameMicFFTArray[0][i].r = 0;
       TwoFrameMicFFTArray[0][i].i = 0;
       TwoFrameMicFFTArray[1][i].r = 0;
       TwoFrameMicFFTArray[1][i].i = 0;
    }


// Synthesis of echo cancelled mic signal.
    {

// Every other frame (M samples) invert every other complex band.
#define MODULATION 1    // Necessary, if we do any in-band filtering
#if (MODULATION)
    for (i=1; i < M; i += 2) {
       TwoFrameMicFFTArray[1][i].r  = -TwoFrameMicFFTArray[1][i].r;
       TwoFrameMicFFTArray[1][i].i  = -TwoFrameMicFFTArray[1][i].i;
    }
#endif

   int FrameNum;
    for (FrameNum = 0; FrameNum < 2; FrameNum++) {


       FFT80CtoR(&TwoFrameMicFFTArray[FrameNum][0]);
       int FFTOut[TWOM];

#define FBSHIFT0 1
#if (FBSHIFT0 == 0)
       for (i=0; i < M; i++) {   // Scale down by 1/TWOM
          FFTOut[2*i]  = (TwoFrameMicFFTArray[FrameNum][i].r * 410 + 16384) >> 15;
          FFTOut[2*i+1]  = (TwoFrameMicFFTArray[FrameNum][i].i * 410 + 16384) >> 15;
       }
#elif (FBSHIFT0 == 1)
       for (i=0; i < M; i++) {   // Scale down by 2/TWOM
          FFTOut[2*i]  = (TwoFrameMicFFTArray[FrameNum][i].r * 819 + 16384) >> 15;
          FFTOut[2*i+1]  = (TwoFrameMicFFTArray[FrameNum][i].i * 819 + 16384) >> 15;
       }
#elif (FBSHIFT0 == 3)
       for (i=0; i < M; i++) {   // Scale down by 8/TWOM
          FFTOut[2*i]  = (TwoFrameMicFFTArray[FrameNum][i].r * 3277 + 16384) >> 15;
          FFTOut[2*i+1]  = (TwoFrameMicFFTArray[FrameNum][i].i * 3277 + 16384) >> 15;
       }
#endif

#define ReconstructionWindow AnalysisWindow
       for (i=0; i<2*M; i++)
          SynthesisDL[i] = SynthesisDL[i+M] + ReconstructionWindow[i]*FFTOut[i];
       j = 0;
       for (i=2*M; i<4*M; i++)
          SynthesisDL[i] = SynthesisDL[i+M] + ReconstructionWindow[i]*FFTOut[j++];
       j = 0;
       for (i=4*M; i<5*M; i++)
          SynthesisDL[i] = SynthesisDL[i+M] + ReconstructionWindow[i]*FFTOut[j++];
       for (i=5*M; i<6*M; i++)
          SynthesisDL[i] = ReconstructionWindow[i]*FFTOut[j++];


#if (VX1_PC0 == 1)
	   if (FrameNum == 0) {
		   for (i=0; i<M; i++)
			   mic[i] = (SynthesisDL[i] + 16384) >> 15;
	   }
	   else {
		   for (i=0; i<M; i++)
			   mic[i+M] = (SynthesisDL[i] + 16384) >> 15;
	   }


#else    // #else for VX1_PC0
       if (FrameNum == 0) {
         for (i=0; i<M; i++)
            fbecmic[i] = (SynthesisDL[i] + 16384) >> 15;
       }
       else {
         for (i=0; i<M; i++)
            fbecmic[i+M] = (SynthesisDL[i] + 16384) >> 15;
       }

#endif      // #endif for VX1_PC0

    }
    }


#if (VX1_PC0 == 1)

    if (Frame10msCount%1000 == 999) {
		EXTRA_LDSPKR_DELAY += 160;
		if (EXTRA_LDSPKR_DELAY > MAX_EXTRA_LDSPKR_DELAY) EXTRA_LDSPKR_DELAY = 0;
		osPrintf("------EXTRA_LDSPKR_DELAY = %4d ------\n",EXTRA_LDSPKR_DELAY);
	}


// for DEBUG!!!!! only monitoring of ERL


	#if (0)
    if (Frame10msCount%600 == 0)
    {
        int Band;
		int Temp0;
		int Temp1;

//        for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
        for (Band=3; Band < 13; Band++)
        {
            osPrintf("%2d: ",Band);
            for (j=ECNumTaps[Band]-1; j >= 0; j--) {
                Temp0 = EchoCancellerCoef[Band][j].r >> 4;
                Temp1 = EchoCancellerCoef[Band][j].i >> 4;
                osPrintf("%2d ",Get1000log10(Temp0*Temp0+Temp1*Temp1)/100);
            }
            osPrintf("\n");
        }
//        getchar();
    }
	#endif


#if 1  /* 1 */

    int Temp0, Temp1;
    static int TDBigSumMic;
    static int TDBigSumRes;

    for (i=0; i<TWOM; i++)
    {
      TDSummic += abs(micdl[i]);

      TDSumres += abs(mic[i]);
    }
    if (Frame10msCount%100 == 99)
    {    // print out ERL every 1s
 //      Temp0 = Get1000log10(TDSummic) - Get1000log10(TDSumres);
 //      osPrintf("%2d ",Temp0/50);  //ERL printout for VxWorks
 //      if (Frame10msCount%200 == 199) osPrintf("\n");

       TDBigSumMic += TDSummic;
       TDBigSumRes += TDSumres;

       TDSummic = 0;
       TDSumres = 0;

       if ( 0/*Frame10msCount%200 == 199*/) {
           Temp0 = Get1000log10(TDBigSumMic) - Get1000log10(TDBigSumRes);
           osPrintf("   %3d  ",Temp0/50);  //ERL printout for VxWorks
           osPrintf("(%3d)",Get1000log10(TDBigSumMic)/50);
           osPrintf("\n");  //ERL printout for VxWorks
           TDBigSumMic = 0;
           TDBigSumRes = 0;
       }



#define PrintUpdateDecisionFlag 0

#if (PrintUpdateDecisionFlag) /* 5 */
	#if 0 /* [ */
	   osPrintf("%6d ",DoubletalkIndicator);
	   if ((Frame10msCount & 0xf) == 0)
	   {
		   int Band;
		   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
			   osPrintf("%5d %5d   %5d %5d ",LdspkrDLPow[Band],ThresholdLdspkrDLPow[Band],
				   (LdspkrDB[Band] + LdspkrMicGain[Band] + 10240),MicDB[Band]);
			   if (Band%2 == 1) osPrintf("\n");
		   }
		   osPrintf("\n");
	   }
	#endif /* ] */
       if (Frame10msCount%200 == 199)
       {
           int Band;
	#if 0 /* [ */
			   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
				   if (Band%20 == 0) osPrintf("\n");
				   osPrintf("%3d ",LdspkrMicGain[Band]/1000);
			   }
			   osPrintf("\n");
	#endif /* ] */

           for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
               if (Band == 20) osPrintf("\n");
               osPrintf("%3d ",DebugUpdateCount[Band]);
           }
           osPrintf("\n");

           for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
               DebugUpdateCount[Band] = 0;
           }
       }
#endif /* 5 */




#if 1 /* 2 */
//       osPrintf(" %6d ", Frame10msCount);
       osPrintf(" %6d ", DoubletalkIndicator);

       static int DisplayBand = 10;
       osPrintf("%2d",DisplayBand);

	#if COMPUTE_BAND_ERL /* 3 */
		   Temp0 = Get1000log10(SumFBMicPow[DisplayBand]) -
			  Get1000log10(SumFBResPow[DisplayBand]);
		   osPrintf(" %2d",Temp0/100);
	#endif /* 3 */

       Temp0 = Get1000log10(LdspkrDLPow[DisplayBand]);
       osPrintf(" %2d",Temp0/100);

       Temp0 = Get1000log10(ThresholdLdspkrDLPow[DisplayBand]);
       osPrintf(" %2d",Temp0/100);


	#if COMPUTE_BAND_ERL /* 4 */
	   Temp0 = Get1000log10(SumFBMicPow[DisplayBand]);
	   osPrintf(" %2d  ",Temp0/100);
	#endif /* 4 */

	   for (j=ECNumTaps[DisplayBand]-1; j >= 0; j--) {
		   Temp0 = EchoCancellerCoef[DisplayBand][j].r >> 4;
		   Temp1 = EchoCancellerCoef[DisplayBand][j].i >> 4;
		   osPrintf("%2d ",Get1000log10(Temp0*Temp0+Temp1*Temp1)/100);
	   }


 	   osPrintf("\n");


/*
       for (j=0; j<MAX_NUM_TAPS; j++) {
          osPrintf("%6d,%6d  ",EchoCancellerCoef[DisplayBand][j].r,
             EchoCancellerCoef[DisplayBand][j].i);
          if (j%7 == 6) osPrintf("\n");
       }
       osPrintf("\n");
*/
//       DisplayBand++;
       if (DisplayBand >= NUM_BANDS_PROCESSED) DisplayBand = 0;
#endif /* 2 */

    }
#endif /* 1 */


#if (0)
    if (Frame10msCount%100 == 99)
    {    // print out max EC coeff every second
    }
#endif


#ifdef _VXWORKS /* [ */

    after9 = *pOsTC;
    after9 = ((after9 - before) * 271) >>10; // microseconds

    // osPrintf(" r=%4d ", after9);

#endif /* _VXWORKS ] */
#endif




#if (VX1_PC0 == 0)
    int Temp0,Temp1;

   dTDSummic = 0.0;
   dTDSumres = 0.0;
   {
// if (Frame10msCount%10 == 0) {
//   if (Frame10msCount%(TotalSamples/(TWOM*10)) == 0) {
      dTDSummic = 0.0;
      dTDSumres = 0.0;
   }
   for (i=0; i<TWOM; i++) {
      dTDSummic += micdl[i]*micdl[i];
      dTDSumres += fbecmic[i]*fbecmic[i];
   }

/* printf("%4d: fb td ERL = %6.2f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   printf("  %2.0f %2.0f\n",10.0*log10(dTDSummic),10.0*log10(dTDSumres)); */

 {
   int Band;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
   {
      if (abs(LdspkrDLPow[Band]) > MaxLdspkrDLPow[Band]) MaxLdspkrDLPow[Band] = abs(LdspkrDLPow[Band]);
#if COMPUTE_BAND_ERL
      if (abs(SumFBMicPow[Band]) > MaxSumFBMicPow[Band]) MaxSumFBMicPow[Band] = abs(SumFBMicPow[Band]);
      if (abs(SumFBResPow[Band]) > MaxSumFBResPow[Band]) MaxSumFBResPow[Band] = abs(SumFBResPow[Band]);
#endif
   }
 }

#if (0)        // For testing filterbank reconstruction (without echo cancellation)
    double dSignalPower, dErrorPower;
    double db;
    dSignalPower = 0;
    dErrorPower = 0;
    for (i=0; i<TWOM; i++) {
       dSignalPower += micdl[i]*micdl[i];
       dErrorPower += (micdl[i]-fbecmic[i])*(micdl[i]-fbecmic[i]);
    }
//  printf("filterbank SNR = %6.3f dB\n",10.0*log10(dSignalPower/dErrorPower));
    printf("%2.0f",db = 10.0*log10(dSignalPower/dErrorPower));
    if (db > 50.0) printf(" ");
    else printf("*");

/*  printf("%2.0f",db = 10.0*log10(dErrorPower));
    if (db < 35.0) printf(" ");
    else printf("#"); */

/*  int MaxVal = 0;
    int MaxIndex = -1;
    for (i=0; i<TWOM; i++) {
       if (abs(micdl[i]-fbecmic[i]) > MaxVal)
       {
          MaxVal = abs(micdl[i]-fbecmic[i]);
          MaxIndex = i;
       }
    }
    printf("%2d %2d ",MaxVal, MaxIndex); */


//    if (Frame10msCount%10 == 9) printf("\n");
    if (1) printf("\n");

#endif

#if (0)
    for (i=0; i<TWOM; i++) {
       printf("%6d %6d %2d  ",micdl[i],fbecmic[i],micdl[i]-fbecmic[i]);
       if (i%4 == 3) printf("\n");
    }
    getchar();
#endif


/*  printf("fb ec output samples:\n");
    for (i=0; i<TWOM; i++) {
       printf("%8.2f ",fbecmic[i]);
       if (i%8 == 7) printf("\n");
    } */

//    if (Frame10msCount%10 == 9) {
    if (1) {

#if (0)
       for (i=0; i<=M; i++) {
          printf("%7.0f ",LdspkrDLPow[i]/65536.0);
          if (i%6 == 5) printf("\n");
       }
       printf("\n");
#endif

#if (1)
   printf("%4d: %2.0f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   fprintf(fp_stats,"%4d: %2.0f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   printf("  %3.0f %2.0f ",10.0*log10(dTDSummic),10.0*log10(dTDSumres));
   fprintf(fp_stats,"  %3.0f %2.0f ",10.0*log10(dTDSummic),10.0*log10(dTDSumres));

   printf("%5d", DoubletalkIndicator);
   fprintf(fp_stats,"%5d", DoubletalkIndicator);


//   if (iEchoSup == 0) {
       printf("\n");
       fprintf(fp_stats,"\n");
//   }

#if 0
  {
    int Band;
    for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
       printf("%2d: ",Band);
       fprintf(fp_stats,"%2d: ",Band);

#if COMPUTE_BAND_ERL
//       printf("%5.2f ",10.0*log10((double)SumFBMicPow[Band]/(double)SumFBResPow[Band]));
       {
          int powdbtimes100,errdbtimes100;
          powdbtimes100 = Get1000log10(SumFBMicPow[Band]);
          errdbtimes100 = Get1000log10(SumFBResPow[Band]);
          Temp0 = (powdbtimes100 - errdbtimes100)/100;
          printf("%2d   ",Temp0);
          fprintf(fp_stats,"%2d   ",Temp0);
       }
#endif
       printf("%4d ",DebugUpdateCount[Band]);
       fprintf(fp_stats,"%4d ",DebugUpdateCount[Band]);

       Temp0 = Get1000log10(LdspkrDLPow[Band]);
       printf("%2d ",Temp0/100);
       fprintf(fp_stats,"%2d ",Temp0/100);

       Temp0 = Get1000log10(ThresholdLdspkrDLPow[Band]);
       printf("%2d   ",Temp0/100);
       fprintf(fp_stats,"%2d   ",Temp0/100);

       printf("%2d ",MicDB[Band]/1000);
       fprintf(fp_stats,"%2d ",MicDB[Band]/1000);

#if DEBUG_ECHO_ESTIMATE_POW
       printf("%2d ",Get1000log10(EchoEstimatePow[Band])/100);
       fprintf(fp_stats,"%2d ",Get1000log10(EchoEstimatePow[Band])/100);
#endif

       printf("%2d   ",ResDB[Band]/1000);
       fprintf(fp_stats,"%2d   ",ResDB[Band]/1000);

       printf("%2d ",LdspkrDB[Band]/1000);
       fprintf(fp_stats,"%2d ",LdspkrDB[Band]/1000);

       printf("%3d   ",LdspkrMicGain[Band]/1000);
       fprintf(fp_stats,"%3d   ",LdspkrMicGain[Band]/1000);

#if DEBUG_ECHO_ESTIMATE_POW
       printf("%3d   ",(LdspkrDB[Band]+LdspkrMicGain[Band])/1000 - Get1000log10(EchoEstimatePow[Band])/100);
       fprintf(fp_stats,"%3d   ",(LdspkrDB[Band]+LdspkrMicGain[Band])/1000 - Get1000log10(EchoEstimatePow[Band])/100);
#endif

#if COMPUTE_BAND_ERL
/*       Temp0 = Get1000log10(SumFBMicPow[Band]);
       printf("%2d  ",Temp0/100); */

       SumFBMicPow[Band] = 0;
       SumFBResPow[Band] = 0;
#endif
       printf("%2d\n",EchoSupDB >> 10);
       fprintf(fp_stats,"%2d\n",EchoSupDB >> 10);

/*     for (j=0; j<MAX_NUM_TAPS; j++) {
          printf("%6d %6d  ",EchoCancellerCoef[Band][j].r, EchoCancellerCoef[Band][j].i);
          if (j%5 == 4) printf("\n");
       } */
    }
  }
#endif
//  getchar();
#endif

#if (1)
    if (Frame10msCount%100 == 99)
    {
        int Band;
        for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
        {
            printf("%2d: ",Band);
            fprintf(fp_stats,"%2d: ",Band);
            for (j=ECNumTaps[Band]-1; j >= 0; j--) {
                Temp0 = EchoCancellerCoef[Band][j].r >> 4;
                Temp1 = EchoCancellerCoef[Band][j].i >> 4;
                printf("%2d ",Get1000log10(Temp0*Temp0+Temp1*Temp1)/100);
                fprintf(fp_stats,"%2d ",Get1000log10(Temp0*Temp0+Temp1*Temp1)/100);
            }
            printf("\n");
            fprintf(fp_stats,"\n");
        }
//        getchar();
    }
#endif

   }

#endif      // #endif for VX1_PC0

   Frame10msCount++;

}

#endif  // DEBUG!!!!! Feb 1, 2005


// Window, overlap and add for two frames. Then DFT each of the 2 frames.

#if (VX1_PC0 == 1)
void FilterBank::TwoFrameFilterBankAnalysis(
                                icomplex outFFTArray[][M+1],
                                int Input[],
                                int AnalysisDL[],
                                int AnalysisWindow[],
                                int iMicOrSpkrFlag)

#else    // #else for VX1_PC0

void TwoFrameFilterBankAnalysis(icomplex outFFTArray[][M+1],
           int Input[],
           int AnalysisDL[],
           int AnalysisWindow[])
#endif      // #endif for VX1_PC0

{
   int i;
   union IntComplexArray {
      icomplex FFTArray[2][M+1];
      int FFTRealIn[2][2*(M+1)];
   } WorkArray;

// Sequence input delay line for an entire 10 ms frame.
// DEBUG!!! With memory to spare, let the buffer creep so that the
// copying only takes place rarely.
/* for (i=0; i<(WINDOWSIZE-M); i++)
      AnalysisDL[i] = AnalysisDL[i+TWOM]; */
   memcpy(AnalysisDL,&AnalysisDL[TWOM], sizeof (int) * (WINDOWSIZE-M));
/* for (i=0; i < TWOM; i++)
      AnalysisDL[i+(WINDOWSIZE-M)] = Input[i]; */
   memcpy(&AnalysisDL[WINDOWSIZE-M],Input, sizeof (int) * TWOM);


   int Temp0, Temp1, Temp2;
   for (i=0; i < TWOM; i++)
   {
      Temp0 = AnalysisWindow[i];
      Temp1 = AnalysisWindow[i+TWOM];
      Temp2 = AnalysisWindow[i+2*TWOM];

// First frame: window, overlap and add.
      WorkArray.FFTRealIn[0][i] = (Temp0*AnalysisDL[i] +
         Temp1*AnalysisDL[i+TWOM] +
         Temp2*AnalysisDL[i+2*TWOM] + 16384) >> 15;

// Second frame: window, overlap and add.
      WorkArray.FFTRealIn[1][i] = (Temp0*AnalysisDL[i+M] +
         Temp1*AnalysisDL[i+TWOM+M] +
         Temp2*AnalysisDL[i+2*TWOM+M] + 16384) >> 15;
   }

/* For each frame do a TWOM-point real to M (M+1) complex DFT. */
   FFT80RtoC(&WorkArray.FFTArray[0][0]);
   FFT80RtoC(&WorkArray.FFTArray[1][0]);

/* Every other frame (M samples) invert every other complex band.
This corresponds to modulation by e**(-j*2*PI*i*FrameNum*M/TWOM).
We can interpret this as making continuous the DFT modulation carrier waves. */
#if (MODULATION)
   for (i=1; i < M; i += 2) {
      WorkArray.FFTArray[1][i].r = -WorkArray.FFTArray[1][i].r;
      WorkArray.FFTArray[1][i].i = -WorkArray.FFTArray[1][i].i;
   }
#endif

// Make sure imaginary parts of bands 0 and M are zeroed.
   WorkArray.FFTArray[0][0].i = 0;
   WorkArray.FFTArray[0][M].i = 0;
   WorkArray.FFTArray[1][0].i = 0;
   WorkArray.FFTArray[1][M].i = 0;

   int j;
   for (j = 0; j < 2; j++) {
      for (i = 0; i <= M; i++) {
#if (FBSHIFT0 == 0)
         outFFTArray[j][i].r = WorkArray.FFTArray[j][i].r;
         outFFTArray[j][i].i = WorkArray.FFTArray[j][i].i;
#elif (FBSHIFT0 == 1)
         outFFTArray[j][i].r = (WorkArray.FFTArray[j][i].r) >> 1;
         outFFTArray[j][i].i = (WorkArray.FFTArray[j][i].i) >> 1;
#elif (FBSHIFT0 == 3)
         outFFTArray[j][i].r = (WorkArray.FFTArray[j][i].r + 4) >> 3;
         outFFTArray[j][i].i = (WorkArray.FFTArray[j][i].i + 4) >> 3;
#endif
      }
   }

}


/* ************************************************************************** */
/* *********************** Doubletalk Detection ***************************** */
/* ************************************************************************** */

#if (VX1_PC0 == 1)
int FilterBank::DoubletalkDetection(icomplex TwoFrameMicFFTArray[][M+1],
                  icomplex TwoFrameLdspkrFFTArray[][M+1],
                  int MicDB[],
                  int LdspkrDB[])

#else    // #else for VX1_PC0

int DoubletalkDetection(icomplex TwoFrameMicFFTArray[][M+1],
                  icomplex TwoFrameLdspkrFFTArray[][M+1],
                  int MicDB[],
                  int LdspkrDB[])
#endif  // #endif for VX1_PC0
{
   int Band;
   int Temp0,Temp1,Temp2;

   /* Compute mic power and weighted loudspeaker power for each band, and
   update leaky average of raw mic power. */
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {

   /* Compute this frame's mic power for doubletalk detection and leaky integrated
      mic power for computing ERL in band. */

#if COMPUTE_BAND_ERL
      SumFBMicPow[Band] -= (SumFBMicPow[Band] >> 5);
#endif

#if USE_IMAGSQ
      Temp2 = imagsq(&(TwoFrameMicFFTArray[0][Band + LOW_BAND]), MIC_GAIN_SHIFT);
      Temp2 += imagsq(&(TwoFrameMicFFTArray[1][Band + LOW_BAND]), MIC_GAIN_SHIFT);
#else
      Temp0 = TwoFrameMicFFTArray[0][Band + LOW_BAND].r >> MIC_GAIN_SHIFT;
      Temp1 = TwoFrameMicFFTArray[0][Band + LOW_BAND].i >> MIC_GAIN_SHIFT;
      Temp2 = (Temp0*Temp0 + Temp1*Temp1);
      Temp0 = TwoFrameMicFFTArray[1][Band + LOW_BAND].r >> MIC_GAIN_SHIFT;
      Temp1 = TwoFrameMicFFTArray[1][Band + LOW_BAND].i >> MIC_GAIN_SHIFT;
      Temp2 += (Temp0*Temp0 + Temp1*Temp1);
#endif


      if (Temp2 < 0) Temp2 = 0x7fffffff;
      MicPow[Band] = Temp2;
      MicDB[Band] = 10*Get1000log10(Temp2);

#if (VX1_PC0 == 0)
      if ((MicDB[Band] == 0) && (Temp2 != 0)) {       // DEBUG!!!!!!!
         printf("Band=%2d MicDB[]=%8x Temp2=%8x\n",Band,MicDB[Band],Temp2);
         getchar();        // DEBUG!!!!!!!
      }
#endif

#if COMPUTE_BAND_ERL
      SumFBMicPow[Band] += (Temp2 + 2) >> 2;
#endif

#if (1)
// WeightedLdspkrDLPow[Band] was updated last frame in the main echo canceller loop.
// Fine tuning WeightedLdspkrDLPow[Band] would use the newest samples.
// This doesn't include the filtering associated with POLE_CONST.
#if USE_IMAGSQ
       Temp2 = (imagsq(&(TwoFrameLdspkrFFTArray[0][Band + LOW_BAND]), 4)) >> 6;
       Temp2 += (imagsq(&(TwoFrameLdspkrFFTArray[1][Band + LOW_BAND]), 4)) >> 6;

#else
       Temp0 = TwoFrameLdspkrFFTArray[0][Band + LOW_BAND].r >> 4;
       Temp1 = TwoFrameLdspkrFFTArray[0][Band + LOW_BAND].i >> 4;
       Temp2 = (Temp0*Temp0 + Temp1*Temp1) >> 6;
       Temp0 = TwoFrameLdspkrFFTArray[1][Band + LOW_BAND].r >> 4;
       Temp1 = TwoFrameLdspkrFFTArray[1][Band + LOW_BAND].i >> 4;
       Temp2 += (Temp0*Temp0 + Temp1*Temp1) >> 6;
#endif
      WeightedLdspkrDLPow[Band] += Temp2;
#endif

      Temp0 = 8000 + 10*Get1000log10(WeightedLdspkrDLPow[Band]);
      //printf("%3d ",(Temp0-LdspkrDB[Band])/1000);
      LdspkrDB[Band] = Temp0;
/* LdspkrDB[Band] is used directly below for doubletalk detection. It is also used later this
frame for echo suppression. */
   }


/* LdspkrMicGain tends to underestimate the gain (overestimate the loss) by
about 3 or 4 dB. By adding 9000 to LdspkrMicGain we are raising the threshold
to about 5 or 6 dB above the predicted echo value. This is justified because
even in the absence of doubletalk the actual Mic value will deviate from the
prediction. */

#if (0)
   Temp0 = 0;
   Temp2 = 0;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
      Temp1 = MicDB[Band] - (LdspkrDB[Band] + LdspkrMicGain[Band]);
      Temp1 = Temp1/1000;
      Temp0 += Temp1*Temp1;
      Temp2 += Temp1;
   }
   printf("%3d %2d ",Temp0/NUM_BANDS_PROCESSED,Temp2/NUM_BANDS_PROCESSED);
   if (Frame10msCount%10 == 9) printf("\n");
#endif

   Temp0 = 0;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
      Temp1 = MicDB[Band] - (LdspkrDB[Band] + LdspkrMicGain[Band] + 9000);  // Was 9000 before 4/23
      if (Temp1 > 0)
         Temp0 += Temp1;
   }

   int DoubletalkIndicator;
   DoubletalkIndicator  = Temp0/NUM_BANDS_PROCESSED;


   // Update estimate of loudspeaker-to-mic gain for each of the bands.
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
      // Compute upper-limited loudspeaker power minus fixed threshold.
      // Any attack or decay of LdspkrMicGain[i] will be proportional to this.
      Temp0 = LdspkrDB[Band] - LMGThresh0[Band];
      if (Temp0 < 0)
         Temp0 = 0;
      else if (Temp0 > 10240)
         Temp0 = 10240;    // 10240 <==> 10 dB


      // mic power in dB (x 1000) minus predicted echo power in dB (x 1000).
      Temp1 = MicDB[Band] - (LdspkrDB[Band] + LdspkrMicGain[Band]);
      if (Temp1 > 6000) {
// slow attack if mic power is more than 6 db larger than predicted echo power
// Max rate of attack: 0.976 dB per second (100 (10 ms frames/sec) * (10240 >> 10) = 1000)
         LdspkrMicGain[Band] += (Temp0 >> 10);

#if 1
         if (Temp1 > 10240) {           // Sept 20, 2002
/* If average DoubletalkIndicator is small, but large doubletalk (large enough to
disable echo canceller updates) is found in a band, then increase the LdspkrMicGain[]
for that band faster than normal. */
             if (DoubletalkIndicator < 1500) {
                 LdspkrMicGain[Band] += (Temp0 >> 9);
             }
             if (DoubletalkIndicator < 1000) {
                 LdspkrMicGain[Band] += (Temp0 >> 9);
             }

         }
#endif

      }
      else if (Temp1 < -3000) {
// Fast decay if mic power is more than 3 dB below predicted echo power
// Max rate of decay: 7.8125 dB per second (100 (10 ms frames/sec) * (10240 >> 7) = 8000)
         LdspkrMicGain[Band] -= (Temp0 >> 7);
      }
   }

#if (0)
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band += 2) {
      printf("%2d ",LdspkrDB[Band]/1000);
      //printf("%2d ",MicDB[Band]/1000);
//    if (Band%21 == 20) printf("\n");
   }
   printf("\n");
#endif

#if (0)
   if (Frame10msCount%200 == 199) {
      printf("%4d:",Frame10msCount);
      for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
         printf("%3d ",LdspkrMicGain[Band]/1000);
         if (Band%14 == 13) printf("\n");
      }
      printf("\n");
//    getchar();
   }
#endif

   return(DoubletalkIndicator);     // returns DoubletalkIndicator
}
/* ************************************************************************** */
/* ******************** End of Doubletalk Detection ************************* */
/* ************************************************************************** */

/* ************************************************************************** */
/* *********************** Compute Loudspeaker Fade********* **************** */
/* ************************************************************************** */

#if (VX1_PC0 == 1)
void FilterBank::ComputeLoudspeakerFade()
#else    // #else for VX1_PC0
void ComputeLoudspeakerFade()
#endif  // #endif for VX1_PC0
{

// There may be a similar or identical calculation in EchoSuppress();
   static int LoudspeakerFadeDIThresh = 1000;
   static int LoudspeakerFadeDILeakConst = 60;
   static int LoudspeakerFadeDIMultiplier = 12;
   static int MaxLoudspeakerFade = 6;
   static int LoudspeakerFadeMicPowThresh = 20000;          // unit is raw power, not dB
   static int LoudspeakerFadeLdspkrPowThreshdB = 43000;        // unit is dB times 1000

   static int LoudspeakerFadeLeakyMaxDoubletalkIndicator = 0;

   int Temp0,Temp1;
   int Band;
   int TargetLoudspeakerFadeDB;


/* Compute sum of weighted loudspeaker signal power for frequencies above and below
   2500 hz. Then take the maximum of the two frequency ranges. */

   Temp0 = 0;
   for (Band=0; Band < 25; Band++)
   {
       Temp0 += WeightedLdspkrDLPow[Band] >> 4;
   }

   // Stronger weighting since high frequency sounds don't have to be as loud
   // to produce audible echo.
   Temp1 = 0;
   for (Band=25; Band < NUM_BANDS_PROCESSED; Band++)
   {
       Temp1 += WeightedLdspkrDLPow[Band] >> 1;
   }

   if (Temp1 > Temp0) Temp0 = Temp1;
   SumWeightedLdspkrPowDBMax = 10*Get1000log10(Temp0);


/* DEBUG!!!!! A low ERL may be another good reason to increase fade, since a low ERL is due
   to either a diverged echo canceller (motion) or doubletalk. However, increasing fade may
   result in eventually worse ERL. */

// Compute mic power above 100 hz.
   SumMicPow = 0;
   for (Band = 1; Band < NUM_BANDS_PROCESSED; Band++) {   // DEBUG!!!! Change if LOW_BAND changes.
       SumMicPow += MicPow[Band] >> 5;
   }



   Temp0 = DoubletalkIndicator - LoudspeakerFadeDIThresh;

// Ignore DoubletalkIndicator if loudspeaker power is below threshold.
   if (SumWeightedLdspkrPowDBMax < LoudspeakerFadeLdspkrPowThreshdB)
   {
       Temp0 = 0;
   }

// Ignore DoubletalkIndicator if mic power is below threshold.
   if (SumMicPow < LoudspeakerFadeMicPowThresh)
   {
       Temp0 = 0;
   }

   if (Temp0 > 8000) Temp0 = 8000;
   LoudspeakerFadeLeakyMaxDoubletalkIndicator =
       (LoudspeakerFadeDILeakConst*LoudspeakerFadeLeakyMaxDoubletalkIndicator) >> 6;
   if (Temp0 > LoudspeakerFadeLeakyMaxDoubletalkIndicator)
       LoudspeakerFadeLeakyMaxDoubletalkIndicator = Temp0;

   TargetLoudspeakerFadeDB =
       (LoudspeakerFadeDIMultiplier*LoudspeakerFadeLeakyMaxDoubletalkIndicator) >> 12;
   if (TargetLoudspeakerFadeDB > MaxLoudspeakerFade) TargetLoudspeakerFadeDB = MaxLoudspeakerFade;


   iLoudspeakerFadeDB = -TargetLoudspeakerFadeDB; // need a range of 0 to -6dB

#if (VX1_PC0 == 0)
#if 1
   printf("%5d %10d %5d %5d Fade=%1d    ",SumWeightedLdspkrPowDBMax, SumMicPow, DoubletalkIndicator,
       LoudspeakerFadeLeakyMaxDoubletalkIndicator, TargetLoudspeakerFadeDB);
   fprintf(fp_stats,"%5d %10d %5d %5d Fade=%1d    ",SumWeightedLdspkrPowDBMax, SumMicPow, DoubletalkIndicator,
       LoudspeakerFadeLeakyMaxDoubletalkIndicator, TargetLoudspeakerFadeDB);
#endif
#endif


}
/* ************************************************************************** */
/* ************* Main echo canceller subband processing function **************** */
/* ************************************************************************** */

#if (VX1_PC0 == 1)
void FilterBank::SubbandECLoop(icomplex TwoFrameMicFFTArray[][M+1],
                        icomplex TwoFrameLdspkrFFTArray[][M+1],
                        int MicDB[],
                        int LdspkrDB[],
                        int ResDB[])

#else    // #else for VX1_PC0

void SubbandECLoop(icomplex TwoFrameMicFFTArray[][M+1],
               icomplex TwoFrameLdspkrFFTArray[][M+1],
               int MicDB[],
               int LdspkrDB[],
               int ResDB[])
#endif  // #endif for VX1_PC0
{
   int Band;
   int Temp0, Temp1, Temp2;
   int FrameNum;
   int EcIndex;
   int DLIndex;
   int ECDLBandNewestIndex;

/* Do echo cancellation and update for each of the NUM_BANDS_PROCESSED bands */
   Band = ECDLHighestAddressBand;
   ECDLHighestAddressBandNewestIndex += 2;
   ECDLBandNewestIndex = ECDLHighestAddressBandNewestIndex;

#if (0)
   printf("ECDLHighestAddressBand=%2d ECDLHighestAddressBandNewestIndex=%4d\n",
      ECDLHighestAddressBand,ECDLHighestAddressBandNewestIndex);
#endif

   int BandCount;
   for (BandCount=0; BandCount < NUM_BANDS_PROCESSED; BandCount++) {

/* Copy the new complex loudspeaker samples into the delay line (of complex loudspeaker samples)
for an entire 10 ms frame. */
// Use a single complex pole filter to boost the band edges and make the reference signal whiter.
#define POLE_CONST 6    // MODULATION must be turned on for this to work.
       ECDL[ECDLBandNewestIndex].r = TwoFrameLdspkrFFTArray[0][Band + LOW_BAND].r -
          ((POLE_CONST*ECDL[ECDLBandNewestIndex-1].r + 8) >> 4);
       ECDL[ECDLBandNewestIndex].i = TwoFrameLdspkrFFTArray[0][Band + LOW_BAND].i -
          ((POLE_CONST*ECDL[ECDLBandNewestIndex-1].i + 8) >> 4);
       ECDL[ECDLBandNewestIndex+1].r = TwoFrameLdspkrFFTArray[1][Band + LOW_BAND].r -
          ((POLE_CONST*ECDL[ECDLBandNewestIndex].r + 8) >> 4);
       ECDL[ECDLBandNewestIndex+1].i = TwoFrameLdspkrFFTArray[1][Band + LOW_BAND].i -
          ((POLE_CONST*ECDL[ECDLBandNewestIndex].i + 8) >> 4);

// Update WeightedLdspkrDLPow[Band] for use next frame. We do the update here
// minimize cache hits associated with accessing the loudspeaker delay line.

#if USE_IMAGSQ
       Temp2 = imagsq(&(ECDL[ECDLBandNewestIndex-2]), 4);
       Temp2 += imagsq(&(ECDL[ECDLBandNewestIndex-1]), 4);
       Temp2 += (imagsq(&(ECDL[ECDLBandNewestIndex]), 4)) >> 3;
       Temp2 += (imagsq(&(ECDL[ECDLBandNewestIndex+1]), 4)) >> 3;

#else
       Temp0 = ECDL[ECDLBandNewestIndex-2].r >> 4;
       Temp1 = ECDL[ECDLBandNewestIndex-2].i >> 4;
       Temp2 = Temp0*Temp0 + Temp1*Temp1;

       Temp0 = ECDL[ECDLBandNewestIndex-1].r >> 4;
       Temp1 = ECDL[ECDLBandNewestIndex-1].i >> 4;
       Temp2 += Temp0*Temp0 + Temp1*Temp1;

       Temp0 = ECDL[ECDLBandNewestIndex].r >> 4;
       Temp1 = ECDL[ECDLBandNewestIndex].i >> 4;
       Temp2 += (Temp0*Temp0 + Temp1*Temp1) >> 3;

       Temp0 = ECDL[ECDLBandNewestIndex+1].r >> 4;
       Temp1 = ECDL[ECDLBandNewestIndex+1].i >> 4;
       Temp2 += (Temp0*Temp0 + Temp1*Temp1) >> 3;
#endif

// DEBUG!!!!!! Could and should make decay rate a function of EchoDecaydB
// Changed Sept. 18, 2002
//       Temp2 += ((WeightedLdspkrDLPow[Band]) >> 1);
//       WeightedLdspkrDLPow[Band] = Temp2;
       Temp2 += ((9*WeightedLdspkrDLPow[Band]) >> 4);
       WeightedLdspkrDLPow[Band] = Temp2;



#if DEBUG_ECHO_ESTIMATE_POW
// New DEBUG!!! Aug 8, 2002
       EchoEstimatePow[Band] = 0;
#endif

       for (FrameNum = 0; FrameNum < 2; FrameNum++)
       {

// Recursively compute power in loudspeaker delay line for Band.
// Add the newest sample power to the current value in LdspkrDLPow[Band].
#define SHIFT2 (4)
           DLIndex = ECDLBandNewestIndex + FrameNum;

#if USE_IMAGSQ
           LdspkrDLPow[Band] += imagsq(&(ECDL[DLIndex]), SHIFT2);
#else
           Temp0 = ECDL[DLIndex].r >> SHIFT2;
           Temp1 = ECDL[DLIndex].i >> SHIFT2;
           LdspkrDLPow[Band] += Temp0*Temp0 + Temp1*Temp1;
#endif

/* Compute the complex echo estimate for band i. */
#define SHIFT1 16
#if (ARITHMETIC_TYPE == 0)
           EcIndex = ECNumTaps[Band]-1;
           DLIndex = ECDLBandNewestIndex + FrameNum;
           {
               int TempDLr, TempDLi;
               Temp0 = 0;
               Temp1 = 0;
               while (EcIndex >= 0) {
                   int Temp3;
                   Temp2 = EchoCancellerCoef[Band][EcIndex].r;
                   Temp3 = EchoCancellerCoef[Band][EcIndex].i;
                   TempDLr = ECDL[DLIndex].r;
                   TempDLi = ECDL[DLIndex].i;
                   Temp0 += Temp2 * TempDLr - Temp3 * TempDLi;
                   Temp1 += Temp2 * TempDLi + Temp3 * TempDLr;
                   EcIndex--;
                   DLIndex--;
               }
               TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].r -= (Temp0 + 32768) >> SHIFT1;
               TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].i -= (Temp1 + 32768) >> SHIFT1;
           }
#elif (ARITHMETIC_TYPE == 1)
           EcIndex = ECNumTaps[Band]-1;
           DLIndex = ECDLBandNewestIndex + FrameNum;
           double dAccR,dAccI;
           dAccR = 0.0;
           dAccI = 0.0;
           while (EcIndex >= 0) {
               int Temp3;
               Temp2 = EchoCancellerCoef[Band][EcIndex].r;
               Temp3 = EchoCancellerCoef[Band][EcIndex].i;
               Temp0 = ECDL[DLIndex].r;
               Temp1 = ECDL[DLIndex].i;
               dAccR += ((double) Temp2 * (double) Temp0 -
                   (double) Temp3 * (double) Temp1);
               dAccI += ((double) Temp2 * (double) Temp1 +
                   (double) Temp3 * (double) Temp0);
               EcIndex--;
               DLIndex--;
           }

#if (1)
           {
               static double MAXdAcc = 0;
               if (fabs(dAccR) > MAXdAcc) {
                   MAXdAcc = fabs(dAccR);
                   printf("MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
                   fprintf(fp_stats,"MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
               }
               if (fabs(dAccI) > MAXdAcc)  {
                   MAXdAcc = fabs(dAccI);
                   printf("MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
                   fprintf(fp_stats,"MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
               }
           }
#endif
           if (dAccR >= 0.0)
               Temp0 = (int) ((dAccR+32768.0)/65536.0);
           else
               Temp0 = (int) ((dAccR-32768.0)/65536.0);
           if (dAccI >= 0.0)
               Temp1 = (int) ((dAccI+32768.0)/65536.0);
           else
               Temp1 = (int) ((dAccI-32768.0)/65536.0);

           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].r -= Temp0;
           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].i -= Temp1;

#if DEBUG_ECHO_ESTIMATE_POW
// New DEBUG!!! Aug 8, 2002
           Temp0 >>= MIC_GAIN_SHIFT;
           Temp1 >>= MIC_GAIN_SHIFT;
           EchoEstimatePow[Band] += Temp0*Temp0 + Temp1*Temp1;
#endif

#elif (ARITHMETIC_TYPE == 2)
           EcIndex = ECNumTaps[Band]-1;
           DLIndex = ECDLBandNewestIndex + FrameNum;

           Word64S       LongTempR;
           Word64S       LongTempI;
           LongTempR = 0;
           LongTempI = 0;
           while (EcIndex >= 0) {
               Word64S   LTempR1;
               Word64S   LTempI1;
               Word64S   LTempR2;
               Word64S   LTempI2;
               LTempR1 = EchoCancellerCoef[Band][EcIndex].r;
               LTempI1 = EchoCancellerCoef[Band][EcIndex].i;
               LTempR2 = ECDL[DLIndex].r;
               LTempI2 = ECDL[DLIndex].i;
               LongTempR += LTempR1 * LTempR2;
               LongTempR -= LTempI1 * LTempI2;
               LongTempI += LTempR1 * LTempI2;
               LongTempI += LTempI1 * LTempR2;
               EcIndex--;
               DLIndex--;
           }
           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].r -= (LongTempR + 32768) >> 16;
           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].i -= (LongTempI + 32768) >> 16;

#elif (ARITHMETIC_TYPE == 3)
           EcIndex = ECNumTaps[Band]-1;
           DLIndex = ECDLBandNewestIndex + FrameNum;
           icomplex cTemp;
           ComplexInnerProduct(&cTemp, &EchoCancellerCoef[Band][EcIndex], &ECDL[DLIndex], EcIndex);
           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].r -= cTemp.r;
           TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].i -= cTemp.i;
#endif

           if (UpdateControl)
           {

               if ((DoubletalkIndicator < 3000) &&
                   (LdspkrDLPow[Band] > ThresholdLdspkrDLPow[Band]) &&
                   (MicDB[Band] < (LdspkrDB[Band] + LdspkrMicGain[Band] + 10240)))
               {
                   DebugUpdateCount[Band]++;       // DEBUG!!!!!!

// Adapt the echo canceller coefficients for Band. */
#define SHIFT3 8
//#define UPDATE_CONSTANT 1.0
#define UPDATE_CONSTANT ((1 << SHIFT1)/2)    // denominator should be 2**(2*SHIFT2 - SHIFT3)
//#define UPDATE_CONSTANT 0
                   icomplex NormedErr;
// Update constant = 1.0
// Numerator interpreted as positive
// added *4 on 4/29/02
                   Temp0 = (0xff000000)/(LdspkrDLPow[Band]+(4*ThresholdLdspkrDLPow[Band])); //upconst=1
                   NormedErr.r = (Temp0 * TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].r + 128) >> SHIFT3;
                   NormedErr.i = (Temp0 * TwoFrameMicFFTArray[FrameNum][Band + LOW_BAND].i + 128) >> SHIFT3;
                   EcIndex = ECNumTaps[Band]-1;
                   DLIndex = ECDLBandNewestIndex + FrameNum;

                   // Graded updates
/* The coefficients which tend to be small get smaller updates. This speeds convergence. */
                   ComplexCoefUpdate(&NormedErr,
                       &EchoCancellerCoef[Band][EcIndex],
                       &ECDL[DLIndex],
                       EcIndex);
               }
           }

/* Recursively compute power in loudspeaker delay line for Band.
Subtract the oldest sample's power from the current value in LdspkrDLPow[Band]. */
           DLIndex = ECDLBandNewestIndex + FrameNum - (ECNumTaps[Band]-1);

#if USE_IMAGSQ
           LdspkrDLPow[Band] -= imagsq(&(ECDL[DLIndex]), SHIFT2);
#else
           Temp0 = ECDL[DLIndex].r >> SHIFT2;
           Temp1 = ECDL[DLIndex].i >> SHIFT2;
           LdspkrDLPow[Band] -= Temp0*Temp0 + Temp1*Temp1;
#endif
        }


/* Compute the residual power for Band */    // DEBUG!!! For now, this isn't used later.
#if COMPUTE_BAND_ERL
        SumFBResPow[Band] -= (SumFBResPow[Band] >> 5);
#endif

#if USE_IMAGSQ
       Temp2 = imagsq(&(TwoFrameMicFFTArray[0][Band + LOW_BAND]), MIC_GAIN_SHIFT);
       Temp2 += imagsq(&(TwoFrameMicFFTArray[1][Band + LOW_BAND]), MIC_GAIN_SHIFT);
#else
       Temp0 = TwoFrameMicFFTArray[0][Band + LOW_BAND].r >> MIC_GAIN_SHIFT;
       Temp1 = TwoFrameMicFFTArray[0][Band + LOW_BAND].i >> MIC_GAIN_SHIFT;
       Temp2 = (Temp0*Temp0 + Temp1*Temp1);
       Temp0 = TwoFrameMicFFTArray[1][Band + LOW_BAND].r >> MIC_GAIN_SHIFT;
       Temp1 = TwoFrameMicFFTArray[1][Band + LOW_BAND].i >> MIC_GAIN_SHIFT;
       Temp2 += (Temp0*Temp0 + Temp1*Temp1);
#endif

#if COMPUTE_BAND_ERL
       SumFBResPow[Band] += (Temp2 + 2) >> 2;
#endif

       ResDB[Band] = 10*Get1000log10(Temp2);


/* Decrement the Band and adjust the pointer to the newest sample in ECDL[] for Band. */
       ECDLBandNewestIndex -= (ECNumTaps[Band] - 1);
       Band--;
       if (Band < 0) Band += NUM_BANDS_PROCESSED;
    }    // This is the  actual end of the compute-intensive filterbank signal processing loop
         // for in-band processing.


/* If there is enough free space at the lowest address end of ECDL[], then copy the highest adresss
band's delay line values to the free space. */
    if (((ECDLBandNewestIndex+1) - (ECNumTaps[ECDLHighestAddressBand] - 2)) >= 0)
    {
       icomplex *DestPtr;
       icomplex *SourcePtr;
       DestPtr = &ECDL[(ECDLBandNewestIndex+1) - (ECNumTaps[ECDLHighestAddressBand]-2)];
       SourcePtr = &ECDL[(ECDLHighestAddressBandNewestIndex+1) - (ECNumTaps[ECDLHighestAddressBand]-2)];
       memcpy(DestPtr, SourcePtr, (sizeof (icomplex)) * (ECNumTaps[ECDLHighestAddressBand]-1));

       ECDLHighestAddressBandNewestIndex -= ECNumTaps[ECDLHighestAddressBand]-1;

#define RECOMPUTE_POWER_FLAG 1      // DEBUG!!! Eventually eliminate this.
#if RECOMPUTE_POWER_FLAG
       int RecomputeDLPowBand;
       RecomputeDLPowBand = ECDLHighestAddressBand;   // Store index of new lowest address band.
#endif

       ECDLHighestAddressBand--;
       if (ECDLHighestAddressBand < 0) ECDLHighestAddressBand += NUM_BANDS_PROCESSED;


#if RECOMPUTE_POWER_FLAG
/* Recompute nonrecursively the delay line power for the new lowest address band.
Otherwise, some error in the computation would persist indefinitely, and it might be difficult
to detect.
DEBUG!!!! This should be unnecessary. */
       EcIndex = ECNumTaps[RecomputeDLPowBand]-2;     // We don't include the oldest sample.
       DLIndex = ECDLBandNewestIndex+1;
       Temp2 = 0;
       while (EcIndex >= 0) {
#if USE_IMAGSQ
          Temp2 += imagsq(&(ECDL[DLIndex]), SHIFT2);
#else
          Temp0 = ECDL[DLIndex].r >> SHIFT2;
          Temp1 = ECDL[DLIndex].i >> SHIFT2;
          Temp2 += Temp0*Temp0 + Temp1*Temp1;
#endif
          EcIndex--;
          DLIndex--;
       }
#if (VX1_PC0 == 0)
       if (Temp2 != LdspkrDLPow[RecomputeDLPowBand])
       {
          printf("Error!!! Temp2=%d LdspkrDLPow[%d]=%d \n",
          Temp2,RecomputeDLPowBand,LdspkrDLPow[RecomputeDLPowBand]);
          fprintf(fp_stats,"Error!!! Temp2=%d LdspkrDLPow[%d]=%d \n",
          Temp2,RecomputeDLPowBand,LdspkrDLPow[RecomputeDLPowBand]);
       }
#else
       if (Temp2 != LdspkrDLPow[RecomputeDLPowBand])
          osPrintf("Error!!! Temp2=%d LdspkrDLPow[%d]=%d \n",
          Temp2,RecomputeDLPowBand,LdspkrDLPow[RecomputeDLPowBand]);

#endif
       LdspkrDLPow[RecomputeDLPowBand] = Temp2;
#endif

    }


// The following operates on one band per frame.
// Measures the power in the low delay segment of the echo canceller and in the tail.
// Also, limit the coefficients magnitude.

    static int MaxCoefValue = 0;                // DEBUG!!!!!!

#define LIMIT_COEFS_FLAG 1
#define PRINT_COEF_LIMITING_FLAG 1
#define COEF_LIMIT 120000

    int LowDelayPower;
    int TailPower;
    LowDelayPower = 0;      // Power in first 8 coefficients
    TailPower = 0;          // Power in last 6 coefficients

    EcIndex = ECNumTaps[CoefExamineBand]-1;
    while (EcIndex >= 0) {
        Temp0 = EchoCancellerCoef[CoefExamineBand][EcIndex].r;
        Temp1 = EchoCancellerCoef[CoefExamineBand][EcIndex].i;

// DEBUG!!!!!!
        if (abs(Temp0) > MaxCoefValue) MaxCoefValue = abs(Temp0);
        if (abs(Temp1) > MaxCoefValue) MaxCoefValue = abs(Temp1);

#if LIMIT_COEFS_FLAG
        if (Temp0 > COEF_LIMIT)
        {
#if PRINT_COEF_LIMITING_FLAG
#if (VX1_PC0 == 1)
            osPrintf("EchoCancellerCoef[%2d][%2d].r = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].r);
#else
            printf("EchoCancellerCoef[%2d][%2d].r = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].r);
#endif
#endif
            EchoCancellerCoef[CoefExamineBand][EcIndex].r = COEF_LIMIT;
        }
        if (Temp0 < -COEF_LIMIT)
        {
#if PRINT_COEF_LIMITING_FLAG
#if (VX1_PC0 == 1)
            osPrintf("EchoCancellerCoef[%2d][%2d].r = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].r);
#else
            printf("EchoCancellerCoef[%2d][%2d].r = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].r);
#endif
#endif
            EchoCancellerCoef[CoefExamineBand][EcIndex].r = -COEF_LIMIT;
        }

        if (Temp1 > COEF_LIMIT)
        {
#if PRINT_COEF_LIMITING_FLAG
#if (VX1_PC0 == 1)
            osPrintf("EchoCancellerCoef[%2d][%2d].i = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].i);
#else
            printf("EchoCancellerCoef[%2d][%2d].i = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].i);
#endif
#endif
            EchoCancellerCoef[CoefExamineBand][EcIndex].i = COEF_LIMIT;
        }
        if (Temp1 < -COEF_LIMIT)
        {
#if PRINT_COEF_LIMITING_FLAG
#if (VX1_PC0 == 1)
            osPrintf("EchoCancellerCoef[%2d][%2d].i = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].i);
#else
            printf("EchoCancellerCoef[%2d][%2d].i = %d\n",CoefExamineBand,EcIndex,
                EchoCancellerCoef[CoefExamineBand][EcIndex].i);
#endif
#endif
            EchoCancellerCoef[CoefExamineBand][EcIndex].i = -COEF_LIMIT;
        }
#endif

#define COEF_EXAMINE_SHIFT_LOW_DELAY 7
#define COEF_EXAMINE_SHIFT_TAIL 3
        if (EcIndex >= ECNumTaps[CoefExamineBand]-8)
        {
            Temp0 >>= COEF_EXAMINE_SHIFT_LOW_DELAY;
            Temp1 >>= COEF_EXAMINE_SHIFT_LOW_DELAY;
            LowDelayPower += Temp0*Temp0 + Temp1*Temp1;
        }
        else if (EcIndex < 6)
        {
            Temp0 >>= COEF_EXAMINE_SHIFT_TAIL;
            Temp1 >>= COEF_EXAMINE_SHIFT_TAIL;
            TailPower += Temp0*Temp0 + Temp1*Temp1;
        }

        EcIndex--;
    }
    BandLowDelayPower[CoefExamineBand] = LowDelayPower;
    BandTailPower[CoefExamineBand] = TailPower;

    CoefExamineBand++;
    if (CoefExamineBand >= NUM_BANDS_PROCESSED)
    {
        CoefExamineBand = 0;

        LowDelayPower = 0;      // Power in first 8 coefficients
        TailPower = 0;          // Power in last 6 coefficients
        for (Band=0; Band < NUM_BANDS_PROCESSED; Band++) {
            LowDelayPower += BandLowDelayPower[Band] >> 5;
            TailPower += BandTailPower[Band] >> 5;
        }
        EchoDecaydB = 602*(COEF_EXAMINE_SHIFT_LOW_DELAY-COEF_EXAMINE_SHIFT_TAIL) +
            Get1000log10(LowDelayPower) - Get1000log10(TailPower);

#if (VX1_PC0 == 1)
 //           osPrintf(">>>>>>>>EchoDecaydB = %5d   MaxCoefValue = %6d <<<<<<<\n",EchoDecaydB,MaxCoefValue);
#endif
            MaxCoefValue = 0;

#if (VX1_PC0 == 0)
        printf("LowDelayPower = %10d TailPower = %10d %4d %4d %4d\n",LowDelayPower,TailPower,
            Get1000log10(LowDelayPower),Get1000log10(TailPower),EchoDecaydB);
        fprintf(fp_stats,"LowDelayPower = %10d TailPower = %10d %4d %4d %4d\n",LowDelayPower,TailPower,
            Get1000log10(LowDelayPower),Get1000log10(TailPower),EchoDecaydB);
#endif
    }


}




/* ************************************************************************** */
/* ******** End of Main echo canceller subband processing function ********** */
/* ************************************************************************** */



/* ************************************************************************** */
/* ************************* Echo Suppression ******************************* */
/* ************************************************************************** */


#if (VX1_PC0 == 1)
void FilterBank::EchoSuppress(icomplex TwoFrameMicFFTArray[][M+1],
                      int LdspkrDB[],
                      int ResDB[],
                      int MicDB[],
                      int DoubletalkIndicator)

#else    // #else for VX1_PC0

void EchoSuppress(icomplex TwoFrameMicFFTArray[][M+1],
             int LdspkrDB[],
             int ResDB[],
             int MicDB[],
             int DoubletalkIndicator)
#endif  // #endif for VX1_PC0
{
   int Band;
   int i;
   int Temp0, Temp1;
   int TargetEchoSupDB;


#define ESUP_SHIFT 8
   int EchoSuppressionS1;
   int EchoSuppressionS2;
   EchoSuppressionS1 = (EchoSuppressionY1 << ESUP_SHIFT)/(EchoSuppressionX1 - EchoSuppressionX0);
   EchoSuppressionS2 = ((EchoSuppressionY2-EchoSuppressionY1) << ESUP_SHIFT)/(EchoSuppressionX2 - EchoSuppressionX1);
   if (SumWeightedLdspkrPowDBMax < EchoSuppressionX0*1024) TargetEchoSupDB = 0;
   else if (SumWeightedLdspkrPowDBMax < EchoSuppressionX1*1024)
   {
       TargetEchoSupDB = ((SumWeightedLdspkrPowDBMax - EchoSuppressionX0*1024) * EchoSuppressionS1) >> ESUP_SHIFT;
   }
   else
   {
       TargetEchoSupDB = EchoSuppressionY1*1024 + (((SumWeightedLdspkrPowDBMax - EchoSuppressionX1*1024) * EchoSuppressionS2) >> ESUP_SHIFT);
   }
   if (TargetEchoSupDB > MAX_TARGET_ECHO_SUP_DB) TargetEchoSupDB = MAX_TARGET_ECHO_SUP_DB;


#if (VX1_PC0 == 1)
#if 0
   if (Frame10msCount%10 == 9)
   {
       osPrintf("SLP=%2d TES=%2d ",SumWeightedLdspkrPowDBMax >>10,TargetEchoSupDB >> 10);
   }
#endif
#endif



#if (VX1_PC0 == 0)
   printf(" %5d",SumWeightedLdspkrPowDBMax);
   fprintf(fp_stats,"  %5d",SumWeightedLdspkrPowDBMax);
   printf(" %5d ",TargetEchoSupDB);
   fprintf(fp_stats," %5d ",TargetEchoSupDB);
#endif


// Reduce echo suppression if there is doubletalk.
   static int LeakyMaxDoubletalkIndicator = 0;

   Temp0 = DoubletalkIndicator - EchoSuppressionDIThresh;
   if (SumMicPow < 20000)     // Ignore DoubletalkIndicator if mic power is below threshold.
   {
       Temp0 = 0;
   }
   if (Temp0 > 8000) Temp0 = 8000;
   LeakyMaxDoubletalkIndicator = (EchoSuppressionDIAvgConst*LeakyMaxDoubletalkIndicator) >> 6;
   if (Temp0 > LeakyMaxDoubletalkIndicator) LeakyMaxDoubletalkIndicator = Temp0;

   TargetEchoSupDB -= (EchoSuppressionDISubConst*LeakyMaxDoubletalkIndicator) >> 4;
   if (TargetEchoSupDB < 0) TargetEchoSupDB = 0;
   if (TargetEchoSupDB > MAX_TARGET_ECHO_SUP_DB) TargetEchoSupDB = MAX_TARGET_ECHO_SUP_DB;



#if (VX1_PC0 == 0)
   Temp0 = 0;
   for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
      Temp0 += LdspkrMicGain[Band];
   Temp0 /= NUM_BANDS_PROCESSED;
   printf("  %5d",Temp0);
   fprintf(fp_stats,"  %5d",Temp0);

   printf("  %5d",DoubletalkIndicator);
   fprintf(fp_stats,"  %5d",DoubletalkIndicator);

   if (DoubletalkIndicator >= 1000) {
       printf("*");
       fprintf(fp_stats,"*");
   }
   else {
       printf(" ");
       fprintf(fp_stats," ");
   }

   printf(" %5d",LeakyMaxDoubletalkIndicator);
   fprintf(fp_stats," %5d",LeakyMaxDoubletalkIndicator);
   printf(" %5d",TargetEchoSupDB);
   fprintf(fp_stats," %5d",TargetEchoSupDB);
#endif


// temporal hold: Take maximum of last N targets
   static int TargetHistory[100];
   for (i=99; i > 0; i--)
       TargetHistory[i] = TargetHistory[i-1];
   TargetHistory[0] = TargetEchoSupDB;


   Temp1 = 0;
   for (i = 0; i < EchoSuppressionHoldTime; i++)
   {
       if (TargetHistory[i] > Temp1) Temp1 = TargetHistory[i];
   }


// temporal smoothing
//   Temp0 = TargetEchoSupDB - EchoSupDB;
   Temp0 = Temp1 - EchoSupDB;


   EchoSuppressionMaxDecay = (-8 * EchoDecaydB) >> 4;


   if (Temp0 < EchoSuppressionMaxDecay) Temp0 = EchoSuppressionMaxDecay;
   EchoSupDB += Temp0;


#if (VX1_PC0 == 1)
#if 0
   if (Frame10msCount%10 == 9)
   {
       osPrintf("%4d %4d",EchoSupDB >>10, EchoSuppressionMaxDecay>>10);
   }
#endif
#endif



//   printf("%6d %6d\n",SumWeightedLdspkrPowDBMax, EchoSupDB);

    for (Band=0; Band < NUM_BANDS_PROCESSED; Band++)
    {
       Temp0 = EchoSupDB >> 10;

       Temp1 = EchoSupMultiplierTable[Temp0];

       TwoFrameMicFFTArray[0][Band + LOW_BAND].r =
          (Temp1 * TwoFrameMicFFTArray[0][Band + LOW_BAND].r + 2048) >> 12;
       TwoFrameMicFFTArray[0][Band + LOW_BAND].i =
          (Temp1 * TwoFrameMicFFTArray[0][Band + LOW_BAND].i + 2048) >> 12;
       TwoFrameMicFFTArray[1][Band + LOW_BAND].r =
          (Temp1 * TwoFrameMicFFTArray[1][Band + LOW_BAND].r + 2048) >> 12;
       TwoFrameMicFFTArray[1][Band + LOW_BAND].i =
          (Temp1 * TwoFrameMicFFTArray[1][Band + LOW_BAND].i + 2048) >> 12;
    }

#if (VX1_PC0 == 0)
   printf("   %3d\n",EchoSupDB >> 10);
   fprintf(fp_stats,"   %3d\n",EchoSupDB >> 10);
#endif


}




/* ************************************************************************** */
/* ********************* End of Echo Suppression **************************** */
/* ************************************************************************** */


/* ************************************************************************** */
// Sum of squares function
#if (VX1_PC0 == 0)

int imagsq(icomplex *ptr, int shift)
{
   int ComplexPower;
   int Temp0, Temp1;
   Temp0 = (ptr->r) >> shift;
   Temp1 = (ptr->i) >> shift;
   ComplexPower = Temp0*Temp0 + Temp1*Temp1;
   return(ComplexPower);
}

#endif



/* ************************************************************************** */
extern "C" {
extern int chooseComplexImplementation(int flag);
extern int cCI(int flag);
}

static int cCI_mode = 1;
int chooseComplexImplementation(int flag)
{
   int save = cCI_mode;
   cCI_mode = flag & 3;
   return save;
}
int cCI(int flag) {return chooseComplexImplementation(flag);}

#define cipCMax 100
#undef cipCMax
#ifdef cipCMax /* [ */
static int cipC[cipCMax] = {
   0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
   0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
   0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
   0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0
};

extern "C" int showCipC(int reset)
{
   int i, c = 0, n = 0;

   for (i=0; i<cipCMax; i++) {
      if (cipC[i]) {
         n += cipC[i];
         c += printf(" %d: %d", i, cipC[i]);
         if (c > 70) {
            printf("\n");
            c = 0;
         }
         if (reset) cipC[i] = 0;
      }
   }
   if (c) printf("\n");
   return n;
}
#endif /* cipCMax ] */

/* ************************************************************************** */
/* Complex inner product to be written in assembly language. */
// #if (ARITHMETIC_TYPE == 3)


void ComplexInnerProduct(icomplex *ResultPtr,
                   icomplex *CoeffPtr,
                   icomplex *DLPtr,
                   int EcIndex)


{
	/*
 if (cCI_mode & 1) {
#ifdef cipCMax
   int foo;
   if (0 > EcIndex) foo = 0;
     else if (cipCMax > EcIndex) foo = EcIndex;
     else foo = cipCMax-1;
   cipC[foo]++;
#endif

   complexInnerProduct(ResultPtr, CoeffPtr, DLPtr, EcIndex);
 } else
*/{
#if (VX1_PC0 == 1)
   Word64S LongTempR, LongTempI;
   Word64S CoefR, CoefI;      // These don't need to be long. They are long here only for compiler.
   Word64S DLR, DLI;       // These don't need to be long. They are long here only for compiler.
#else
   int LongTempR, LongTempI;
   int CoefR, CoefI;
   int DLR, DLI;
#endif
   LongTempR = 0;
   LongTempI = 0;
   while (EcIndex >= 0) {     // Largest contemplated EcIndex is 22
      CoefR = CoeffPtr->r;
      CoefI = CoeffPtr->i;
      CoeffPtr--;
      DLR = DLPtr->r;
      DLI = DLPtr->i;
      DLPtr--;
      LongTempR += CoefR * DLR;
      LongTempR -= CoefI * DLI;
      LongTempI += CoefR * DLI;
      LongTempI += CoefI * DLR;
      EcIndex--;
   }
   ResultPtr->r = (short) ((LongTempR + 32768) >> 16);
   ResultPtr->i = (short) ((LongTempI + 32768) >> 16);
 }
}
// #endif
/* ************************************************************************** */

/* ************************************************************************** */
/* Complex coefficient update to be written in assembly language. */
#if (VX1_PC0 == 1)
void FilterBank::ComplexCoefUpdate(icomplex *NormedErrPtr,
                           icomplex *CoeffPtr,
                           icomplex *DLPtr,
                           int EcIndex)
{
	/*
 if (cCI_mode & 2) {
   complexCoefUpdate5(NormedErrPtr, CoeffPtr, DLPtr, EcIndex, GradingShift);
 } else
 */
{
// Graded updates
/* The coefficients which tend to be small get smaller updates. This speeds
convergence. */
   int *GradingShiftPtr;
   int Temp0, Temp1;
   int DLR, DLI;
   int NormedErrR, NormedErrI;

   GradingShiftPtr = GradingShift;
   NormedErrR = NormedErrPtr->r;
   NormedErrI = NormedErrPtr->i;
   while (EcIndex >= 0) {     // Largest contemplated EcIndex is 22
      Temp0 = *GradingShiftPtr++;
      DLR = DLPtr->r;
      DLI = DLPtr->i;
      DLPtr--;
      Temp1 = (NormedErrR * DLR +
         NormedErrI * DLI) >> Temp0;
      CoeffPtr->r += (Temp1 + 0x8000) >> 16;
      Temp1 = (NormedErrR * DLI -
         NormedErrI * DLR) >> Temp0;
      CoeffPtr->i -= (Temp1 + 0x8000) >> 16;
      CoeffPtr--;
      EcIndex--;
   }
 }
}

#else    // #else for VX1_PC0

void ComplexCoefUpdate(icomplex *NormedErrPtr,
                  icomplex *CoeffPtr,
                  icomplex *DLPtr,
                  int EcIndex)

{
// Graded updates
/* The coefficients which tend to be small get smaller updates. This speeds
convergence. */
   int *GradingShiftPtr;
   int Temp0, Temp1;
   int DLR, DLI;
   int NormedErrR, NormedErrI;

   GradingShiftPtr = GradingShift;
   NormedErrR = NormedErrPtr->r;
   NormedErrI = NormedErrPtr->i;
   while (EcIndex >= 0) {     // Largest contemplated EcIndex is 22
      Temp0 = *GradingShiftPtr++;
      DLR = DLPtr->r;
      DLI = DLPtr->i;
      DLPtr--;
      Temp1 = (NormedErrR * DLR +
         NormedErrI * DLI) >> Temp0;
      CoeffPtr->r += (Temp1 + 0x8000) >> 16;
      Temp1 = (NormedErrR * DLI -
         NormedErrI * DLR) >> Temp0;
      CoeffPtr->i -= (Temp1 + 0x8000) >> 16;
      CoeffPtr--;
      EcIndex--;
   }
}
#endif  // #endif for VX1_PC0

#endif /* __pingtel_on_posix__ ] */

/* ************************************************************************** */
