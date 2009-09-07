//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef QSTYPES__H__
#define QSTYPES__H__

/*-----------------------------------------------------------------------------
 * File: qsTypes.h
 * Module: STUN
 * Description:
 *  This file contains the declaration of the types that are used in the STUN
 *  module.
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * Module history
 *
 * Date			Description
 * ----			-----------
 * 26 Feb 05		Initial version of the file
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * Declarations and definitions:
 *  This section contains the local declarations and definitions of constants,
 *  macros, typdefs, etc.
 *-----------------------------------------------------------------------------
 */

/* 8 bit integers */
typedef char CHAR;
typedef unsigned char UCHAR;

/* 16 bit integers */
typedef short SHORT;
typedef unsigned short USHORT;

/* 32 bit integers */
typedef int INT;
typedef unsigned int UINT;

/* 64 bit integers */
#ifdef WIN32
typedef unsigned __int64 UInt64;
typedef __int64 Int64;
#else
typedef unsigned long long UInt64;
typedef long long int Int64;
#endif /* WIN32 */

/* 128 bit integers; unsigned is not defined since it is not likely needed */
typedef struct {
    unsigned char octet[16];
} UINT128;

#endif /* LOCALTYPES__H__ */
