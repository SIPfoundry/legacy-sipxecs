//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
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

// APPLICATION INCLUDES
#include <net/SipMessageEvent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipMessageEvent::SipMessageEvent(SipMessage* message, int status) :
OsMsg(OsMsg::PHONE_APP, SipMessage::NET_SIP_MESSAGE)
{
   messageStatus = status;
   sipMessage = message;
}

// Destructor
SipMessageEvent::~SipMessageEvent()
{
        if(sipMessage)
        {
                delete sipMessage;
                sipMessage = NULL;
        }
}

OsMsg* SipMessageEvent::createCopy() const
{
        // Ineffient but easy coding way to copy message
        SipMessage* sipMsg = NULL;

        if(sipMessage)
        {
                sipMsg = new SipMessage(*sipMessage);
        }

        return(new SipMessageEvent(sipMsg, messageStatus));
}
/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipMessageEvent&
SipMessageEvent::operator=(const SipMessageEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);
        messageStatus = rhs.messageStatus;
        if(sipMessage)
        {
                delete sipMessage;
                sipMessage = NULL;
        }

        if(rhs.sipMessage)
        {
                sipMessage = new SipMessage(*(rhs.sipMessage));
        }

   return *this;
}

/* ============================ ACCESSORS ================================= */

/// Set the SipMessage pointer in the SipMessageEvent.
void SipMessageEvent::setMessage(SipMessage* message)
{
   sipMessage = message;
}

const SipMessage* SipMessageEvent::getMessage()
{
        return(sipMessage);
}

void SipMessageEvent::setMessageStatus(int status)
{
        messageStatus = status;
}

int SipMessageEvent::getMessageStatus() const
{
        return(messageStatus);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
