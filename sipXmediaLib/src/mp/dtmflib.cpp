//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mp/MpTypes.h"
#include "mp/MpMisc.h"
#include "os/OsMutex.h"

#define PI  3.1415926
#define PIt2 (2.0 * PI)
#define TwoPI PIt2

#define DTMF_KEY_ROW_AMP (.3887F * .25F)
#define DTMF_KEY_COL_AMP (.5490F * .25F)

#include "mp/dtmflib.h"

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif
#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

typedef struct __tone_tag {
        int Hz;
        int costh;
        int sinth;
        int sinm1;
        int sinm2;
} tone, *tonePtr;

typedef struct __tonepair_tag {
        tonePtr low;
        tonePtr high;
        int usecs; /* length of tone pair */
        int curusecs; /* portion played so far */
} tonePair, *tonePairPtr;

#define MAX_DTMFS 6
typedef struct __dtmf_tag {
        tonePair pairs[MAX_DTMFS];
        int curpair;
        int repeating;
} dtmfPattern, *dtmfPatternPtr;

typedef struct __MpToneGen_tag {
    int samplerate; /* samples per second */
    int usecspersample;
    OsMutex* mpMutex;
    short shCurrToneID;
    dtmfPatternPtr currentDtmfSound;
    tone st350, st400, st425, st440, st450, st480, st620, st697, st770, st852,
        st941, st1209h, st1209l, st1336, st1477h, st1477l, st1633, misc,
        st480Loud, st620Loud;
    dtmfPattern key1, key2, key3, keya, key4, key5, key6, keyb, key7,
        key8, key9, keyc, keystar, key0, keysharp, keyd, dial, busy, ringback,
        ringtone, callfailed, silence, callwait, backspace, callheld,
        loudFastBusy;
} MpToneGen;

static int sineWaveHz = 0;

#undef DEBUG_DTMF

#undef DTMF_LOUD_BUSY_AMP
#define DTMF_LOUD_BUSY_AMP (.95F)

// To allow experiments with loudness of loud fast busy, undefine
// the DTMF_LOUD_BUSY_AMP macro, and the code below will make it
// adjustable from the console with the setLoudness(0..100) command.

#ifndef DTMF_LOUD_BUSY_AMP /* [ */
extern "C" {
extern int setLoudness(int pct);
}

static int iLoudness = 95;
static float fLoudness = .95F;
int setLoudness(int pct)
{
   int save = iLoudness;
   if (pct > 100) pct = 100;
   if (pct > 0) {
      iLoudness = pct;
      fLoudness = pct;
      fLoudness = fLoudness / 100.;
   }
   return save;
}
#define DTMF_LOUD_BUSY_AMP fLoudness
#endif /* !DTMF_LOUD_BUSY_AMP ] */

static void startDtmf(MpToneGenPtr p, dtmfPatternPtr d)
{
        int i;

        p->mpMutex->acquire();
        if (NULL != d) {
            d->curpair = 0;
            for (i=0; i<MAX_DTMFS; i++) {
                d->pairs[i].curusecs = 0;
            }
        }
        p->currentDtmfSound = d;
        p->mpMutex->release();
}

void MpToneGen_startTone(MpToneGenPtr p, int toneID)
{
        dtmfPatternPtr pNew = NULL;
        STATUS ret = OS_SUCCESS;

        switch (toneID) {
            case '1': case  1 : pNew = &(p->key1); break;
            case '2': case  2 : pNew = &(p->key2); break;
            case '3': case  3 : pNew = &(p->key3); break;
            case 'a': case 'A': pNew = &(p->keya); break;
            case '4': case  4 : pNew = &(p->key4); break;
            case '5': case  5 : pNew = &(p->key5); break;
            case '6': case  6 : pNew = &(p->key6); break;
            case 'b': case 'B': pNew = &(p->keyb); break;
            case '7': case  7 : pNew = &(p->key7); break;
            case '8': case  8 : pNew = &(p->key8); break;
            case '9': case  9 : pNew = &(p->key9); break;
            case 'c': case 'C': pNew = &(p->keyc); break;
            case '*':           pNew = &(p->keystar); break;
            case '0': case  0 : pNew = &(p->key0); break;
            case '#':           pNew = &(p->keysharp); break;
            case 'd': case 'D': pNew = &(p->keyd); break;

            case DTMF_TONE_DIALTONE: pNew = &(p->dial); break;
            case DTMF_TONE_SILENCE: pNew = &(p->silence); break;


            case DTMF_TONE_BUSY: pNew = &(p->busy); break;

            case DTMF_TONE_RINGBACK: pNew = &(p->ringback); break;
            case DTMF_TONE_RINGTONE: pNew = &(p->ringtone); break;
            case DTMF_TONE_CALLFAILED: pNew = &(p->callfailed); break;
            case DTMF_TONE_BACKSPACE: pNew = &(p->backspace); break;
            case DTMF_TONE_CALLWAITING: pNew = &(p->callwait); break;
            case DTMF_TONE_CALLHELD: pNew = &(p->callheld); break;
            case DTMF_TONE_LOUD_FAST_BUSY: pNew = &(p->loudFastBusy); break;

            default: ret = OS_INVALID_ARGUMENT; break;
        }
        startDtmf(p, pNew);
} /* MpToneGen_startTone */

