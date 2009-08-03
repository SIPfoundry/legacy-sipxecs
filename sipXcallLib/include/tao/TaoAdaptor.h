//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoAdaptor_h_
#define _TaoAdaptor_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "tao/TaoListenerManager.h"
#include "tao/TaoMessage.h"
//#include "tao/TaoReference.h"
//#include "TaoDefs.h"  // Added by ClassView

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Abstract event handler for processing call management event
/*! This object must be sub-classed to implement the handleMessage
 * method that will process the incoming call manager events.
 */
class TaoAdaptor : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoAdaptor(const UtlString& name = "TaoAdaptor-%d",
                                const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoAdaptor(const TaoAdaptor& rTaoAdaptor);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoAdaptor();

/* ============================ MANIPULATORS ============================== */

         //!Handle an incoming message.
         /*! If the message is not one that the object is prepared to process,
          * the handleMessage() method in the derived class should return FALSE
          * which will cause the OsServerTask::handleMessage() method to be
          * invoked on the message.
      */
        virtual UtlBoolean handleMessage(OsMsg& rMsg);

        virtual void setListenerManager(TaoListenerManager*& rpListenerMgr)
                        { mpListenerMgr = rpListenerMgr; };

        virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        virtual OsStatus setErrno(int errno);
         //:Set the errno status for the task
         // This call has no effect under Windows NT and, if the task has been
         // started, will always returns OS_SUCCESS

/* //////////////////////////// PROTECTED //////////////////////////////////// */
protected:

        unsigned char   mCmd;
        TaoObjHandle    mMsgID;
        TaoObjHandle    mObjId;
        TaoObjHandle    mClientSocket;
        UtlString               mArgList;
        int                             mArgCnt;

        TaoListenerManager*     mpListenerMgr;

private:


};

#endif // _TaoAdaptor_h_
