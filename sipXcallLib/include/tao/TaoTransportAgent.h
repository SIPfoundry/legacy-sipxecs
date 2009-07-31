//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoTransportAgent_h_
#define _TaoTransportAgent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsTask.h>
#include <os/OsServerTask.h>
#include <os/OsBSem.h>
#include "tao/TaoMessage.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class TaoTransportAgent : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   TaoTransportAgent(OsSocket* socket = NULL,
           const char* remoteHostName = NULL,
           const char* callId = NULL,
                const char* toField = NULL, const char* fromField = NULL);
     //:Default constructor

   TaoTransportAgent(OsSocket* pSocket, OsServerTask* pServer);
     //:Constructor

   virtual
   ~TaoTransportAgent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
        int send(TaoMessage& rMsg);

        virtual int run(void* pArg);

        virtual OsStatus setErrno(int errno);
         //:Set the errno status for the task
         // This call has no effect under Windows NT and, if the task has been
         // started, will always returns OS_SUCCESS

 /* ============================ ACCESSORS ================================= */

        void getHostIp(UtlString* hostAddress) const;
        void getAgentName(UtlString* pAgentName) const;
        void getCallId(UtlString* callId) const;
        void getToField(UtlString* toField) const;
        void getFromField(UtlString* fromField) const;
        long getLastTouchedTime() const;
        int getHostPort() const;
/* ============================ INQUIRY =================================== */

        UtlBoolean isOk();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        OsSocket*               mpSocket;
        OsServerTask*   mpServer;
        UtlString      viaName;
        UtlString      callIdLabel;
        UtlString      toFieldLabel;
        UtlString      fromFieldLabel;
        long touchedTime;
        OsBSem mWriteSem ;

   int readUntilDone(OsSocket* pSocket, char *pBuf, int iLength) ;
   //: read iLength bytes from passed socket (waiting until completion)


   TaoTransportAgent(const TaoTransportAgent& rTaoTransportAgent);
     //:disable Copy constructor

   TaoTransportAgent& operator=(const TaoTransportAgent& rhs);
     //:disable Assignment operator

};

#endif // _TaoTransportAgent_h_
