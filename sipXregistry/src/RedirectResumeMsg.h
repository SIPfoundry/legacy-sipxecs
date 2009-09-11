//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef REDIRECTRESUMEMSG_H
#define REDIRECTRESUMEMSG_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Message to the redirect server to resume processing a redirection request.
class RedirectResumeMsg : public OsMsg
{
public:

   /** Message type code.
    */
   static const int REDIRECT_RESTART = USER_START;

   /** Construct a message saying that redirector redirectorNo is willing to
    * reprocess request seqNo.
    */
   RedirectResumeMsg(RedirectPlugin::RequestSeqNo seqNo,
                     int redirectorNo);

   /**
    * Copy this message.
    */
   virtual RedirectResumeMsg* createCopy(void) const;

   /** Get the sequence number
    */
   inline RedirectPlugin::RequestSeqNo getRequestSeqNo() const
   {
      return mSeqNo;
   }

   /** Get the redirector number
    */
   inline int getRedirectorNo() const
   {
      return mRedirectorNo;
   }

private:

   /** Sequence number of the request
    */
   RedirectPlugin::RequestSeqNo mSeqNo;

   /** Number of the redirector
    */
   int mRedirectorNo;
};

#endif /*  REDIRECTRESUMEMSG_H */