void MpToneGen_stopTone(MpToneGenPtr p)
{
        startDtmf(p, NULL);
} /* MpToneGen_stopTone */

#define AUDIO_A2D_BITS 14
#define AUDIO_PAD_BITS 3
#define AUDIO_A2D_MAX ((1<<(AUDIO_A2D_BITS-1))-1) /* i.e. 2047 */

//static int setsw(tonePtr t, short *d, int l)
static int setsw(tonePairPtr p, int16_t *d, int l)
{
       // int y, sinm2, sinm1, costh, range;
        int y1, sinm12, sinm11, costh1;
        int y2, sinm22, sinm21, costh2;
        int i;
        int z;

        sinm12 = p->low->sinm2;
        sinm11 = p->low->sinm1;
        costh1 = p->low->costh;

        if(p->high == NULL) {
             for (i=0; i<l; i++) {
                  y1 = ( (costh1*sinm11)>>(AUDIO_A2D_BITS) ) - sinm12;
                  sinm12 = sinm11;
                  sinm11 = y1;
                  *d++ = (int16_t) y1;
             }
             p->low->sinm2 = sinm12;
             p->low->sinm1 = sinm11;
             return (1);
        }
        sinm22 = p->high->sinm2;
        sinm21 = p->high->sinm1;
        costh2 = p->high->costh;

        for (i=0; i<l; i++) {
             y1 = ( (costh1*sinm11)>>(AUDIO_A2D_BITS) ) - sinm12;
             sinm12 = sinm11;
             sinm11 = y1;

             y2= ( (costh2*sinm21)>>(AUDIO_A2D_BITS) ) - sinm22;
             sinm22 = sinm21;
             sinm21 = y2;
             z = (y1+y2);
             if (z > 32767) z = 32767;
             if (z < -32767) z = -32767;
             *d++ = (int16_t) z;
        }
        p->low->sinm2 = sinm12;
        p->low->sinm1 = sinm11;

        p->high->sinm2 = sinm22;
        p->high->sinm1 = sinm21;
        return (l);

}

#if 0 /* [ */
static int addsw(tonePtr t, int16_t *d, int l)
{

        int y, sinm2, sinm1, costh;

        int i;

        sinm2 = t->sinm2;
        sinm1 = t->sinm1;
        costh = t->costh;
        for (i=0; i<l; i++) {
                y = ( (costh*sinm1)>>(AUDIO_A2D_BITS) ) - sinm2;
                sinm2 = sinm1;
                sinm1 = y;
                *d++ += (int16_t) y;
        }
        t->sinm2 = sinm2;
        t->sinm1 = sinm1;
        return l;
}
#endif /* ] */

/*
 * dtmfGenNext -- put the next n samples for a DTMF sequence into buffer b.
 */

