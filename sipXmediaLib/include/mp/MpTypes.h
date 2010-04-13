//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INCLUDED_MPTYPES_H /* [ */
#define _INCLUDED_MPTYPES_H

#include <stdint.h>

/**************************************************************************/
/* One of these should be defined, and the other should undefined!!!!!    */
/* These are used to determine whether 16 bit samples need to be byte     */
/* swapped.  All hosts (SA1100 and WinTel) are currently little endian    */
#ifdef _VXWORKS /* [ */
#undef  _BYTE_ORDER_IS_LITTLE_ENDIAN
#undef  _BYTE_ORDER_IS_BIG_ENDIAN
#define _BYTE_ORDER_IS_LITTLE_ENDIAN
#else /* _VXWORKS ] [ */
#undef  _BYTE_ORDER_IS_LITTLE_ENDIAN
#undef  _BYTE_ORDER_IS_BIG_ENDIAN
#define _BYTE_ORDER_IS_LITTLE_ENDIAN
#endif /* _VXWORKS ] */
/*/////////////////////////////////////////////////////////////////////// */

#ifdef _VXWORKS /* [ */
#include "vxWorks.h"
#else /* _VXWORKS ] [ */
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef int STATUS;
typedef void * MSG_Q_ID;
typedef void * SEM_ID;
#endif /* _VXWORKS ] */

typedef int16_t Sample;

typedef struct __MpBuf_tag MpBuf;
typedef struct __MpBuf_tag *MpBufPtr;
typedef struct __MpBufPool_tag *MpBufPoolPtr;

/* buffer message types */

#define DMA_COMPLETE 103

#endif /* _INCLUDED_MPTYPES_H ] */
