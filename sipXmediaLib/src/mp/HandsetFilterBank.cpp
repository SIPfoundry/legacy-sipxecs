//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef __pingtel_on_posix__ /* [ */

#define VX1_PC0 1     // Set to 1 for VXWORKS, 0 for PC simulation

////////////////////////////////// debug defines ////////////////////

#define COMPUTE_BAND_ERL 1

#define PASS_THROUGH_TEST 0
/////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <math.h>



static int i_HS_LoudspeakerFadeDB;
static int i_HS_DoubleTalkIndicatorFlag;

/***************** New for suppression of headset electrical nonlinear echo */
static int i_HS_EchoSup = 1;

static int HS_EchoSuppressionX0 = 20;
static int HS_EchoSuppressionX1 = 40;
static int HS_EchoSuppressionY1 = 12;
static int HS_EchoSuppressionX2 = 55;
static int HS_EchoSuppressionY2 = 25;


static int HS_EchoSuppressionDIThresh = 1000;
static int HS_EchoSuppressionDIAvgConst = 60;
static int HS_EchoSuppressionDISubConst = 32;

static int HS_EchoSuppressionHoldTime = 24;
static int HS_EchoSuppressionMaxDecay = -4096;

int HSupTweak(int X0, int X1, int Y1, int X2, int Y2 )
{
   HS_EchoSuppressionX0 = X0;
   HS_EchoSuppressionX1 = X1;
   HS_EchoSuppressionY1 = Y1;
   HS_EchoSuppressionX2 = X2;
   HS_EchoSuppressionY2 = Y2;

   return(1);
}

int HTimeTweak(int Hold, int Decay)
{
   HS_EchoSuppressionHoldTime = Hold;
   HS_EchoSuppressionMaxDecay = Decay;

   return(1);
}

int HSEchoSup (int iFlag)
{
   int save = i_HS_EchoSup;
   i_HS_EchoSup = iFlag;
   return save;
}



/* ****** Echo suppression table ************************** */
/* This table allows a maximum loss for echo suppression of 42 dB. */
   static int HS_EchoSupMultiplierTable[43] = {
      4096, 3649, 3251, 2896, 2580, 2299,
      2048, 1825, 1625, 1448, 1290, 1149,
      1024,  912,  813,  724,  645,  575,
      512,  456,  406,  362,  323,  287,
      256,  228,  203,  181,  161,  144,
      128,  114,  102,   91,   81,   72,
       64,   57,   51,   45,   40,   36,
       32};

#define HS_MAX_TARGET_ECHO_SUP_DB (40*1024-1)

static int HS_SumWeightedLdspkrPowDBMax;
/* **************** end of New for suppression of headset electrical nonlinear echo */


static int HS_UpdateControl = 1;

#include "mp/HandsetFilterBank.h"
#include "mp/FilterBank.h"

#if (VX1_PC0 == 1)
//#define HS_EXTRA_MIC_DELAY 72      // Probably, this would be 0 in the real-time product
static int ExtraLoudspeakerDelay = 0;
//#include "mp/DSPAEC.h"
#include "mp/DSP_type.h"
#include "mp/MpBuf.h"
#include "mp/dsplib.h"
#include "mp/MpCodec.h"

#else    // #else for VX1_PC0
#define HS_EXTRA_MIC_DELAY (0)      // Probably, this would be 0 in the real-time product
//static int ExtraLoudspeakerDelay = 320;
static int ExtraLoudspeakerDelay = 320+16;
#include <string.h>
extern FILE *fp_stats;
#endif

static int HandsetMicDelay = 32;
int ELD(int iFlag )
{
   int save = ExtraLoudspeakerDelay;
   ExtraLoudspeakerDelay = iFlag;
   return (save);
}

int MicDelay(int iFlag )
{
   int save = HandsetMicDelay;
   HandsetMicDelay = iFlag;
   return (save);
}

// Constructor
HandsetFilterBank::HandsetFilterBank()
{
    this->HandsetFilterBankInit();
    this->HandsetFilterBankReinit();
}

// Destructor
HandsetFilterBank::~HandsetFilterBank()
{
}

#if (VX1_PC0 == 1) //////////////////////////////////////////////////////////

#define ARITHMETIC_TYPE 3

#else    ////////////////////////////////////// #else for VX1_PC0

   int imagsq(icomplex *, int);
   void HandsetFilterBankReport();
   void HandsetFilterBankFinalReport(int);
   void HandsetFilterBank(short *, short *, short *, int);

#define ARITHMETIC_TYPE 1
   void ComplexCoefUpdate(icomplex *, icomplex *, icomplex *, int);

#endif      ///////////////////////  #endif for VX1_PC0


#define L1 (HS_MAX_NUM_TAPS)
   static int ECNumTaps[HS_NUM_BANDS_PROCESSED] = {
       L1,L1,L1,L1,L1,
           L1,L1,L1,L1};

// loudspeaker to mic gain estimates (log scale).
   static int LdspkrMicGain[HS_NUM_BANDS_PROCESSED] = {-30000,-29000,-14000,-7000,1000,2000,6000,0,0};

static int Frame10msCount;
static int DebugUpdateCount[HS_NUM_BANDS_PROCESSED];
#if COMPUTE_BAND_ERL
static int SumFBMicPow[HS_NUM_BANDS_PROCESSED];
static int SumFBResPow[HS_NUM_BANDS_PROCESSED];
#endif

void HandsetFilterBank::HandsetFilterBankReinit()
{
   int i,j;
   int Band;

   for (j=0; j < (HS_FRAME_SIZE + HandsetMicDelay); j++)
      DelayedMic[j] = 0;

   for (j=0; j < (HS_FRAME_SIZE + ExtraLoudspeakerDelay); j++)
      DelayedLdspkr[j] = 0;

   for (j=0; j<HS_WINDOWSIZE+HS_M*(HS_NUM_FRAMES_PER_10MS-1); j++)
       MultiFrameMicAnalysisDL[j] = 0;

   for (j=0; j<HS_WINDOWSIZE+HS_M*(HS_NUM_FRAMES_PER_10MS-1); j++)
       MultiFrameLdspkrAnalysisDL[j] = 0;

   for (j=0; j<HS_WINDOWSIZE; j++)
       SynthesisDL[j] = 0;

   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      HS_LdspkrDLPow[Band] = 0;
   }

