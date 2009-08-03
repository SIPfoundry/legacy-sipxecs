//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include <stdlib.h>
#include <string.h>

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "tao/TaoString.h"

#ifdef TAOSTR_DEBUG
unsigned int TaoString::mStrCnt = 0;
unsigned int getTaoStrCnt()
{
        return TaoString::mStrCnt;
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoString::TaoString()
{
#ifdef TAOSTR_DEBUG
        mStrCnt++;
#endif

   mCnt = 0;
}

TaoString::TaoString(const char *str, const char *delimiter)
        : mCnt(0),
    mDelimiter(delimiter)
{
#ifdef TAOSTR_DEBUG
   mStrCnt++;
#endif
   size_t    length;
   size_t   pos;
   ssize_t   pos2;
   size_t   patLen;
   size_t    strLen ;

   pos = 0;
   pos2 = 0;
   patLen = strlen(delimiter);
   UtlString tmp(str);
   strLen = strlen(str) ;

   while (UTL_NOT_FOUND != (pos2 = tmp.index(delimiter, pos)))
   {
      length = pos2 - pos ;
      if (length)
      {
         mStrArray[mCnt] = (char*) malloc(length+1) ;
         strncpy(mStrArray[mCnt], &str[pos], length) ;
         mStrArray[mCnt][length] = 0 ;
      }
      else
         mStrArray[mCnt] = (char*) strdup("") ;


      pos = pos2 + patLen;
      mCnt++;
      if (mCnt >= TAO_STRING_ARRAY_LENGTH)
      {
         mCnt = TAO_STRING_ARRAY_LENGTH - 1;
         break ;
      }
   }

   if (pos < strLen)
   {
      length = strLen - pos ;
      mStrArray[mCnt] = (char*) malloc(length+1) ;
      strncpy(mStrArray[mCnt], &str[pos], strLen-pos) ;
      mStrArray[mCnt][length] = 0 ;
      mCnt++ ;
   }
   else if ((mCnt == 0) && (*str != 0))
   {
      mStrArray[mCnt++] = strdup(str) ;
   }
   mStrArray[mCnt] = NULL;
}

TaoString::~TaoString()
{
   for (int i=0;i<mCnt; i++)
   {
      free(mStrArray[i]) ;
      mStrArray[i] = NULL ;
   }
   mCnt = 0 ;

#ifdef TAOSTR_DEBUG
        mStrCnt--;
#endif
}

const char* TaoString::operator []( int nIndex ) const
{
        if ((nIndex >= 0) && (nIndex < mCnt))
                return mStrArray[nIndex] ;
        else
                return "";
}