OsStatus MpToneGen_getNextBuff(MpToneGenPtr pThis, int16_t *b, int N)
{
        dtmfPatternPtr d;
        int n = 0;
        int i;
        tonePairPtr p;
        OsStatus ret = OS_SUCCESS;

        pThis->mpMutex->acquire();
        d = pThis->currentDtmfSound;
        if (NULL == d) {
                n = 0;
        } else {
            p = &(d->pairs[d->curpair]);
            while (n<N) {
                i = (p->usecs - p->curusecs) / pThis->usecspersample;
                if (i > 0) {
                  /* for as many as remain, do setsw() and addsw() as needed */
                    i = min(i, (N-n));
                    if(NULL!=(p->low) || NULL!=(p->high)) {
                        setsw(p, b, i);
                    } else {
                        if (i != N) {
                            memset(b, 0, sizeof(int16_t) * i);
                        } else {
                            ret = OS_NO_MORE_DATA;
                        }
                    }

                    p->curusecs += (i * (pThis->usecspersample));
                    b += i;
                    n += i;
                } else {
                  /* move to next pair, or if none, back to the beginning */
                    d->curpair++;
                    if (d->curpair >= MAX_DTMFS) {
                        if (d->repeating) {
                            d->curpair = 0;
                            p = &(d->pairs[0]);
                            if (0 >= p->usecs) {
                                printf(
                                 "Illegal DTMF setup: d->pairs[0].usecs<=0\n");
                                p->usecs = 1000000;
                            }
                        } else {
                            startDtmf(pThis, NULL);
                            n = N;
                            ret = OS_WAIT_TIMEOUT;
                        }
                    } else {
                        p = &(d->pairs[d->curpair]);
                    }
                    p->curusecs = 0;
                }
            }
        }
        Nprintf("MTG_gNB(0x%p, 0x%p, %d) -> %d\n", pThis, b, N, n, 0,0);
        pThis->mpMutex->release();
        return ret;
} /*  MpToneGen_getNextBuff */

static void dtmfAddTone(dtmfPatternPtr d, tonePtr lo, tonePtr hi, int msecs)
{
        int i;

        if (MAX_DTMFS > d->curpair) {
            i = d->curpair++;
            d->pairs[i].low = lo;
            d->pairs[i].high = hi;
           /* d->pairs[i].lowAmp = loAmp;
            d->pairs[i].highAmp = hiAmp; */
            d->pairs[i].usecs = msecs * 1000;
            d->pairs[i].curusecs = 0;
        } else {
            printf("ERROR: Attempt to add more than %d tones to a DTMF"
                        " sequence!\n", MAX_DTMFS);
        }
}

static void dtmfReset(dtmfPatternPtr d, int repeating)
{
        memset(d, 0, sizeof(dtmfPattern));
        d->repeating = repeating;
}

static void dtmfResetCallProgressTones(MpToneGenPtr pToneGen)
{
        dtmfReset(&(pToneGen->dial), TRUE);
        dtmfReset(&(pToneGen->busy), TRUE);
        dtmfReset(&(pToneGen->ringback), TRUE);
        dtmfReset(&(pToneGen->callfailed), TRUE);
}

