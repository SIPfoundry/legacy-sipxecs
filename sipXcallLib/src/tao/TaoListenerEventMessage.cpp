//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "tao/TaoListenerEventMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TaoListenerEventMessage::TaoListenerEventMessage(PtEvent::PtEventId eventId,
                           intptr_t intData1,
                           intptr_t intData2,
                           intptr_t intData3,
                           const char* stringData1,
                           const char* stringData2,
                           const char* stringData3) :
 OsMsg(OsMsg::TAO_LISTENER_EVENT_MSG, eventId)
{
    if(stringData1) mStringData1.append(stringData1);
    if(stringData2) mStringData2.append(stringData2);
    if(stringData3) mStringData3.append(stringData3);
    mIntData1 = intData1;
    mIntData2 = intData2;
    mIntData3 = intData3;

        mEventId = eventId;
}

// Copy constructor
 TaoListenerEventMessage::TaoListenerEventMessage(const TaoListenerEventMessage& rTaoListenerEventMessage) :
OsMsg(rTaoListenerEventMessage)
{
    mStringData1 = rTaoListenerEventMessage.mStringData1;
    mStringData2 = rTaoListenerEventMessage.mStringData2;
    mStringData3 = rTaoListenerEventMessage.mStringData3;
    mIntData1 = rTaoListenerEventMessage.mIntData1;
    mIntData2 = rTaoListenerEventMessage.mIntData2;
    mIntData3 = rTaoListenerEventMessage.mIntData3;

        mEventId = rTaoListenerEventMessage.mEventId;
}

// Destructor
TaoListenerEventMessage::~TaoListenerEventMessage()
{
    mStringData1.remove(0);
    mStringData2.remove(0);
    mStringData3.remove(0);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
TaoListenerEventMessage&
TaoListenerEventMessage::operator=(const TaoListenerEventMessage& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);

    mStringData1 = rhs.mStringData1;
    mStringData2 = rhs.mStringData2;
    mStringData3 = rhs.mStringData3;
    mIntData1 = rhs.mIntData1;
    mIntData2 = rhs.mIntData2;
    mIntData3 = rhs.mIntData3;
        mEventId = rhs.mEventId;
   return *this;
}

void TaoListenerEventMessage::setStringData1(const char* stringData)
{
    mStringData1.remove(0);
    if(stringData) mStringData1.append(stringData);
}

void TaoListenerEventMessage::setStringData2(const char* stringData)
{
    mStringData2.remove(0);
    if(stringData) mStringData2.append(stringData);
}

void TaoListenerEventMessage::setStringData3(const char* stringData)
{
    mStringData3.remove(0);
    if(stringData) mStringData3.append(stringData);
}

void TaoListenerEventMessage::setIntData1(intptr_t intData)
{
    mIntData1 = intData;
}

void TaoListenerEventMessage::setIntData2(intptr_t intData)
{
    mIntData2 = intData;
}

void TaoListenerEventMessage::setIntData3(intptr_t intData)
{
    mIntData3 = intData;
}

/* ============================ ACCESSORS ================================= */

void TaoListenerEventMessage::getStringData1(UtlString& stringData)
{
    if (stringData && !mStringData1.isNull())
        {
                stringData.remove(0);
                stringData.append(mStringData1.data());
        }
}

void TaoListenerEventMessage::getStringData2(UtlString& stringData)
{
    if (stringData && !mStringData2.isNull())
        {
                stringData.remove(0);
                stringData.append(mStringData2.data());
        }
}

void TaoListenerEventMessage::getStringData3(UtlString& stringData)
{
    if (stringData && !mStringData3.isNull())
        {
                stringData.remove(0);
                stringData.append(mStringData3.data());
        }
}


intptr_t TaoListenerEventMessage::getIntData1()
{
    return(mIntData1);
}

intptr_t TaoListenerEventMessage::getIntData2()
{
    return(mIntData2);
}

intptr_t TaoListenerEventMessage::getIntData3()
{
    return(mIntData3);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