#define THRESHOLD_SCALE 8
//#define THRESHOLD_SCALE 1

   ThresholdLdspkrDLPow[0] = THRESHOLD_SCALE*2048; // higher means less frequent update
   for (Band=1; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      ThresholdLdspkrDLPow[Band] = (15*ThresholdLdspkrDLPow[Band-1]) >> 5;
   }


   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      DebugUpdateCount[Band] = 0;
   }


   ECDLHighestAddressBand = HS_NUM_BANDS_PROCESSED-1;
   ECDLHighestAddressBandNewestIndex = -HS_NUM_FRAMES_PER_10MS;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      ECDLHighestAddressBandNewestIndex += ((ECNumTaps[Band]-1)+HS_NUM_FRAMES_PER_10MS);
   }

   for (i=0; i < HS_ECDL_SIZE; i++)
   {
      ECDL[i].r = 0;
      ECDL[i].i = 0;
   }


   for (i=0; i < HS_NUM_BANDS_PROCESSED; i++) {
      for (j=0; j<HS_MAX_NUM_TAPS; j++) {
         EchoCancellerCoef[i][j].r = 0;
         EchoCancellerCoef[i][j].i = 0;
      }
   }


#if COMPUTE_BAND_ERL
   for (i=0; i < HS_NUM_BANDS_PROCESSED; i++) {
      SumFBMicPow[i] = 0;
      SumFBResPow[i] = 0;
   }
#endif

   Frame10msCount = 0;

}



#if (VX1_PC0 == 1)

void HandsetFilterBank::HandsetFilterBankInit()
{

}

#else    // #else for VX1_PC0

static double dTDSummic, dTDSumres;
static double BigTDSummic;
static double BigTDSumres;
static double BigTDSumerl;

static int MaxLdspkrDLPow[HS_NUM_BANDS_PROCESSED];

#if COMPUTE_BAND_ERL
static int MaxSumFBMicPow[HS_NUM_BANDS_PROCESSED];
static int MaxSumFBResPow[HS_NUM_BANDS_PROCESSED];
#endif

void HandsetFilterBank::HandsetFilterBankInit()
{

   TestDft();

   BigTDSummic = 0.0;
   BigTDSumres = 0.0;
   BigTDSumerl = 0.0;

   int Band;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      MaxLdspkrDLPow[Band] = 0;
#if COMPUTE_BAND_ERL
      MaxSumFBMicPow[Band] = 0;
      MaxSumFBResPow[Band] = 0;
#endif
   }

}


void HandsetFilterBankReport()
{
   double erldb;

   printf("dTDSummic = %f   dTDSumres = %f  ",dTDSummic,dTDSumres);
   erldb = 10.0*log10(dTDSummic/dTDSumres);
   printf("fb td ERL = %6.2f\n",erldb);

   BigTDSummic += dTDSummic;
   BigTDSumres += dTDSumres;
   BigTDSumerl += erldb;

#if (1)
   int Band;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++) {
      printf("%4d ",DebugUpdateCount[Band]);
      if (Band%14 == 13) printf("\n");
   }
   printf("\n");
#endif
}

void HandsetFilterBankFinalReport(int total_simulations)
{
   printf("HandsetFilterBank total td ERL = %6.2f avg ERL = %8.2f\n",
      10.0*log10(BigTDSummic/BigTDSumres),
      BigTDSumerl/total_simulations);

#if (0)
   int Band;
   printf("Band: MaxLdspkrDLPow[Band],MaxSumFBMicPow[Band],MaxSumFBResPow[Band]\n");
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      printf("%8x %8x %8x   ",MaxLdspkrDLPow[Band],MaxSumFBMicPow[Band],MaxSumFBResPow[Band]);
      if (Band%2 == 1) printf("\n");
   }
#endif

}
#endif      // #endif for VX1_PC0


/* ********************************************************************* */