static void dtmfSetupCallProgressTones(MpToneGenPtr p,
                                       const char* toneLocale)
{
        bool useDefault = false;

        if (toneLocale && *toneLocale)
        {
            char locale[3];

            locale[0] = toupper(toneLocale[0]);  // convert toneLocale to
            locale[1] = toupper(toneLocale[1]);  // uppercase
            locale[2] = 0;

            // if there is a toneLocale string, check whether it is one
            // that we recognize
            if (strcmp(locale, "US") == 0)        // United States
            {
                useDefault = true;   // US locale uses default tones
            }
            else if (strcmp(locale, "DE") == 0)   // Germany
            {
                dtmfAddTone(&(p->dial), &(p->st425), NULL,  1000);
                dtmfAddTone(&(p->busy), &(p->st425), NULL,  480);
                dtmfAddTone(&(p->busy), NULL, NULL, 480);
                dtmfAddTone(&(p->ringback), &(p->st425), NULL, 1000);
                dtmfAddTone(&(p->ringback), NULL, NULL, 4000);
                dtmfAddTone(&(p->callfailed), &(p->st425), NULL, 240);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 240);
            }
            else if (strcmp(locale, "FR") == 0)   // France
            {
                dtmfAddTone(&(p->dial), &(p->st440), NULL,  1000);
                dtmfAddTone(&(p->busy), &(p->st440), NULL,  500);
                dtmfAddTone(&(p->busy), NULL, NULL, 500);
                dtmfAddTone(&(p->ringback), &(p->st440), NULL, 1500);
                dtmfAddTone(&(p->ringback), NULL, NULL, 3500);
                dtmfAddTone(&(p->callfailed), &(p->st440), NULL, 250);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 250);
            }
            else if (strcmp(locale, "GB") == 0)   // Great Britain
            {
                dtmfAddTone(&(p->dial), &(p->st350), &(p->st440),  1000);
                dtmfAddTone(&(p->busy), &(p->st400), NULL,  375);
                dtmfAddTone(&(p->busy), NULL, NULL, 375);
                dtmfAddTone(&(p->ringback), &(p->st400), &(p->st450), 400);
                dtmfAddTone(&(p->ringback), NULL, NULL, 200);
                dtmfAddTone(&(p->ringback), &(p->st400), &(p->st450), 400);
                dtmfAddTone(&(p->ringback), NULL, NULL, 2000);
                dtmfAddTone(&(p->callfailed), &(p->st400), NULL, 400);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 350);
                dtmfAddTone(&(p->callfailed), &(p->st400), NULL, 225);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 525);
            }
            else if (strcmp(locale, "IL") == 0)   // Israel
            {
                dtmfAddTone(&(p->dial), &(p->st400), NULL,  1000);
                dtmfAddTone(&(p->busy), &(p->st400), NULL,  500);
                dtmfAddTone(&(p->busy), NULL, NULL, 500);
                dtmfAddTone(&(p->ringback), &(p->st400), NULL, 1000);
                dtmfAddTone(&(p->ringback), NULL, NULL, 3000);
                dtmfAddTone(&(p->callfailed), &(p->st400), NULL, 250);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 250);
            }
            else if (strcmp(locale, "NL") == 0)   // Netherlands
            {
                dtmfAddTone(&(p->dial), &(p->st425), NULL,  1000);
                dtmfAddTone(&(p->busy), &(p->st425), NULL,  500);
                dtmfAddTone(&(p->busy), NULL, NULL, 500);
                dtmfAddTone(&(p->ringback), &(p->st425), NULL, 1000);
                dtmfAddTone(&(p->ringback), NULL, NULL, 4000);
                dtmfAddTone(&(p->callfailed), &(p->st425), NULL, 250);
                dtmfAddTone(&(p->callfailed), NULL, NULL, 250);
            }
            else
            {
                useDefault = true;   // no match found, use the default
            }
        }
        else
        {
            useDefault = true;   // no toneLocale, use the default
        }

        if (useDefault)
        {
            dtmfAddTone(&(p->dial), &(p->st350), &(p->st440),  1000);
            dtmfAddTone(&(p->busy), &(p->st480), &(p->st620),  500);
            dtmfAddTone(&(p->busy), NULL, NULL, 500);
            dtmfAddTone(&(p->ringback), &(p->st480), &(p->st440), 2000);
            dtmfAddTone(&(p->ringback), NULL, NULL, 4000);
            dtmfAddTone(&(p->callfailed), &(p->st480), &(p->st620), 300);
            dtmfAddTone(&(p->callfailed), NULL, NULL, 200);
        }

        if (0 != sineWaveHz)
        {
            // For testing ...
            // If sineWaveHz != 0, we use a sine wave of the corresponding
            // frequency in place of the normal dial tone.
            dtmfReset(&(p->dial), TRUE);
            dtmfAddTone(&(p->dial), &(p->misc), NULL,  1000);
        }
}

static void setupsw(int f, tonePtr t, int samprate, float flAmp)
{
        double xstep, dsamprate;

        dsamprate = samprate;
        xstep = f;
        xstep = (TwoPI * xstep) / dsamprate;

        t->Hz = f;

/*        t->sinth = (int) (8192.* sin(xstep) * flAmp);
        t->costh = (int) (16384.* cos(xstep));
        t->sinm2 = 0;
        t->sinm1 = t->sinth;

        t->costh = (int) (16384.* cos(xstep));
        t->sinm1 = (int)(8192.* sin(xstep) * flAmp);
*/
        t->costh = (int) (32768.* cos(xstep));
        t->sinm1 = (int)(16384.* sin(xstep) * flAmp *2);
        t->sinm2 = 0;

#ifdef DEBUG_DTMF
        printf(" Theta = %10.5f, sin(theta) = %5d, cos(theta) = %5d\n",
            xstep, t->sinth, t->costh);
#endif
}

