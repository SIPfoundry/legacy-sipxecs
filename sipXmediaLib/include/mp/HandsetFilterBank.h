//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _HandsetFilterBank_ /* [ */
#define _HandsetFilterBank_

#include "mp/dsplib.h"




class HandsetFilterBank
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   void DoHandsetFilterBank(short *, short *, short *); // (mic_out, mic_in, spkr)
   void HandsetFilterBankInit();
   void HandsetFilterBankReinit();


/* ============================ CREATORS ================================== */

   HandsetFilterBank();
     //:Constructor

virtual
   ~HandsetFilterBank();
     //:Destructor


private:

#define HS_M 8             /* decimation ratio */
#define HS_TWOM (2*HS_M)
#define HS_FRAME_SIZE (80)
#define HS_NUM_FRAMES_PER_10MS 10
#define HS_WINDOWSIZE (HS_M*6)

#define HS_MAX_NUM_TAPS 8

#define HS_LOW_BAND 0
#define HS_HIGH_BAND HS_M
#define HS_NUM_BANDS_PROCESSED ((HS_HIGH_BAND - HS_LOW_BAND) + 1)
#define HS_ECDL_SIZE ((HS_NUM_BANDS_PROCESSED+2)*(HS_MAX_NUM_TAPS+HS_NUM_FRAMES_PER_10MS-1))     // Unnecessarily large


   icomplex EchoCancellerCoef[HS_NUM_BANDS_PROCESSED][HS_MAX_NUM_TAPS];
   icomplex ECDL[HS_ECDL_SIZE];           // DEBUG!!! Determine size later.

   int ECDLHighestAddressBand;
   int ECDLHighestAddressBandNewestIndex;
   int HS_DoubletalkIndicator;     // This should be available for loudspeaker fade
   int HS_LdspkrDLPow[HS_NUM_BANDS_PROCESSED];
   int DelayedMic[HS_FRAME_SIZE + 80]; // for now, 80
   int DelayedLdspkr[HS_FRAME_SIZE + 320+80]; // for now, max sixe of 320
   int MultiFrameMicAnalysisDL[HS_WINDOWSIZE+HS_M*(HS_NUM_FRAMES_PER_10MS-1)];
   int MultiFrameLdspkrAnalysisDL[HS_WINDOWSIZE+HS_M*(HS_NUM_FRAMES_PER_10MS-1)];
   int SynthesisDL[HS_WINDOWSIZE];
   int ThresholdLdspkrDLPow[HS_NUM_BANDS_PROCESSED];

	int HS_EchoDecaydB;

   int HandsetDoubletalkDetection(icomplex [][HS_M+1], icomplex [][HS_M+1], int [], int []);
   void MultiFrameHandsetFilterBankAnalysis(icomplex [][HS_M+1], int *, int *, int *, int);
   void HandsetSubbandECLoop(icomplex [][HS_M+1], icomplex [][HS_M+1], int [], int []);

	void HandsetComputeLoudspeakerFade();
	void HandsetEchoSuppress(icomplex [][HS_M+1],int);


};


#endif  /* _HandsetFilterBank_ ] */
