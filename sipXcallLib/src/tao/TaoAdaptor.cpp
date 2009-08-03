//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include <assert.h>

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "tao/TaoAdaptor.h"
#include "tao/TaoTransportTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoAdaptor::TaoAdaptor(const UtlString& name,
                                           const int maxRequestQMsgs) :
OsServerTask(name, NULL, maxRequestQMsgs)
{
}

TaoAdaptor::~TaoAdaptor()
{
}

UtlBoolean TaoAdaptor::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled;

   handled = FALSE;

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_SHUTDOWN:
      handled = TRUE;
      break;
   default:
      assert(FALSE);
      break;
   }

   return handled;
}

void TaoAdaptor::parseMessage(TaoMessage& rMsg)
{
        mCmd                    = rMsg.getCmd();
        mMsgID                  = rMsg.getMsgID();
        mObjId                  = rMsg.getTaoObjHandle();
        mClientSocket   = rMsg.getSocket();
        mArgList                = rMsg.getArgList();
        mArgCnt                 = rMsg.getArgCnt();

}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus TaoAdaptor::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}