#define DTMF_KEY_AMPS  DTMF_KEY_ROW_AMP, DTMF_KEY_COL_AMP
static float sineWaveAmp = 0;

static void setuptones(MpToneGenPtr p, int samprate,
                       const char* toneLocale)
{
        p->samplerate = samprate;
        p->usecspersample = (1000000 + (samprate/2)) / samprate;

        setupsw(350, &(p->st350), samprate, .2F);
        setupsw(400, &(p->st400), samprate, .2F);
        setupsw(425, &(p->st425), samprate, .2F);
        setupsw(440, &(p->st440), samprate, .2F);
        setupsw(450, &(p->st450), samprate, .2F);
        setupsw(480, &(p->st480), samprate, .2F);
        setupsw(480, &(p->st480Loud), samprate, DTMF_LOUD_BUSY_AMP);
        setupsw(620, &(p->st620), samprate, .2F);
        setupsw(620, &(p->st620Loud), samprate, DTMF_LOUD_BUSY_AMP);
        setupsw(697, &(p->st697), samprate, DTMF_KEY_ROW_AMP);
        setupsw(770, &(p->st770), samprate, DTMF_KEY_ROW_AMP);
        setupsw(852, &(p->st852), samprate, DTMF_KEY_ROW_AMP);
        setupsw(941, &(p->st941), samprate, DTMF_KEY_ROW_AMP);
        setupsw(1209, &(p->st1209h), samprate, .47F);
        setupsw(1209, &(p->st1209l), samprate, DTMF_KEY_COL_AMP);
        setupsw(1336, &(p->st1336), samprate, DTMF_KEY_COL_AMP);
        setupsw(1477, &(p->st1477h), samprate, .47F);
        setupsw(1477, &(p->st1477l), samprate, DTMF_KEY_COL_AMP);
        setupsw(1633, &(p->st1633), samprate, DTMF_KEY_COL_AMP);

        setupsw(sineWaveHz, &(p->misc), samprate, sineWaveAmp);

        p->mpMutex->acquire();
        dtmfReset(&(p->key1), TRUE);
        dtmfReset(&(p->key2), TRUE);
        dtmfReset(&(p->key3), TRUE);
        dtmfReset(&(p->keya), TRUE);
        dtmfReset(&(p->key4), TRUE);
        dtmfReset(&(p->key5), TRUE);
        dtmfReset(&(p->key6), TRUE);
        dtmfReset(&(p->keyb), TRUE);
        dtmfReset(&(p->key7), TRUE);
        dtmfReset(&(p->key8), TRUE);
        dtmfReset(&(p->key9), TRUE);
        dtmfReset(&(p->keyc), TRUE);
        dtmfReset(&(p->keystar), TRUE);
        dtmfReset(&(p->key0), TRUE);
        dtmfReset(&(p->keysharp), TRUE);
        dtmfReset(&(p->keyd), TRUE);
        dtmfReset(&(p->ringtone), TRUE);
        dtmfReset(&(p->backspace), FALSE);
        dtmfReset(&(p->callwait), FALSE);
        dtmfReset(&(p->callheld), TRUE);
        dtmfReset(&(p->loudFastBusy), TRUE);
        dtmfReset(&(p->silence), TRUE);
        dtmfResetCallProgressTones(p);

        dtmfAddTone(&(p->key1), &(p->st697), &(p->st1209l), 1000);
        dtmfAddTone(&(p->key2), &(p->st697), &(p->st1336),  1000);
        dtmfAddTone(&(p->key3), &(p->st697), &(p->st1477l), 1000);
        dtmfAddTone(&(p->keya), &(p->st697), &(p->st1633),  1000);
        dtmfAddTone(&(p->key4), &(p->st770), &(p->st1209l), 1000);
        dtmfAddTone(&(p->key5), &(p->st770), &(p->st1336),  1000);
        dtmfAddTone(&(p->key6), &(p->st770), &(p->st1477l), 1000);
        dtmfAddTone(&(p->keyb), &(p->st770), &(p->st1633),  1000);
        dtmfAddTone(&(p->key7), &(p->st852), &(p->st1209l), 1000);
        dtmfAddTone(&(p->key8), &(p->st852), &(p->st1336),  1000);
        dtmfAddTone(&(p->key9), &(p->st852), &(p->st1477l), 1000);
        dtmfAddTone(&(p->keyc), &(p->st852), &(p->st1633),  1000);
        dtmfAddTone(&(p->keystar), &(p->st941), &(p->st1209l), 1000);
        dtmfAddTone(&(p->key0), &(p->st941), &(p->st1336), 1000);
        dtmfAddTone(&(p->keysharp), &(p->st941), &(p->st1477l), 1000);
        dtmfAddTone(&(p->keyd), &(p->st941), &(p->st1633), 1000);
        dtmfAddTone(&(p->ringtone), &(p->st1209h),  NULL, 500);
        /* dtmfAddTone(&(p->ringtone), &(p->st480),  NULL, 125); */
        dtmfAddTone(&(p->ringtone), &(p->st1477h), NULL, 500);
        dtmfAddTone(&(p->ringtone), NULL, NULL, 2000);
        dtmfAddTone(&(p->backspace), &(p->st852), &(p->st1633),  50);
        dtmfAddTone(&(p->callwait), &(p->st941), &(p->st1633), 50);
        dtmfAddTone(&(p->callheld), &(p->st480), &(p->st440), 200);
        dtmfAddTone(&(p->callheld), NULL, NULL, 150);
        dtmfAddTone(&(p->callheld), &(p->st480), &(p->st440), 200);
        dtmfAddTone(&(p->callheld), NULL, NULL, 150);
        dtmfAddTone(&(p->callheld), &(p->st480), &(p->st440), 200);
        dtmfAddTone(&(p->callheld), NULL, NULL, 1950);
        dtmfAddTone(&(p->loudFastBusy), &(p->st480Loud), &(p->st620Loud), 250);
        dtmfAddTone(&(p->loudFastBusy), NULL, NULL, 250);
        dtmfAddTone(&(p->silence), NULL, NULL, 1000);
        dtmfSetupCallProgressTones(p, toneLocale);
        p->mpMutex->release();

} /* setuptones */

