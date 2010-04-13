//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MpFlowGraphMsg_h_
#define _MpFlowGraphMsg_h_

// SYSTEM INCLUDES
#include <stdint.h>

// APPLICATION INCLUDES
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpResource;

//:Message object used to communicate with the media processing task
class MpFlowGraphMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Phone set message types
   enum MpFlowGraphMsgType
   {
      FLOWGRAPH_ADD_LINK,
      FLOWGRAPH_ADD_RESOURCE,
      FLOWGRAPH_DESTROY_RESOURCES,
      FLOWGRAPH_DISABLE,
      FLOWGRAPH_ENABLE,
      FLOWGRAPH_PROCESS_FRAME,
      FLOWGRAPH_REMOVE_CONNECTION,
      FLOWGRAPH_REMOVE_LINK,
      FLOWGRAPH_REMOVE_RESOURCE,
      FLOWGRAPH_SET_SAMPLES_PER_FRAME,
      FLOWGRAPH_SET_SAMPLES_PER_SEC,
      FLOWGRAPH_START,
      FLOWGRAPH_STOP,
      RESOURCE_DISABLE,
      RESOURCE_ENABLE,
      RESOURCE_SET_SAMPLES_PER_FRAME,
      RESOURCE_SET_SAMPLES_PER_SEC,

      FLOWGRAPH_START_PLAY,
      FLOWGRAPH_START_TONE,
      FLOWGRAPH_START_RECORD,

      FLOWGRAPH_STOP_PLAY,
      FLOWGRAPH_STOP_TONE,
      FLOWGRAPH_STOP_RECORD,

      FLOWGRAPH_SYNCHRONIZE,

      FLOWGRAPH_SET_PREMIUM_SOUND,

      FLOWGRAPH_SET_DTMF_NOTIFY,

      RESOURCE_SPECIFIC_START = 100     // start of resource-specific messages
   };

/* ============================ CREATORS ================================== */

   MpFlowGraphMsg(int msg, MpResource* pMsgDest=NULL,
                  void* pPtr1=NULL, void* pPtr2=NULL,
                  int int1=-1, int int2=-1);
     //:Constructor

   MpFlowGraphMsg(const MpFlowGraphMsg& rMpFlowGraphMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~MpFlowGraphMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MpFlowGraphMsg& operator=(const MpFlowGraphMsg& rhs);
     //:Assignment operator

   void setMsgDest(MpResource* pMsgDest);
     //:Sets the intended recipient for this message.  Setting the message
     //:destination to NULL indicates that the message is intended for the
     //:flow graph itself.

   void setPtr1(void* p);
     //:Sets pointer 1 (void*) of the media flow graph message

   void setPtr2(void* p);
     //:Sets pointer 2 (void*) of the media flow graph message

   void setInt1(int i);
     //:Sets integer 1 of the media flow graph message

   void setInt2(int i);
     //:Sets integer 2 of the media flow graph message

/* ============================ ACCESSORS ================================= */

   int getMsg(void) const;
     //:Returns the type of the media flow graph message

   MpResource* getMsgDest(void) const;
     //:Returns the MpResource object that is the intended recipient for this
     //:message.  A NULL return indicates that the message is intended for
     //:the flow graph itself.

   void* getPtr1(void) const;
     //:Return pointer 1 (void*) of the media flow graph message

   void* getPtr2(void) const;
     //:Return pointer 2 (void*) of the media flow graph message

   int getInt1(void) const;
     //:Return integer 1 of the media flow graph message

   int getInt2(void) const;
     //:Return integer 2 of the media flow graph message

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpResource* mpMsgDest; // Intended recipient for this message
   void*       mpPtr1;    // Message pointer 1
   void*       mpPtr2;    // Message pointer 2
   int         mInt1;     // Message integer 1
   int         mInt2;     // Message integer 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphMsg_h_
