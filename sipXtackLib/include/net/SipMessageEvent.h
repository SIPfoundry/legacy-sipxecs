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

#ifndef _SipMessageEvent_h_
#define _SipMessageEvent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/** OsMsg subclass to carry a SipMessage and a status code describing the origin
 *  and context of the message.
 */
class SipMessageEvent : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   enum MessageStatusTypes
   {
      APPLICATION = 0,
      TRANSPORT_ERROR = 1,
      AUTHENTICATION_RETRY,
      SESSION_REINVITE_TIMER,
      TRANSACTION_RESEND,
      TRANSACTION_EXPIRATION
   };

/* ============================ CREATORS ================================== */

   SipMessageEvent(SipMessage* message = NULL, int status = APPLICATION);
   //:Default constructor

   virtual
      ~SipMessageEvent();
   //:Destructor

   virtual OsMsg* createCopy(void) const;
/* ============================ MANIPULATORS ============================== */

/// Set the SipMessage pointer in the SipMessageEvent.
//  setMessage(NULL) effectively detaches the SipMessage from the SipMessageEvent.
   void setMessage(SipMessage* message);

/* ============================ ACCESSORS ================================= */

   const SipMessage* getMessage();
   void setMessageStatus(int status);
   int getMessageStatus() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   SipMessage* sipMessage;
   int messageStatus;

   SipMessageEvent(const SipMessageEvent& rSipMessageEvent);
   //:disable Copy constructor

   SipMessageEvent& operator=(const SipMessageEvent& rhs);
   //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipMessageEvent_h_