static void dtmfSetSampleRate(MpToneGenPtr p, int samprate,
                              const char* toneLocale)
{
        setuptones(p, samprate, toneLocale);
}

void MpToneGen_delete(MpToneGenPtr p)
{
        delete p->mpMutex;
        delete p;
}

MpToneGenPtr MpToneGen_MpToneGen(int samprate, const char* toneLocale)
{
        MpToneGenPtr pThis;

        pThis = (MpToneGenPtr) new MpToneGen;
        if (NULL != pThis) {
            pThis->mpMutex = new OsMutex(OsMutex::Q_PRIORITY);
            if (NULL != pThis->mpMutex) {
                pThis->currentDtmfSound = NULL;
                dtmfSetSampleRate(pThis, samprate, toneLocale);
            } else {
                delete pThis;
                pThis = NULL;
            }
        }
        Zprintf("MpToneGen_MpToneGen(%d) -> 0x%p\n",
            samprate, pThis, 0,0,0,0);
        return pThis;
}

/***************************************************************************
 *
 *  This is the user interface to the sine wave generator.
 *
 *  When this feature is enabled, the standard dialtone is replaced with
 *  a continuous single frequency tone of a designated frequency.  By
 *  default, this feature is disabled, which is indicated by the static
 *  variable "sineWaveHz" being equal to 0.  The amplitude of the tone
 *  can be set by steps of 1% from 10% to 100% (of the full range of our
 *  16 bit digital samples) (NOTE: I have not verified that the amplitude
 *  still covers the full range; it did before the conversion from floating
 *  point to integer tone generation...).
 *
 *  Input:
 *      int Hz -- frequency for single tone sine wave, if Hz > 0;
 *                show current settings, no change, if Hz == 0;
 *                disable feature, restore standard dialtone if Hz < 0
 *      int pct -- amplitude, as a percentage.  If pct > 100, pct is
 *                 set to 100; if pct < 10, pct defaults to 80.
 *
 *  Returns:  The subsequent value of sineWaveHz:
 *            retval > 0  ==> value is frequency of tone;
 *            retval == 0 ==> standard dialtone enabled
 */

