//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#if !defined _RESAMPBY2FIX
#define _RESAMPBY2FIX

/* Definitions of data types */
typedef int                GIPS_Word32;
typedef short int          GIPS_Word16;
typedef unsigned int       GIPS_UWord32;
typedef unsigned short int GIPS_UWord16;


#ifdef __cplusplus
extern "C" {
#endif

/* decimator */
void GIPS_downsampling2(
   const GIPS_Word16* in,               /* input array */
   int                len,              /* length of input array */
   GIPS_Word16*       out,              /* output array (of length len/2) */
   GIPS_Word32*       filtState         /* filter state array; length = 8 */
);

void GIPS_upsampling2(
   const GIPS_Word16* in,               /* input array */
   int                len,              /* length of input array */
   GIPS_Word16*       out,              /* output array (of length len*2) */
   GIPS_Word32*       filtState         /* filter state array; length = 8 */
);

#ifdef __cplusplus
}
#endif

#endif
