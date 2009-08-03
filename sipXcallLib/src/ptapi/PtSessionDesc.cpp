//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// PtSessionDesc.cpp: implementation of the PtTerminalConnection2 class.
//
//////////////////////////////////////////////////////////////////////

int FORCE_REFERENCE_PtSessionDesc = 0 ;


#include "ptapi/PtSessionDesc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PtSessionDesc::PtSessionDesc()
    : UtlString("")
{
    mToUrl = "";
    mFromUrl = "";
    mLocalContact = "";

    mNextCseq = -1;
    mLastFromCseq = -1;
    mLastToCseq = -1;
    mSessionState = SESSION_UNKNOWN;
}


// Constructor
PtSessionDesc::PtSessionDesc(const char* callId,
                                                         const char* toUrl,
                                                         const char* fromUrl,
                             const char* localContact,
                                                         int nextCseq,
                                                         int lastFromCseq,
                                                         int lastToCseq,
                                                         int sessionState)
    : UtlString(callId)
{
    mToUrl = toUrl;
    mFromUrl = fromUrl;
    mLocalContact = localContact;

    mNextCseq = nextCseq;
    mLastFromCseq = lastFromCseq;
    mLastToCseq = lastToCseq;
    mSessionState = sessionState;
}

// Copy constructor
PtSessionDesc::PtSessionDesc(const PtSessionDesc& rPtSessionDesc)
  : UtlString(rPtSessionDesc)
{
   mFromUrl = rPtSessionDesc.mFromUrl;
   mToUrl = rPtSessionDesc.mToUrl;
   mLocalContact = rPtSessionDesc.mLocalContact;
   mNextCseq = rPtSessionDesc.mNextCseq;
   mLastFromCseq = rPtSessionDesc.mLastFromCseq;
   mLastToCseq = rPtSessionDesc.mLastToCseq;
   mSessionState = rPtSessionDesc.mSessionState;
}

PtSessionDesc::~PtSessionDesc()
{

}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
PtSessionDesc&
PtSessionDesc::operator=(const PtSessionDesc& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs);  // assign fields for parent class


   mFromUrl = rhs.mFromUrl;
   mToUrl = rhs.mToUrl;
   mLocalContact = rhs.mLocalContact;
   mNextCseq = rhs.mNextCseq;
   mLastFromCseq = rhs.mLastFromCseq;
   mLastToCseq = rhs.mLastToCseq;
   mSessionState = rhs.mSessionState;

   return *this;
}

void PtSessionDesc::setCallId(const char* callId)
{
    remove(0);
    append(callId ? callId : "");
}

void PtSessionDesc::setFromUrl(const UtlString& fromUrl)
{
    mFromUrl = fromUrl;
}

void PtSessionDesc::setToUrl(const UtlString& toUrl)
{
    mToUrl = toUrl;
}

void PtSessionDesc::setLocalContact(const UtlString& localContact)
{
    mLocalContact = localContact;
}

void PtSessionDesc::setLastFromCseq(int lastFromCseq)
{
    mLastFromCseq = lastFromCseq;
}

void PtSessionDesc::setLastToCseq(int lastToCseq)
{
    mLastToCseq = lastToCseq;
}

/* ============================ ACCESSORS ================================= */

void PtSessionDesc::getCallId(UtlString& callId)
{
    callId = data();
}

void PtSessionDesc::getFromUrl(UtlString& fromUrl)
{
    fromUrl = mFromUrl;
}

void PtSessionDesc::getToUrl(UtlString& toUrl)
{
    toUrl = mToUrl;
}

void PtSessionDesc::getLocalContact(UtlString& localContact)
{
    localContact = mLocalContact;
}

int PtSessionDesc::getLastFromCseq()
{
    return(mLastFromCseq);
}

int PtSessionDesc::getLastToCseq()
{
    return(mLastToCseq);
}

int PtSessionDesc::getNextFromCseq()
{
    mLastFromCseq++;
    return(mLastFromCseq);
}

int PtSessionDesc::getSessionState()
{
    return(mSessionState);
}


/* ============================ FUNCTIONS ================================= */