void HandsetFilterBank::DoHandsetFilterBank(short fbecmic[], short mic[], short ldspkr[])
{

#include "mp/HandsetFilterbankWindows.h"

    int i,j;
    int FrameNum;

    icomplex MultiFrameMicFFTArray[HS_NUM_FRAMES_PER_10MS][HS_M+1];
    icomplex MultiFrameLdspkrFFTArray[HS_NUM_FRAMES_PER_10MS][HS_M+1];

    int MicDB[HS_NUM_BANDS_PROCESSED];
    int LdspkrDB[HS_NUM_BANDS_PROCESSED];


#if (0)
   int max;
   max = 0;
   for (i=0; i<HS_FRAME_SIZE; i++) {
      if (abs(mic[i]) > max) max = abs(mic[i]);
   }
   printf("mic max = %10.0f ",max);

   max = 0;
   for (i=0; i<HS_FRAME_SIZE; i++) {
      if (abs(ldspkr[i]) > max) max = abs(ldspkr[i]);
   }
   printf("ldspkr max = %10.0f ",max);

   getchar();
#endif



/* DEBUG!!!!This is only for monitoring time domain ERL.
   This can be eliminated in final release. */
    static short micdl[5*HS_M+HS_FRAME_SIZE+200];  // For monitoring time domain ERL
    for (i=0; i<5*HS_M+HandsetMicDelay; i++)
       micdl[i] = micdl[i+HS_FRAME_SIZE];
    for (i=0; i<HS_FRAME_SIZE; i++)
       micdl[i+5*HS_M+HandsetMicDelay] =  mic[i];


// If needed, delay the mic signal.
// DEBUG!!! With memory to spare, let buffer creep so that the
// copying only takes place rarely.
    for (i=0; i < HandsetMicDelay; i++)
      DelayedMic[i] = DelayedMic[i+HS_FRAME_SIZE];
    for (i=0; i<HS_FRAME_SIZE; i++)
       DelayedMic[HandsetMicDelay+i] = mic[i];

//DEBUG!!!
#define TEST_SINUSOID_DEBUG 0
#if (TEST_SINUSOID_DEBUG)
    for (i=0; i<HS_FRAME_SIZE; i++) {
       static double Angle;
       double dTemp0;
       dTemp0 = 1000.0 * sin(6.2831853*Angle);
       Angle += 200./8000.;
       if (dTemp0 >= 0.0) j = (int) (dTemp0 + 0.5);
       else j = (int) (dTemp0 - 0.5);
       DelayedMic[HandsetMicDelay+i] = j;
    }
#endif

// If needed, delay the loudspeaker signal.
// This also copies the samples into an array of 32-bit integers.
    for (i=0; i < ExtraLoudspeakerDelay; i++)
      DelayedLdspkr[i] = DelayedLdspkr[i+HS_FRAME_SIZE];
    for (i=0; i<HS_FRAME_SIZE; i++)
       DelayedLdspkr[i+ExtraLoudspeakerDelay] = ldspkr[i];


    MultiFrameHandsetFilterBankAnalysis(MultiFrameMicFFTArray,
       DelayedMic,MultiFrameMicAnalysisDL,AnalysisWindow, 0); // mic data

// 1/12/2005 DEBUG!!!!!
#if (0)
	osPrintf("ldspkr samples:");
	for (i=0; i<8; i++)
		osPrintf("%6d ",ldspkr[i]);
	osPrintf("\n");
#endif
    MultiFrameHandsetFilterBankAnalysis(MultiFrameLdspkrFFTArray,
       DelayedLdspkr,MultiFrameLdspkrAnalysisDL,LdspkrAnalysisWindow, 1); // spkr data

//DEBUG!!!
#if (TEST_SINUSOID_DEBUG)
    for (FrameNum = 0; FrameNum < 2; FrameNum++) {
       for (i=0; i<6; i++) {
          printf("%5d,%5d  ",MultiFrameMicFFTArray[FrameNum][i].r,MultiFrameMicFFTArray[FrameNum][i].i);
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

#if (PASS_THROUGH_TEST == 0)
    HS_DoubletalkIndicator = HandsetDoubletalkDetection(MultiFrameMicFFTArray,
       MultiFrameLdspkrFFTArray,
       MicDB,
       LdspkrDB);

/* **************** New for suppression of headset electrical nonlinear echo */
#ifdef _VXWORKS /* [ */
    if (i_HS_EchoSup && MpCodec_isHeadsetSpeakerOn())
       HandsetComputeLoudspeakerFade();
#endif /* _VXWORKS ] */
/* **************** end of New for suppression of headset electrical nonlinear echo */

/* Main echo canceller loop for subbands. */
    HandsetSubbandECLoop(MultiFrameMicFFTArray,
       MultiFrameLdspkrFFTArray,
       MicDB,
       LdspkrDB);

/* **************** New for suppression of headset electrical nonlinear echo */
#ifdef _VXWORKS /* [ */
    if (i_HS_EchoSup && MpCodec_isHeadsetSpeakerOn())
       HandsetEchoSuppress(MultiFrameMicFFTArray,HS_DoubletalkIndicator);
#endif /* _VXWORKS ] */
/* **************** end of New for suppression of headset electrical nonlinear echo */

#endif


/* Zero the bands that aren't processed. */
#if (PASS_THROUGH_TEST == 0)

#if (HS_LOW_BAND > 0)
    for (i=0; i < HS_LOW_BAND; i++)
    {
        for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
        {
            MultiFrameMicFFTArray[FrameNum][i].r = 0;
            MultiFrameMicFFTArray[FrameNum][i].i = 0;
        }
    }
#endif

#if (HS_HIGH_BAND < HS_M)
    for (i=HS_HIGH_BAND+1; i <= HS_M; i++)
    {
        for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
        {
            MultiFrameMicFFTArray[FrameNum][i].r = 0;
            MultiFrameMicFFTArray[FrameNum][i].i = 0;
        }
    }
#endif

#endif

/*
    MultiFrameHandsetFilterBankSynthesis(MultiFrameMicFFTArray,
       SynthesisDL,ReconstructionWindow);
*/

// Synthesis of echo cancelled mic signal.

// Every other frame (M samples) invert every other complex band.
#define MODULATION 1    // Necessary, if we do any in-band filtering
#if (MODULATION)
    for (FrameNum = 1; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum += 2)
    {
        for (i=1; i < HS_M; i += 2) {
            MultiFrameMicFFTArray[FrameNum][i].r  = -MultiFrameMicFFTArray[FrameNum][i].r;
            MultiFrameMicFFTArray[FrameNum][i].i  = -MultiFrameMicFFTArray[FrameNum][i].i;
        }
    }
#endif

    for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
    {

        FFT16CtoR(&MultiFrameMicFFTArray[FrameNum][0]);

        int FFTOut[HS_TWOM];

#define FBSHIFT0 1
#if (FBSHIFT0 == 0)
       for (i=0; i < HS_M; i++) {   // Scale down by 1/HS_TWOM
          FFTOut[2*i]  = (MultiFrameMicFFTArray[FrameNum][i].r * 2048 + 16384) >> 15;
          FFTOut[2*i+1]  = (MultiFrameMicFFTArray[FrameNum][i].i * 2048 + 16384) >> 15;
       }
#elif (FBSHIFT0 == 1)
       for (i=0; i < HS_M; i++) {   // Scale down by 2/HS_TWOM
/*          FFTOut[2*i]  = (MultiFrameMicFFTArray[FrameNum][i].r * 4096 + 16384) >> 15;
          FFTOut[2*i+1]  = (MultiFrameMicFFTArray[FrameNum][i].i * 4096 + 16384) >> 15; */
          FFTOut[2*i]  = (MultiFrameMicFFTArray[FrameNum][i].r + 4) >> 3;
          FFTOut[2*i+1]  = (MultiFrameMicFFTArray[FrameNum][i].i + 4) >> 3;
       }
#elif (FBSHIFT0 == 3)
       for (i=0; i < HS_M; i++) {   // Scale down by 8/HS_TWOM
          FFTOut[2*i]  = (MultiFrameMicFFTArray[FrameNum][i].r * 16384 + 16384) >> 15;
          FFTOut[2*i+1]  = (MultiFrameMicFFTArray[FrameNum][i].i * 16384 + 16384) >> 15;
       }
#endif

#define ReconstructionWindow AnalysisWindow
       for (i=0; i<2*HS_M; i++)
          SynthesisDL[i] = SynthesisDL[i+HS_M] + ReconstructionWindow[i]*FFTOut[i];
       j = 0;
       for (i=2*HS_M; i<4*HS_M; i++)
          SynthesisDL[i] = SynthesisDL[i+HS_M] + ReconstructionWindow[i]*FFTOut[j++];
       j = 0;
       for (i=4*HS_M; i<5*HS_M; i++)
          SynthesisDL[i] = SynthesisDL[i+HS_M] + ReconstructionWindow[i]*FFTOut[j++];
       for (i=5*HS_M; i<6*HS_M; i++)
          SynthesisDL[i] = ReconstructionWindow[i]*FFTOut[j++];

       for (i=0; i<HS_M; i++)
           fbecmic[i+FrameNum*HS_M] = (SynthesisDL[i] + 16384) >> 15;
    }




#if (VX1_PC0 == 1)
// for DEBUG!!!!! only monitoring of ERL

#if 1 // Printout for ERL etc.
    int Temp0, Temp1;
    static int TDSummic=0;
    static int TDSumres=0;
    static int DisplayBand = 0;

    for (i=0; i<HS_FRAME_SIZE; i++)
    {
      TDSummic += abs(micdl[i]);
      TDSumres += abs(fbecmic[i]);
    }
    if (Frame10msCount%20 == 19)
    {    // print out ERL every 200 ms
       Temp0 = Get1000log10(TDSummic) - Get1000log10(TDSumres);

       osPrintf("ERL_hand= %2d",Temp0/50);  //ERL printout for VxWorks
       TDSummic = 0;
       TDSumres = 0;

       osPrintf(" DT= %5d", HS_DoubletalkIndicator);

       osPrintf("   %2d:",DisplayBand);

#if COMPUTE_BAND_ERL
       Temp0 = Get1000log10(SumFBMicPow[DisplayBand]) -
          Get1000log10(SumFBResPow[DisplayBand]);
       osPrintf(" %2d",Temp0/100);
#endif

#if COMPUTE_BAND_ERL



       Temp0 = Get1000log10(SumFBResPow[DisplayBand]);
       osPrintf("  M:%2d  ",Temp0/100);

	   osPrintf("L:%6d  ",LdspkrDB[DisplayBand]/1000);

#endif
       SumFBMicPow[DisplayBand] = 0;
       SumFBResPow[DisplayBand] = 0;


       for (j=ECNumTaps[DisplayBand]-1; j >= 0; j--) {
           Temp0 = EchoCancellerCoef[DisplayBand][j].r >> 4;
           Temp1 = EchoCancellerCoef[DisplayBand][j].i >> 4;
           osPrintf(" %2d",Get1000log10(Temp0*Temp0+Temp1*Temp1)/100);
       }

		 osPrintf("  %3d",DebugUpdateCount[DisplayBand]);
       DebugUpdateCount[DisplayBand] = 0;


       osPrintf("\n");
       DisplayBand++;
       if (DisplayBand >= HS_NUM_BANDS_PROCESSED) DisplayBand = 0;

    }
#endif

#endif


#if (VX1_PC0 == 0)

/*   dTDSummic = 0.0;
   dTDSumres = 0.0; */

   for (i=0; i<HS_FRAME_SIZE; i++) {
      dTDSummic += micdl[i]*micdl[i];
      dTDSumres += fbecmic[i]*fbecmic[i];
   }

/* printf("%4d: fb td ERL = %6.2f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   printf("  %2.0f %2.0f\n",10.0*log10(dTDSummic),10.0*log10(dTDSumres)); */

 {
   int Band;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
      if (abs(HS_LdspkrDLPow[Band]) > MaxLdspkrDLPow[Band]) MaxLdspkrDLPow[Band] = abs(HS_LdspkrDLPow[Band]);
#if COMPUTE_BAND_ERL
      if (abs(SumFBMicPow[Band]) > MaxSumFBMicPow[Band]) MaxSumFBMicPow[Band] = abs(SumFBMicPow[Band]);
      if (abs(SumFBResPow[Band]) > MaxSumFBResPow[Band]) MaxSumFBResPow[Band] = abs(SumFBResPow[Band]);
#endif
   }
 }

#if (PASS_THROUGH_TEST)        // For testing HandsetFilterBank reconstruction (without echo cancellation)
    double dSignalPower, dErrorPower;
    double db;
    dSignalPower = 0;
    dErrorPower = 0;
    for (i=0; i<HS_FRAME_SIZE; i++) {
       dSignalPower += micdl[i]*micdl[i];
       dErrorPower += (micdl[i]-fbecmic[i])*(micdl[i]-fbecmic[i]);
    }
//  printf("HandsetFilterBank SNR = %6.3f dB\n",10.0*log10(dSignalPower/dErrorPower));
    printf("%2.0f",db = 10.0*log10(dSignalPower/dErrorPower));
    if (db > 50.0) printf(" ");
    else printf("*");

/*  printf("%2.0f",db = 10.0*log10(dErrorPower));
    if (db < 35.0) printf(" ");
    else printf("#"); */

/*  int MaxVal = 0;
    int MaxIndex = -1;
    for (i=0; i<HS_FRAME_SIZE; i++) {
       if (abs(micdl[i]-fbecmic[i]) > MaxVal)
       {
          MaxVal = abs(micdl[i]-fbecmic[i]);
          MaxIndex = i;
       }
    }
    printf("%2d %2d ",MaxVal, MaxIndex); */


    if (Frame10msCount%10 == 9) printf("\n");

#endif

#if (0)
    printf("\n");
    for (i=0; i<HS_FRAME_SIZE; i++) {
//       printf("%6d %6d %2d  ",micdl[i],fbecmic[i],micdl[i]-fbecmic[i]);
       printf("%6d %6d",micdl[i],fbecmic[i]);
       if (i%4 == 3) printf("\n");
    }
    getchar();
#endif


/*  printf("fb ec output samples:\n");
    for (i=0; i<HS_FRAME_SIZE; i++) {
       printf("%8.2f ",fbecmic[i]);
       if (i%8 == 7) printf("\n");
    } */
    if (Frame10msCount%10 == 9) {

#if (0)
       for (i=0; i<=M; i++) {
          printf("%7.0f ",HS_LdspkrDLPow[i]/65536.0);
          if (i%6 == 5) printf("\n");
       }
       printf("\n");
#endif

#if (1)
   printf("%4d: fb td ERL = %6.2f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   fprintf(fp_stats,"%4d: fb td ERL = %6.2f",Frame10msCount, 10.0*log10(dTDSummic/dTDSumres));
   printf("  %6.2f %6.2f  ",10.0*log10(dTDSummic),10.0*log10(dTDSumres));
   fprintf(fp_stats,"  %6.2f %6.2f  ",10.0*log10(dTDSummic),10.0*log10(dTDSumres));
   dTDSummic = 0.0;
   dTDSumres = 0.0;

   printf("%5d ",HS_DoubletalkIndicator);
   fprintf(fp_stats,"%5d ",HS_DoubletalkIndicator);
   printf("\n");
   fprintf(fp_stats,"\n");

  {
    int Band;
    for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
    {
       printf("%2d: ",Band);
       fprintf(fp_stats,"%2d: ",Band);

       int Temp0;
#if COMPUTE_BAND_ERL
       printf("%5.2f ",10.0*log10((double)SumFBMicPow[Band]/(double)SumFBResPow[Band]));
       fprintf(fp_stats,"%5.2f ",10.0*log10((double)SumFBMicPow[Band]/(double)SumFBResPow[Band]));
#if 0
       {
          int powdbtimes100,errdbtimes100;
          powdbtimes100 = Get1000log10(SumFBMicPow[Band]);
          errdbtimes100 = Get1000log10(SumFBResPow[Band]);
          Temp0 = (powdbtimes100 - errdbtimes100)/100;
          printf("%2d   ",Temp0);
          fprintf(fp_stats,"%2d   ",Temp0);
       }
#endif

       Temp0 = Get1000log10(SumFBMicPow[Band]);
       printf("%2d ",Temp0/100);
       fprintf(fp_stats,"%2d ",Temp0/100);

       Temp0 = Get1000log10(SumFBResPow[Band]);
       printf("%2d ",Temp0/100);
       fprintf(fp_stats,"%2d ",Temp0/100);

       SumFBMicPow[Band] = 0;
       SumFBResPow[Band] = 0;

#endif

       printf("%3d ",LdspkrMicGain[Band]/1000);
       fprintf(fp_stats,"%3d ",LdspkrMicGain[Band]/1000);

       printf("  %2d ",LdspkrDB[Band]/1000);
       fprintf(fp_stats,"  %2d ",LdspkrDB[Band]/1000);

       Temp0 = Get1000log10(HS_LdspkrDLPow[Band]);
       printf("%2d ",Temp0/100);
       fprintf(fp_stats,"%2d ",Temp0/100);

       Temp0 = Get1000log10(ThresholdLdspkrDLPow[Band]);
       printf("%2d ",Temp0/100);
       fprintf(fp_stats,"%2d ",Temp0/100);


       printf("%3d ",DebugUpdateCount[Band]);
       fprintf(fp_stats,"%3d ",DebugUpdateCount[Band]);
       DebugUpdateCount[Band] = 0;

       printf("\n");
       fprintf(fp_stats,"\n");

    }
  }
 // getchar();
#endif

#if (1)
    if (Frame10msCount%10 == 9)
    {
        int Temp0, Temp1;
        int Band;
        for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
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
}               // end of HandsetFilterBank::DoHandsetFilterBank()



// Window, overlap and add for HS_NUM_FRAMES_PER_10MS subframes. Then DFT each of the subframes.
void HandsetFilterBank::MultiFrameHandsetFilterBankAnalysis(icomplex outFFTArray[][HS_M+1],
                                                            int Input[],
                                                            int AnalysisDL[],
                                                            int AnalysisWindow[],
                                                            int iMicOrSpkrFlag)
{
   int i;
   int FrameNum;
   union IntComplexArray {
      icomplex FFTArray[HS_NUM_FRAMES_PER_10MS][HS_M+1];
      int FFTRealIn[HS_NUM_FRAMES_PER_10MS][2*(HS_M+1)];
   } WorkArray;

// Sequence input delay line for an entire 10 ms frame.
// DEBUG!!! With memory to spare, let the buffer creep so that the
// copying only takes place rarely.
/* for (i=0; i<(HS_WINDOWSIZE-HS_M); i++)
      AnalysisDL[i] = AnalysisDL[i+HS_FRAME_SIZE]; */
   memcpy(AnalysisDL,&AnalysisDL[HS_FRAME_SIZE], sizeof (int) * (HS_WINDOWSIZE-HS_M));
/* for (i=0; i < HS_FRAME_SIZE; i++)
      AnalysisDL[i+(HS_WINDOWSIZE-HS_M)] = Input[i]; */
   memcpy(&AnalysisDL[HS_WINDOWSIZE-HS_M],Input, sizeof (int) * HS_FRAME_SIZE);


   int Temp0, Temp1, Temp2;
   for (i=0; i < HS_TWOM; i++)
   {
      Temp0 = AnalysisWindow[i];
      Temp1 = AnalysisWindow[i+HS_TWOM];
      Temp2 = AnalysisWindow[i+2*HS_TWOM];

      for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
      {
// each frame: window, overlap and add.
          WorkArray.FFTRealIn[FrameNum][i] = (Temp0*AnalysisDL[i+FrameNum*HS_M] +
              Temp1*AnalysisDL[i+HS_TWOM+FrameNum*HS_M] +
              Temp2*AnalysisDL[i+2*HS_TWOM+FrameNum*HS_M] + 16384) >> 15;
      }
   }

/* For each frame do a HS_TWOM-point real to M (M+1) complex DFT. */
   for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
   {
       FFT16RtoC(&WorkArray.FFTArray[FrameNum][0]);
   }

/* Every other frame (M samples) invert every other complex band.
This corresponds to modulation by e**(-j*2*PI*i*FrameNum*M/HS_TWOM).
We can interpret this as making continuous the DFT modulation carrier waves. */
#if (MODULATION)
   for (FrameNum = 1; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum += 2)
   {
       for (i=1; i < HS_M; i += 2) {
           WorkArray.FFTArray[FrameNum][i].r = -WorkArray.FFTArray[FrameNum][i].r;
           WorkArray.FFTArray[FrameNum][i].i = -WorkArray.FFTArray[FrameNum][i].i;
       }
   }
#endif


// Make sure imaginary parts of bands 0 and M are zeroed.
   for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
   {
       WorkArray.FFTArray[FrameNum][0].i = 0;
       WorkArray.FFTArray[FrameNum][HS_M].i = 0;
   }

   for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++) {
      for (i = 0; i <= HS_M; i++) {
#if (FBSHIFT0 == 0)
         outFFTArray[FrameNum][i].r = WorkArray.FFTArray[FrameNum][i].r;
         outFFTArray[FrameNum][i].i = WorkArray.FFTArray[FrameNum][i].i;
#elif (FBSHIFT0 == 1)
         outFFTArray[FrameNum][i].r = (WorkArray.FFTArray[FrameNum][i].r) >> 1;
         outFFTArray[FrameNum][i].i = (WorkArray.FFTArray[FrameNum][i].i) >> 1;
#elif (FBSHIFT0 == 3)
         outFFTArray[FrameNum][i].r = (WorkArray.FFTArray[FrameNum][i].r + 4) >> 3;
         outFFTArray[FrameNum][i].i = (WorkArray.FFTArray[FrameNum][i].i + 4) >> 3;
#endif
      }
   }


}


/* ************************************************************************** */
/* *********************** Doubletalk Detection ***************************** */
/* ************************************************************************** */

int HandsetFilterBank::HandsetDoubletalkDetection(icomplex MultiFrameMicFFTArray[][HS_M+1],
                                                  icomplex MultiFrameLdspkrFFTArray[][HS_M+1],
                                                  int MicDB[],
                                                  int LdspkrDB[])
{
   int Band;
   int Temp0,Temp1,Temp2;
   int FrameNum;


   /* Compute mic power and weighted loudspeaker power for each band, and
   update leaky average of raw mic power. */
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {

   /* Compute this frame's mic power for doubletalk detection and leaky integrated
      mic power for computing ERL in band. */

#if COMPUTE_BAND_ERL
//      SumFBMicPow[Band] -= (SumFBMicPow[Band] >> 5);
#endif

      Temp2 = 0;
      for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
      {
          Temp2 += imagsq(&(MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND]), MIC_GAIN_SHIFT);
      }

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

   }


   int ECDLBandNewestIndex;
   int BandCount;

   ECDLHighestAddressBandNewestIndex += HS_NUM_FRAMES_PER_10MS;

   Band = ECDLHighestAddressBand;
   ECDLBandNewestIndex = ECDLHighestAddressBandNewestIndex;
   for (BandCount=0; BandCount < HS_NUM_BANDS_PROCESSED; BandCount++) {

/* Copy the new complex loudspeaker samples to the delay line (of complex loudspeaker samples)
for an entire 10 ms frame. */
// Use a single complex pole filter to boost the band edges and make the reference signal whiter.
#define POLE_CONST 6    // MODULATION must be turned on for this to work.
       for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
       {
           ECDL[ECDLBandNewestIndex+FrameNum].r = MultiFrameLdspkrFFTArray[FrameNum][Band + HS_LOW_BAND].r -
               ((POLE_CONST*ECDL[ECDLBandNewestIndex+FrameNum-1].r + 8) >> 4);
           ECDL[ECDLBandNewestIndex+FrameNum].i = MultiFrameLdspkrFFTArray[FrameNum][Band + HS_LOW_BAND].i -
               ((POLE_CONST*ECDL[ECDLBandNewestIndex+FrameNum-1].i + 8) >> 4);
       }

/* Use the recursively computed power in the loudspeaker DL to initialize the sum of powers.
Then add in the powers of the new samples. */
       Temp2 = HS_LdspkrDLPow[Band];
#define SHIFT2 (4)
       for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
       {
           Temp2 += imagsq(&(ECDL[ECDLBandNewestIndex+FrameNum]), SHIFT2);
       }

       Temp0 = 10*Get1000log10(Temp2);
       LdspkrDB[Band] = Temp0;

/* Decrement the Band and adjust the pointer to the newest sample in ECDL[] for Band. */
       ECDLBandNewestIndex -= ((ECNumTaps[Band] - 1)+HS_NUM_FRAMES_PER_10MS);
       Band--;
       if (Band < 0) Band += HS_NUM_BANDS_PROCESSED;
    }



   Temp0 = 0;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++) {
      Temp1 = MicDB[Band] - (LdspkrDB[Band] + LdspkrMicGain[Band] + 9000);  // Was 9000 before 4/23
      if (Temp1 > 0)
         Temp0 += Temp1;
   }

   int HS_DoubletalkIndicator;
   HS_DoubletalkIndicator  = Temp0/HS_NUM_BANDS_PROCESSED;

   return(HS_DoubletalkIndicator);     // returns HS_DoubletalkIndicator
}
/* ************************************************************************** */
/* ******************** End of Doubletalk Detection ************************* */
/* ************************************************************************** */



/* ************************************************************************** */
/* ************* Main echo canceller subband processing function **************** */
/* ************************************************************************** */
void HandsetFilterBank::HandsetSubbandECLoop(icomplex MultiFrameMicFFTArray[][HS_M+1],
                                             icomplex MultiFrameLdspkrFFTArray[][HS_M+1],
                                             int MicDB[],
                                             int LdspkrDB[])
{
	int Band;
	int Temp0, Temp1, Temp2;
	int FrameNum;

	/* Do echo cancellation and update for each of the HS_NUM_BANDS_PROCESSED bands */
   int EcIndex;
   int DLIndex;

   int ECDLBandNewestIndex;
   int BandCount;

	Band = ECDLHighestAddressBand;
   ECDLBandNewestIndex = ECDLHighestAddressBandNewestIndex;

   for (BandCount=0; BandCount < HS_NUM_BANDS_PROCESSED; BandCount++)
	{
		for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
		{

// Recursively compute power in loudspeaker delay line for Band.
// Add the newest sample power to the current value in HS_LdspkrDLPow[Band].
			DLIndex = ECDLBandNewestIndex + FrameNum;
			HS_LdspkrDLPow[Band] += imagsq(&(ECDL[DLIndex]), SHIFT2);

// DEBUG!!!!! Bands 0 and 8 are all real, so could have special function for those bands.
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
				MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r -= (Temp0 + 32768) >> SHIFT1;
				MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i -= (Temp1 + 32768) >> SHIFT1;
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

#if (0)
			{
             static double MAXdAcc;
             if (fabs(dAccR) > MAXdAcc) {
                MAXdAcc = fabs(dAccR);
                printf("MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
             }
             if (fabs(dAccI) > MAXdAcc)  {
                MAXdAcc = fabs(dAccI);
                printf("MAXdAcc = %8d\n", (int) (MAXdAcc/65536.0));
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

//        MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r -= (int) (dAccR/65536.0);
//        MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i -= (int) (dAccI/65536.0);
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r -= Temp0;
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i -= Temp1;
#elif (ARITHMETIC_TYPE == 3)
			EcIndex = ECNumTaps[Band]-1;
			DLIndex = ECDLBandNewestIndex + FrameNum;
			icomplex cTemp;
			ComplexInnerProduct(&cTemp, &EchoCancellerCoef[Band][EcIndex], &ECDL[DLIndex], EcIndex);
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r -= cTemp.r;
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i -= cTemp.i;
#endif

			if (HS_UpdateControl)
			{
//             if ((HS_DoubletalkIndicator < 3000) &&
				if ((HS_DoubletalkIndicator < 7000) &&			// Changed Oct 16
					(HS_LdspkrDLPow[Band] > ThresholdLdspkrDLPow[Band]) &&
					(MicDB[Band] < (LdspkrDB[Band] + LdspkrMicGain[Band] + 10240)))  // Use HS_LdspkrDLPow[Band] in dB
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
//             Temp0 = (0xff000000)/(HS_LdspkrDLPow[Band]+(4*ThresholdLdspkrDLPow[Band])); //upconst=1
					Temp0 = (0x7f000000)/(HS_LdspkrDLPow[Band]+(8*ThresholdLdspkrDLPow[Band])); //upconst=1/2
					NormedErr.r = (Temp0 * MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r + 128) >> SHIFT3;
					NormedErr.i = (Temp0 * MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i + 128) >> SHIFT3;
					EcIndex = ECNumTaps[Band]-1;
					DLIndex = ECDLBandNewestIndex + FrameNum;

 // Graded updates
/* The coefficients which tend to be small get smaller updates. This speeds convergence. */
//             static int GradingShift[HS_MAX_NUM_TAPS] = {2,1,0,0,0,1,2};
					static int GradingShift[HS_MAX_NUM_TAPS] = {2,1,0,0,0,0,1,2};
					int *GradingShiftPtr;
					GradingShiftPtr = GradingShift;
					while (EcIndex >= 0)
					{
						Temp0 = *GradingShiftPtr++;
						Temp1 = (NormedErr.r * ECDL[DLIndex].r +
							NormedErr.i * ECDL[DLIndex].i) >> Temp0;
						EchoCancellerCoef[Band][EcIndex].r += (Temp1 + 0x8000) >> 16;
						Temp1 = (NormedErr.r * ECDL[DLIndex].i -
							NormedErr.i * ECDL[DLIndex].r) >> Temp0;
						EchoCancellerCoef[Band][EcIndex].i -= (Temp1 + 0x8000) >> 16;
						EcIndex--;
						DLIndex--;
					}
				}
			}

/* Recursively compute power in loudspeaker delay line for Band.
Subtract the oldest sample's power from the current value in HS_LdspkrDLPow[Band]. */
			DLIndex = ECDLBandNewestIndex + FrameNum - (ECNumTaps[Band]-1);
			HS_LdspkrDLPow[Band] -= imagsq(&(ECDL[DLIndex]), SHIFT2);
		}


/* Compute the residual power for Band */    // DEBUG!!! For now, this isn't used later.
#if COMPUTE_BAND_ERL
//		SumFBResPow[Band] -= (SumFBResPow[Band] >> 5);

		Temp2 = 0;
		for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
		{
			Temp2 += imagsq(&(MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND]), MIC_GAIN_SHIFT);
		}

		SumFBResPow[Band] += (Temp2 + 2) >> 2;
#endif


/* Decrement the Band and adjust the pointer to the newest sample in ECDL[] for Band. */
       ECDLBandNewestIndex -= ((ECNumTaps[Band] - 1)+HS_NUM_FRAMES_PER_10MS);

       Band--;
       if (Band < 0) Band += HS_NUM_BANDS_PROCESSED;
    }    // This is the  actual end of the compute-intensive filterbank signal processing loop
         // for in-band processing.

#if (0)
   printf("ECDLBandNewestIndex=%2d\n",ECDLBandNewestIndex);
   getchar();
#endif


/* If there is enough free space at the lowest address end of ECDL[], then copy the highest adresss
band's delay line values to the free space.
If (ECNumTaps[]-1) is less than HS_NUM_FRAMES_PER_10MS, then we may copy several band's delay lines. */
    while ((ECDLBandNewestIndex - (ECNumTaps[ECDLHighestAddressBand] - 3)) >= 0)
    {
       icomplex *DestPtr;
       icomplex *SourcePtr;
       DestPtr = &ECDL[(ECDLBandNewestIndex+(HS_NUM_FRAMES_PER_10MS-1)) -
           (ECNumTaps[ECDLHighestAddressBand]-2)];
       SourcePtr = &ECDL[(ECDLHighestAddressBandNewestIndex+(HS_NUM_FRAMES_PER_10MS-1)) -
           (ECNumTaps[ECDLHighestAddressBand]-2)];
       memcpy(DestPtr, SourcePtr, (sizeof (icomplex)) * (ECNumTaps[ECDLHighestAddressBand]-1));

       ECDLHighestAddressBandNewestIndex -= ((ECNumTaps[ECDLHighestAddressBand]-1)+HS_NUM_FRAMES_PER_10MS);

#define RECOMPUTE_POWER_FLAG 1      // DEBUG!!! Eventually eliminate this.
       int RecomputeDLPowBand;
       RecomputeDLPowBand = ECDLHighestAddressBand;   // Store index of new lowest address band.

       ECDLHighestAddressBand--;
       if (ECDLHighestAddressBand < 0) ECDLHighestAddressBand += HS_NUM_BANDS_PROCESSED;


#if RECOMPUTE_POWER_FLAG
/* Recompute nonrecursively the delay line power for the new lowest address band.
Otherwise, some error in the computation would persist indefinitely, and it might be difficult
to detect.
DEBUG!!!! This should be unnecessary. */
       EcIndex = ECNumTaps[RecomputeDLPowBand]-2;     // We don't include the oldest sample.
       DLIndex = ECDLBandNewestIndex+(HS_NUM_FRAMES_PER_10MS-1);
       Temp2 = 0;
       while (EcIndex >= 0) {
          Temp2 += imagsq(&(ECDL[DLIndex]), SHIFT2);
          EcIndex--;
          DLIndex--;
       }

#if (VX1_PC0 == 0)
       if (Temp2 != HS_LdspkrDLPow[RecomputeDLPowBand])
          printf("Error!!! Temp2=%d HS_LdspkrDLPow[%d]=%d \n",Temp2,RecomputeDLPowBand,HS_LdspkrDLPow[RecomputeDLPowBand]);
#endif
       HS_LdspkrDLPow[RecomputeDLPowBand] = Temp2;
#endif

/* Decrement the Band and adjust the pointer to the newest sample in ECDL[] for Band. */
       ECDLBandNewestIndex -= ((ECNumTaps[RecomputeDLPowBand] - 1)+HS_NUM_FRAMES_PER_10MS);
    }


}
/* ************************************************************************** */
/* ******** End of Main echo canceller subband processing function ********** */
/* ************************************************************************** */


/* **************** New for suppression of headset electrical nonlinear echo */
/* ************************************************************************** */
/* *********************** Compute Loudspeaker Fade********* **************** */
/* ************************************************************************** */
void HandsetFilterBank::HandsetComputeLoudspeakerFade()
{

// There may be a similar or identical calculation in HandsetEchoSuppress();
   static int LoudspeakerFadeDIThresh = 1000;
   static int LoudspeakerFadeDILeakConst = 60;
   static int LoudspeakerFadeDIMultiplier = 12;
   static int MaxLoudspeakerFade = 6;
   static int LoudspeakerFadeMicPowThresh = 20000;          // unit is raw power, not dB
   static int LoudspeakerFadeLdspkrPowThreshdB = 43000;        // unit is dB times 1000

   static int LoudspeakerFadeLeakyMaxDoubletalkIndicator = 0;

   int Temp0,Temp1;
   int Band;
//   int TargetLoudspeakerFadeDB;


/* Compute sum of weighted loudspeaker signal power for frequencies above and below
   2500 hz. Then take the maximum of the two frequency ranges. */

   Temp0 = 0;
   for (Band=0; Band < 5; Band++)
   {
       Temp0 += HS_LdspkrDLPow[Band] >> 4;
   }

   // Stronger weighting since high frequency sounds don't have to be as loud
   // to produce audible echo.
   Temp1 = 0;
   for (Band=5; Band < HS_NUM_BANDS_PROCESSED; Band++)
   {
       Temp1 += HS_LdspkrDLPow[Band] >> 1;
   }

   if (Temp1 > Temp0) Temp0 = Temp1;
   HS_SumWeightedLdspkrPowDBMax = 10*Get1000log10(Temp0);


// DEBUG!!!! for now there is no loudspeaker fade here.
}

/* ************************************************************************** */
/* ************************* Echo Suppression ******************************* */
/* ************************************************************************** */
void HandsetFilterBank::HandsetEchoSuppress(icomplex MultiFrameMicFFTArray[][HS_M+1],
														  int HS_DoubletalkIndicator)
{
   int Band;
	int FrameNum;
   int i;
   int Temp0, Temp1;
   int TargetEchoSupDB;


#define ESUP_SHIFT 8
   int HS_EchoSuppressionS1;
   int HS_EchoSuppressionS2;
   HS_EchoSuppressionS1 = (HS_EchoSuppressionY1 << ESUP_SHIFT)/
      (HS_EchoSuppressionX1 - HS_EchoSuppressionX0);
   HS_EchoSuppressionS2 = ((HS_EchoSuppressionY2-HS_EchoSuppressionY1) <<
      ESUP_SHIFT)/(HS_EchoSuppressionX2 - HS_EchoSuppressionX1);
   if (HS_SumWeightedLdspkrPowDBMax < HS_EchoSuppressionX0*1024)
      TargetEchoSupDB = 0;
   else if (HS_SumWeightedLdspkrPowDBMax < HS_EchoSuppressionX1*1024)
   {
       TargetEchoSupDB = ((HS_SumWeightedLdspkrPowDBMax - HS_EchoSuppressionX0*1024) *
          HS_EchoSuppressionS1) >> ESUP_SHIFT;
   }
   else
   {
       TargetEchoSupDB = HS_EchoSuppressionY1*1024 +
          (((HS_SumWeightedLdspkrPowDBMax - HS_EchoSuppressionX1*1024) *
          HS_EchoSuppressionS2) >> ESUP_SHIFT);
   }
   if (TargetEchoSupDB > HS_MAX_TARGET_ECHO_SUP_DB)
      TargetEchoSupDB = HS_MAX_TARGET_ECHO_SUP_DB;


#if (VX1_PC0 == 1)
#if 0
   if (Frame10msCount%10 == 9)
   {
       osPrintf("SLP=%2d TES=%2d \n",HS_SumWeightedLdspkrPowDBMax >>10,TargetEchoSupDB >> 10);
   }
#endif
#endif



#if (VX1_PC0 == 0)
   printf(" %5d",HS_SumWeightedLdspkrPowDBMax);
   fprintf(fp_stats,"  %5d",HS_SumWeightedLdspkrPowDBMax);
   printf(" %5d ",TargetEchoSupDB);
   fprintf(fp_stats," %5d ",TargetEchoSupDB);
#endif


// Reduce echo suppression if there is doubletalk.
   static int LeakyMaxDoubletalkIndicator = 0;

   Temp0 = HS_DoubletalkIndicator - HS_EchoSuppressionDIThresh;
/*
   if (SumMicPow < 20000)     // Ignore HS_DoubletalkIndicator if mic power is below threshold.
   {
       Temp0 = 0;
   }
*/
   if (Temp0 > 8000) Temp0 = 8000;
   LeakyMaxDoubletalkIndicator = (HS_EchoSuppressionDIAvgConst*LeakyMaxDoubletalkIndicator) >> 6;
   if (Temp0 > LeakyMaxDoubletalkIndicator) LeakyMaxDoubletalkIndicator = Temp0;

   TargetEchoSupDB -= (HS_EchoSuppressionDISubConst*LeakyMaxDoubletalkIndicator) >> 4;
   if (TargetEchoSupDB < 0) TargetEchoSupDB = 0;
   if (TargetEchoSupDB > HS_MAX_TARGET_ECHO_SUP_DB) TargetEchoSupDB = HS_MAX_TARGET_ECHO_SUP_DB;



#if (VX1_PC0 == 0)
   Temp0 = 0;
   for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
      Temp0 += LdspkrMicGain[Band];
   Temp0 /= HS_NUM_BANDS_PROCESSED;
   printf("  %5d",Temp0);
   fprintf(fp_stats,"  %5d",Temp0);

   printf("  %5d",HS_DoubletalkIndicator);
   fprintf(fp_stats,"  %5d",HS_DoubletalkIndicator);

   if (HS_DoubletalkIndicator >= 1000) {
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
   static int HS_EchoSupDB =0;
   for (i=99; i > 0; i--)
       TargetHistory[i] = TargetHistory[i-1];
   TargetHistory[0] = TargetEchoSupDB;


   Temp1 = 0;
   for (i = 0; i < HS_EchoSuppressionHoldTime; i++)
   {
       if (TargetHistory[i] > Temp1) Temp1 = TargetHistory[i];
   }


// temporal smoothing
//   Temp0 = TargetEchoSupDB - HS_EchoSupDB;
   Temp0 = Temp1 - HS_EchoSupDB;


   //HS_EchoSuppressionMaxDecay = (-8 * HS_EchoDecaydB) >> 4;
   HS_EchoSuppressionMaxDecay = -2048; // 2dB per frame max step


   if (Temp0 < HS_EchoSuppressionMaxDecay) Temp0 = HS_EchoSuppressionMaxDecay;
   HS_EchoSupDB += Temp0;


#if (VX1_PC0 == 1)
#if 0
   if (Frame10msCount%10 == 9)
   {
       osPrintf("HS_EchoSupDB>>10=%4d  HS_ESMDecay>>10=%4d\n",HS_EchoSupDB >>10, HS_EchoSuppressionMaxDecay>>10);
   }
#endif
#endif

	Temp0 = HS_EchoSupDB >> 10;

   if (Temp0 > 43)
   {
      osPrintf("************* Temp0 = %d ***************\n", Temp0);
   }

	Temp1 = HS_EchoSupMultiplierTable[Temp0];

	for (Band=0; Band < HS_NUM_BANDS_PROCESSED; Band++)
	{
		for (FrameNum = 0; FrameNum < HS_NUM_FRAMES_PER_10MS; FrameNum++)
		{
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r =
				(Temp1 * MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].r + 2048) >> 12;
			MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i =
				(Temp1 * MultiFrameMicFFTArray[FrameNum][Band + HS_LOW_BAND].i + 2048) >> 12;
		}
	}


#if (VX1_PC0 == 0)
   printf("   %3d\n",HS_EchoSupDB >> 10);
   fprintf(fp_stats,"   %3d\n",HS_EchoSupDB >> 10);
#endif

}
/* ************************************************************************** */
/* ********************* End of Echo Suppression **************************** */
/* ************************************************************************** */
/* **************** end of New for suppression of headset electrical nonlinear echo */



#if (1)
/* ************************************************************************** */
// Sum of squares function
//#if (VX1_PC0 == 0)

int imagsq(icomplex *ptr, int shift)
{
   int ComplexPower;
   int Temp0, Temp1;

   //printf("here ");

   Temp0 = (ptr->r) >> shift;
   Temp1 = (ptr->i) >> shift;
   ComplexPower = Temp0*Temp0 + Temp1*Temp1;
   return(ComplexPower);
}

//#endif


/* ************************************************************************** */
extern "C" {
extern void complexInnerProduct(icomplex *ResultPtr,
                            icomplex *CoeffPtr, icomplex *DLPtr, int EcIndex);

extern void complexCoefUpdate(icomplex *NormedErrPtr,
                            icomplex *CoeffPtr, icomplex *DLPtr, int EcIndex);
extern int chooseComplexImplementation(int flag);
extern int cCI(int flag);
}

#if 0
static int cCI_mode = 1;
int chooseComplexImplementation(int flag)
{
   int save = cCI_mode;
   cCI_mode = flag & 3;
   return save;
}
int cCI(int flag) {return chooseComplexImplementation(flag);}
#endif
/* ************************************************************************** */
/* Complex inner product to be written in assembly language. */
#if (ARITHMETIC_TYPE == 3)



#endif
/* ************************************************************************** */


#endif /* __pingtel_on_posix__ ] */
/* ************************************************************************** */
#endif