int setSine(int Hz, int pct) {
    if (0 > Hz) {
        printf("Restoring standard dialtone...\n");
        sineWaveHz = 0;
        sineWaveAmp = 0.0;
        return 0;
    }
    if (0 == sineWaveHz) {
        printf("Currently using standard dialtone\n");
    } else {
        printf("Currently substituting %d Hz (with amplitude = %f)"
            " for dialtone\n", sineWaveHz, sineWaveAmp);
    }
    if (0 < Hz) {

        sineWaveHz = Hz;
        if (pct > 99) pct = 100;
        if (pct < 10) pct = 80;
        sineWaveAmp = (float) (pct / 100.);
        printf("Replacing with %d Hz (with amplitude = %f)\n",
                 sineWaveHz, sineWaveAmp);
    }
    return sineWaveHz;
}

#define TEST_DRIVER
#undef TEST_DRIVER
#ifdef TEST_DRIVER /* [ */
#ifdef _VXWORKS /* [ */
#include <taskLib.h>
#include <ioLib.h>
#include "mp/sa1100.h"
#endif /* _VXWORKS ] */
/**********************************************************************/
static int samprate;
static int samps_per_tone;

        volatile struct mcpreg *mcp = (struct mcpreg *) MCPBASE;

void mcpCsrWr(int a, int d) {
        volatile struct mcpreg *mcp;

        mcp = (struct mcpreg *) MCPBASE;

        while ((mcp->mcsr & MCSR_M_CWC) == 0);
        mcp->mcdr2 = ((a&0xf) << MCDR2_V_RN) | MCDR2_M_nRW | (d & 0xFFFF);
}

int mcpAudioOut2(int l, uint16_t *d) {
        volatile struct mcpreg *mcp;
        int i;
        int16_t *p;

        mcp = (struct mcpreg *) MCPBASE;

        p = d;
        for (i=0; i<l; i++) {
            while ((mcp->mcsr & MCSR_M_ANF) == 0); /* wait for room in fifo */
            mcp->mcdr0 = *p++; /* put the data */
        }
        return l;
}

int mcpCsrRd(int a) {
        volatile struct mcpreg *mcp;

        mcp = (struct mcpreg *) MCPBASE;

        while ((mcp->mcsr & MCSR_M_CWC) == 0);
        mcp->mcdr2 = ((a&0xf) << MCDR2_V_RN);
        while ((mcp->mcsr & MCSR_M_CRC) == 0);
        return (mcp->mcdr2 & 0xFFFF);
}

/**********************************************************************/

int errno;

#define CHUNK_SAMPS 200

char get1char()
{
        char x[256];

        memset(x, 0, 256);
        errno = 0;
        gets(x);
        return x[0];
}

static SEM_ID semDelay = NULL;

int ticks(ms)
{
#define Hz 60
        return (((ms * Hz) + (Hz/2)) / 1000);
}

void playsilence(MpToneGenPtr p, int ms)
{
        MpToneGen_startTone(p, DTMF_TONE_SILENCE);
        semTake(semDelay, ticks(ms));
}

void playdigit(MpToneGenPtr p, char digit, int ms)
{
        MpToneGen_startTone(p, digit);
        semTake(semDelay, ticks(max(60,ms)));
        playsilence(p, 80);
}

int iskey(char x)
{
    int is = 0;

    switch (x) {
    case '1':
    case '2':
    case '3':
    case 'a':
    case 'A':
    case '4':
    case '5':
    case '6':
    case 'b':
    case 'B':
    case '7':
    case '8':
    case '9':
    case 'c':
    case 'C':
    case '*':
    case '0':
    case '#':
    case 'd':
    case 'D':
        is = 1;
        break;
    default:
        break;
    }
    return is;
}

int playnumb(MpToneGenPtr p, char *n)
{
        int i;
        char x;

        if ('+' != *n) {
            i = 0;
            while ((x = (*n++))) {
                if (iskey(x)) {
                    playdigit(p, x, 100);
                    i++;
                }
            }
        } else {
            i = 0;
            x = 0;
            while ('q' != x) {
                x = get1char();
                if (iskey(x)) {
                    i++;
                    playsilence(p, 80);
                    MpToneGen_startTone(p, x);
                }
            }
        }
        return i;
}

void playdial(MpToneGenPtr p, int ms)
{
        MpToneGen_startTone(p, DTMF_TONE_DIALTONE);
        semTake(semDelay, ticks(max(60,ms)));
}

