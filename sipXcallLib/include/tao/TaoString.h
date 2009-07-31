//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoString_h_
#define _TaoString_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <utl/UtlString.h>

#define TAO_STRING_ARRAY_LENGTH 100
// #define TAOSTR_DEBUG

class TaoString : public UtlString
{
public:
        TaoString();

        TaoString(const char* str, const char* delimiter);

        const char* operator []( int nIndex ) const;


        virtual ~TaoString();

        int getCnt() const { return mCnt; };

   TaoString(const TaoString& rTaoString);
    //:Copy constructor (not implemented for this class)

#ifdef TAOSTR_DEBUG
        static unsigned int mStrCnt;
#endif

private:
        int                     mCnt;
        UtlString       mDelimiter;
   char*       mStrArray[TAO_STRING_ARRAY_LENGTH + 1] ;


};

#endif // _TaoString_h_
