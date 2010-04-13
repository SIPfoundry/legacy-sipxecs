//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _FilterBank_ /* [ */
#define _FilterBank_

#include "mp/dsplib.h"

#define MAX_NUM_TAPS 40

///////////////////////////  Math functions  ///////////////////////////////
int nrand();
int myrand();

int Get1000log10(int);
int FFTError(int *, int *, int);

void TestDft(); // for testing of 80 point FFT for subband structure
void getdft80tables();
void ComplexInnerProduct(icomplex *, icomplex *, icomplex *, int);
void FFT8CtoC(icomplex *);
void FFT16RtoC(icomplex *);
void FFT16CtoR(icomplex *);
void GetDft40Tables();
void FFT80RtoC(icomplex *);
void FFT80CtoR(icomplex *);
void FFT40CtoC(icomplex *);
void FFT5CtoC(icomplex *);

void FFT16CtoC(icomplex *); // will be used for wideband
void FFT80CtoC(icomplex *); // will be used for wideband

//////////////////////////////////////////////////////////////////////////////

class FilterBank
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


   void DoFilterBank(short *, short *);




/* ============================ CREATORS ================================== */

   FilterBank();
     //:Constructor

virtual
   ~FilterBank();
     //:Destructor


private:

   #define M 40             /* decimation ratio */
   #define LOW_BAND 1
   #define HIGH_BAND 39
   #define NUM_BANDS_PROCESSED ((HIGH_BAND - LOW_BAND) + 1)
   #define ECDL_SIZE (NUM_BANDS_PROCESSED*MAX_NUM_TAPS)     // Unnecessarily large


   icomplex EchoCancellerCoef[NUM_BANDS_PROCESSED][MAX_NUM_TAPS];
   icomplex ECDL[ECDL_SIZE];           // DEBUG!!! Determine size later.


   int DoubletalkDetection(icomplex [][M+1], icomplex [][M+1], int [], int []);
   void ComputeLoudspeakerFade();
   void SubbandECLoop(icomplex [][M+1], icomplex [][M+1], int [], int [], int []);

   void FilterBankInit();
   void FilterBankReinit();

   static void TwoFrameFilterBankAnalysis(icomplex [][M+1], int *, int *, int *, int);
   void EchoSuppress(icomplex [][M+1], int [], int [], int [], int);
   static void ComplexCoefUpdate(icomplex *, icomplex *, icomplex *, int);

};


#endif  /* _FilterBank_ ] */