void playbusy(MpToneGenPtr p, int ms)
{
        MpToneGen_startTone(p, DTMF_TONE_BUSY);
        semTake(semDelay, ticks(max(60,ms)));
}

void playringback(MpToneGenPtr p, int ms)
{
        MpToneGen_startTone(p, DTMF_TONE_RINGBACK);
        semTake(semDelay, ticks(max(60,ms)));
}

void playring(MpToneGenPtr p, int ms)
{
        MpToneGen_startTone(p, DTMF_TONE_RINGTONE);
        semTake(semDelay, ticks(max(60,ms)));
}

static int toning;
static MpToneGenPtr pTG;

int play_tones(int in_samprate, int atten, int t3, int t4, int t5, int t6,
                int t7, int t8, int t9, int t10)
{
        int div, db=0;
        short buf[CHUNK_SAMPS];

        {
                div = ((11981000 / in_samprate) + 16) >> 5;
                printf("Sample rate is %d Hz, divisor = %d", in_samprate, div);
                if (div < 6) {
                        div = 6;
                        printf(", divisor adjusted to %d", div);
                }
                if (div > 127) {
                        div = 127;
                        printf(", divisor adjusted to %d", div);
                }
                printf("\n");
        }

        samprate = ((11981000 / div) + 16) >> 5;
        samps_per_tone = (samprate + 4) / 5;
        printf("sample rate is %d\n", samprate);
        pTG = MpToneGen_MpToneGen(samprate);
        toning = 1;
        /* dtmfStopTone(); */

        semGive(semDelay);

        /* turn on the MCP, set MCP audio divisor to div */
        mcp->mccr = MCCR_M_ADM | MCCR_M_MCE | (40 << MCCR_V_TSD) | div;
        mcpCsrWr(1, 0x807F);

        /* set gain and Codec freq div */
        mcpCsrWr(7, (db<<7) | div);
        printf("set gain to %d\n", db);

        /* set output attenuation and turn on audio out  */
        mcpCsrWr(8, 0x8000 | (0x1f & atten));

        while(toning) {
                MpToneGen_getNextBuff(pTG, buf, CHUNK_SAMPS);
                mcpAudioOut2(CHUNK_SAMPS, buf);
        }
        MpToneGen_delete(pTG);

        /* Turn off the MCP */
        mcp->mccr = MCCR_M_ADM | 0 | (40 << MCCR_V_TSD) | div;

        semGive(semDelay);
        return 0;
}

int player(char *number, int in_samprate, int atten, int scenario)
{
        int myPrio, tid1, digits;
        char s[100];

        semDelay = semBCreate(0, 0);

        /* status = ioctl(0, FIOSETOPTIONS, OPT_RAW); */

        taskPriorityGet(taskIdSelf(), &myPrio);
        if (ERROR ==
          (tid1 = taskSpawn("ToneGen", /*myPrio+10*/ 250, 0, 8192,
            play_tones, in_samprate, atten, 0,0,0,0,0,0,0,0))) {
                semDelete(semDelay);
                semDelay = NULL;
                return ERROR;
        }
        printf("Spawned ToneGen (0x%X) \n", tid1);

        semTake(semDelay, WAIT_FOREVER);

        digits = -1;
        playsilence(pTG, 200);
        switch (scenario) {
        case 1:
                MpToneGen_startTone(pTG, DTMF_TONE_DIALTONE);
                gets(s);
                digits = playnumb(pTG, number);
                playsilence(pTG, 800);
                MpToneGen_startTone(pTG, DTMF_TONE_BUSY);
                gets(s);
                break;
        case 2:
                playring(pTG, 7500);
                break;
        case 3:
                MpToneGen_startTone(pTG, DTMF_TONE_DIALTONE);
                gets(s);
                digits = playnumb(pTG, number);
                playsilence(pTG, 800);
                MpToneGen_startTone(pTG, DTMF_TONE_RINGBACK);
                gets(s);
                break;
        }
        playsilence(pTG, 500);
        toning = 0;
        if (-1 != digits) printf("played %d digits\n", digits);
        semTake(semDelay, WAIT_FOREVER);
        semDelete(semDelay);
        semDelay = NULL;
        return 0;
}
#endif /* TEST_DRIVER ] */
