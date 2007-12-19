//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include "os/OsSysLog.h"

#include "net/SipLineList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SipLineList::SipLineList()
{}

SipLineList::~SipLineList()
{    
    while (SipLine* pLine = (SipLine*) m_LineList.pop()) 
    {
        delete pLine ;
    }
}

void
SipLineList::add(SipLine *newLine)
{
        m_LineList.push(newLine);
}

UtlBoolean
SipLineList::remove(SipLine *line)
{
    // Changed this to call common code
    Url lineIdentityUrl = line->getIdentity();
    return remove( lineIdentityUrl );
}

UtlBoolean
SipLineList::remove(const Url& lineIdentityUrl)
{
   SipLine* nextline = NULL;

   int iteratorHandle = m_LineList.getIteratorHandle();
        while(NULL != (nextline = (SipLine*) m_LineList.next(iteratorHandle)))
        {
      // compare the line identities
      Url nextLineUrl = nextline->getIdentity();
      if (lineIdentityUrl.isUserHostPortEqual(nextLineUrl))
      {
         m_LineList.remove(iteratorHandle);
         break;
      }
   }
   m_LineList.releaseIteratorHandle(iteratorHandle);

        return( nextline != NULL );
}

UtlBoolean
SipLineList::isDuplicate( SipLine *line )
{
    // Changed this to call common code
    Url lineIdentityUrl = line->getIdentity();
    return isDuplicate( lineIdentityUrl );
}

UtlBoolean
SipLineList::isDuplicate( const Url& lineIdentityUrl )
{
   SipLine* nextline = NULL;
   UtlBoolean isDuplicate = FALSE;

   int iteratorHandle = m_LineList.getIteratorHandle();
   while(NULL != (nextline = (SipLine*) m_LineList.next(iteratorHandle)))
   {
      // compare the line identities
      Url nextLineUrl = nextline->getIdentity();
      if (lineIdentityUrl.isUserHostPortEqual(nextLineUrl))
      {
         isDuplicate = TRUE;
         break;
      }
   }
   m_LineList.releaseIteratorHandle(iteratorHandle);

        return isDuplicate;
}

SipLine*
SipLineList::getLine( const Url& lineIdentityUrl )
{
   SipLine* nextline = NULL;

   int iteratorHandle = m_LineList.getIteratorHandle();
        while(NULL != (nextline = (SipLine*) m_LineList.next(iteratorHandle)))
   {
      // compare the line identities
      Url nextLineUrl = nextline->getIdentity();
      if (lineIdentityUrl.isUserHostPortEqual(nextLineUrl))
      {
         break ;
      }
   }

   m_LineList.releaseIteratorHandle(iteratorHandle);

        return nextline;
}

SipLine*
SipLineList::getLine(const UtlString& lineId)
{
        SipLine* nextline = NULL;

    if ( !lineId.isNull() )
    {
            int iteratorHandle = m_LineList.getIteratorHandle();
            while(NULL != (nextline = (SipLine*) m_LineList.next(iteratorHandle)))
        {
            // compare the line identities
            UtlString nextLineId = nextline->getLineId();
            if (!nextLineId.isNull())
            {
#ifdef TEST_PRINT
                osPrintf( "SipLineList::getLine Comparing %s to %s \n",
                          lineId.data(), nextLineId.data() );
#endif
                        if( lineId == nextLineId )
                {
#ifdef TEST_PRINT
                    osPrintf("SipLineList::getLine TRUE\n");
#endif
                                break;
                }
            }
        }
            m_LineList.releaseIteratorHandle(iteratorHandle);
    }
        return nextline;
}

SipLine*
SipLineList::getLine (
    const UtlString& userId,
    int& numOfMatches )
{
        SipLine* nextline = NULL;
    SipLine* firstMatchedLine = NULL;
    UtlString nextUserId;
    numOfMatches = 0;

    if (!userId.isNull())
    {
        int iteratorHandle = m_LineList.getIteratorHandle();
            while(NULL != (nextline = (SipLine*) m_LineList.next(iteratorHandle)))
            {
            nextUserId.remove(0);
            Url identity = nextline->getIdentity();
            identity.getUserId( nextUserId );

            if (!nextUserId.isNull())
            {
#ifdef TEST_PRINT
                osPrintf( "SipLineList::getLine Comparing %s to %s \n",
                          nextUserId.data(), userId.data() );
#endif
                        if( nextUserId.compareTo( userId ) == 0 )
                        {
#ifdef TEST_PRINT
                    osPrintf("SipLineList::getLine TRUE\n");
#endif
                    if( numOfMatches == 0)
                    {
                        firstMatchedLine = nextline;
                    }
                    numOfMatches ++;
                }
            }
            }
        m_LineList.releaseIteratorHandle(iteratorHandle);
    }
        return firstMatchedLine;
}


int
SipLineList::getListSize()
{
        return m_LineList.getCount();
}

UtlBoolean
SipLineList::linesInArray(
    int maxSize,
    int *returnSize,
    SipLine Line[])
{

        int counter = 0 ;
        UtlBoolean retVal;
        int iteratorHandle = m_LineList.getIteratorHandle();
        SipLine* nextLine = NULL;

        while (counter < maxSize && ((nextLine = (SipLine*) m_LineList.next(iteratorHandle))!= NULL))
        {
                //return copy of the line
                Line[counter] = *nextLine;
                counter++;
        }
        m_LineList.releaseIteratorHandle(iteratorHandle);
        *returnSize = counter;
        counter > 0 ? retVal = true : retVal = false;
        return retVal;
}

