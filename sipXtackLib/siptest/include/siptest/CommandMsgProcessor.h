//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _CommandMsgProcessor_h_
#define _CommandMsgProcessor_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CommandMsgProcessor : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CommandMsgProcessor(SipUserAgent* userAgent = NULL);
     //:Default constructor

   virtual
   ~CommandMsgProcessor();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


        virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */

        void stopResponding();
        void startResponding(int responseCode, const char* responseText,
                                                 int numMessagesToRespondTo = -1);

        void startResponding(const char* filename, int numMessagesToRespondTo = -1 );
    void resendLastResponse();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        SipUserAgent* userAgent;
        int numRespondToMessages;
        int responseStatusCode;
        UtlString responseStatusText;
    SipMessage* mpResponseMessage;
        SipMessage* mpLastResponseMessage;

        CommandMsgProcessor& operator=(const CommandMsgProcessor& rhs);
        //:Assignment operator

        CommandMsgProcessor(const CommandMsgProcessor& rCommandMsgProcessor);
        //:Copy constructor

#ifdef TEST
   static bool sIsTested;
     //:Set to true after the tests for this class have been executed once

   void test();
     //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CommandMsgProcessor_h_
