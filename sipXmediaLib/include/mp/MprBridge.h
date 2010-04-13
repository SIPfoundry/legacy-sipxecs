//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprBridge_h_
#define _MprBridge_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
//?#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"
#include "mp/MpConnection.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The conference bridge resource.

class MprBridge : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprBridge(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Default constructor

   virtual
   ~MprBridge();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   int connectPort(MpConnectionID connID);
     //:Attach MpConnection to an available port.

   OsStatus disconnectPort(MpConnectionID connID);
     //:disconnect MpConnection from its port.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum { MAX_BRIDGE_PORTS = 10 };

   MpConnectionID mpConnectionIDs[MAX_BRIDGE_PORTS];
   OsBSem         mPortLock;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprBridge(const MprBridge& rMprBridge);
     //:Copy constructor (not implemented for this class)

   MprBridge& operator=(const MprBridge& rhs);
     //:Assignment operator (not implemented for this class)

   int findFreePort(void);
     //:Find and return the index to an unused port pair

   UtlBoolean isPortActive(int portIdx) const;
     //:Check whether this port is connected to both input and output

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridge_h_