UtlBoolean
SipLineList::linesInArray(
    int maxSize,
    int *returnSize,
    SipLine *Line[])
{
        int counter = 0 ;
        UtlBoolean retVal;
        int iteratorHandle = m_LineList.getIteratorHandle();
        SipLine* nextLine = NULL;

        while (counter < maxSize && ((nextLine = (SipLine*) m_LineList.next(iteratorHandle))!= NULL))
        {
                //return copy of the line
                *Line[counter] = *nextLine;
                counter++;
        }
        m_LineList.releaseIteratorHandle(iteratorHandle);
        *returnSize = counter;
        counter > 0 ? retVal = true : retVal = false;
        return retVal;
}

UtlBoolean
SipLineList::getFirstLine( SipLine *Line )
{
        UtlBoolean retVal = FALSE;
        int iteratorHandle = m_LineList.getIteratorHandle();
        SipLine* nextLine = NULL;
        nextLine = (SipLine*) m_LineList.next(iteratorHandle);
        if(nextLine)
        {
                //return copy of the line
                *Line = *nextLine;
                retVal = TRUE;
        }
        m_LineList.releaseIteratorHandle(iteratorHandle);
        return retVal;
}

UtlBoolean
SipLineList::getDeviceLine(SipLine *line)
{
        UtlBoolean retVal = FALSE;
        UtlString user;
        int iteratorHandle = m_LineList.getIteratorHandle();
        SipLine* nextLine = NULL;
        while ((nextLine = (SipLine*) m_LineList.next(iteratorHandle))!= NULL)
        {
                user = nextLine->getUser();
                if ( user.compareTo("device" , UtlString::ignoreCase) == 0)
                {
                        *line = *nextLine;
                        retVal = TRUE;
         break;
                }
                user.remove(0);
        }
        m_LineList.releaseIteratorHandle(iteratorHandle);
        return retVal;
}



//
// Priorities:
//   1. Matches first lineID & realm
//   2. Matches first matches toFromUrl & realm
//   3. Matches first user & realm
//   4. Matches first default line & realm
//
SipLine* SipLineList::findLine(const char* lineId,
                               const char* realm,
                               const Url&  toFromUrl,
                               const char* userId,
                               const Url&  defaultLine)
{
   SipLine* pLineMatchingLineID  = NULL ;
   SipLine* pLineMatchingUrl     = NULL ;
   SipLine* pLineMatchingUser    = NULL ;
   SipLine* pLineMatchingDefault = NULL ;

#  ifdef TEST_PRINT
   OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG,
                 "SipLineList::findLine searching for lineid '%s' realm '%s' url '%s' default '%s'",
                 lineId, realm, toFromUrl.toString().data(), defaultLine.toString().data()
                 );
#  endif
   
   int iteratorHandle = m_LineList.getIteratorHandle();
   SipLine* nextLine = NULL;
   for (nextLine = (SipLine*) m_LineList.next(iteratorHandle);
        pLineMatchingLineID == NULL && nextLine != NULL;
        nextLine = (SipLine*) m_LineList.next(iteratorHandle)
        )
   {
#     ifdef TEST_PRINT
      Url tmpLineUrl = nextLine->getIdentity();
      OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG,
                    "SipLineList::findLine checking '%s'",
                    tmpLineUrl.toString().data());
#     endif

      // If the realm doesn't match, simply skip it
      if ((realm != NULL) && strlen(realm)
          && (nextLine->IsDuplicateRealm(realm)))
      {
         //
         // Priority 1: Check LineId
         //
         if (   lineId != NULL
             && nextLine->getLineId().compareTo(lineId) == 0)
         {
            // We have match for the given lineId
            pLineMatchingLineID = nextLine ;
            // since this is the best possible match, it will exit the loop.
            OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG,
                          "SipLineList::findLine matched line id '%s'",
                          lineId);
         }
         else
         {
            Url nextLineIdentity = nextLine->getIdentity();

            OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG,
                          "SipLineList::findLine checking '%s'",
                          nextLineIdentity.toString().data());

            //
            // Priority 2: check ToFromUrl
            //
            if (   pLineMatchingUrl == NULL
                && nextLineIdentity.isUserHostPortEqual(toFromUrl)
                )
            {
               pLineMatchingUrl = nextLine ;
               // Continue searching, because we may find a better match
            }
            else
            {
               //
               // Priority 3: Matches user & realm
               //
               UtlString user = nextLine->getUser() ;
               if (   pLineMatchingUser == NULL
                   && userId != NULL
                   && user.compareTo(userId) == 0    // should be case sensitive
                   )
               {
                  pLineMatchingUser = nextLine ;
                  // Continue searching, because we may find a better match
               }
               else
               {
                  //
                  // Priority 4: Check for default line
                  //
                  if (nextLineIdentity.isUserHostPortEqual(defaultLine))
                  {
                     pLineMatchingDefault = nextLine ;
                     // Continue searching, because we may find a better match
                  }
               }
            }
         }
      }
   }
   m_LineList.releaseIteratorHandle(iteratorHandle) ;

   // This is ugly, but needed for the desired effect
   SipLine* foundLine = (  pLineMatchingLineID  ? pLineMatchingLineID
           : pLineMatchingUrl     ? pLineMatchingUrl
           : pLineMatchingUser    ? pLineMatchingUser
           : pLineMatchingDefault ? pLineMatchingDefault
           : NULL );
   
   OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG, "SipLineList::findLine %s",
                 foundLine ? "found" : "NOT found");
 
   return foundLine;
}
