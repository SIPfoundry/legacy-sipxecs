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

#include "tao/TaoEventDispatcher.h"
#include "tao/TaoMessage.h"
#include "tao/TaoObject.h"
#include "tao/TaoEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoEventDispatcher::TaoEventDispatcher(const UtlString& name, const int maxRequestQMsgs)
        : OsServerTask(name, NULL, maxRequestQMsgs)
{
        mTaoObjHandle = 0; // NULL
        mpProviderEvent = new TaoEvent(0);
}

TaoEventDispatcher::~TaoEventDispatcher()
{
        if (mpProviderEvent)
        {
                delete mpProviderEvent;
                mpProviderEvent = 0;
        }
}

UtlBoolean TaoEventDispatcher::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (rMsg.getMsgSubType())
        {
        case TaoMessage::RESPONSE_ADDRESS:
        case TaoMessage::RESPONSE_CALL:
        case TaoMessage::RESPONSE_CONNECTION:
        case TaoMessage::RESPONSE_PROVIDER:
        case TaoMessage::RESPONSE_TERMCONNECTION:
        case TaoMessage::RESPONSE_TERMINAL:
        case TaoMessage::RESPONSE_PHONECOMPONENT:
                {
                        TaoObjHandle handle = ((TaoMessage &)rMsg).getTaoObjHandle();
                        int data = ((TaoMessage &)rMsg).getArgCnt();
                        mpProviderEvent->setIntData(data);
                        data = ((TaoMessage &)rMsg).getCmd();
                        mpProviderEvent->setIntData2(data);
                        UtlString argList = ((TaoMessage &)rMsg).getArgList();
                        // need to convert to ints and find a way to pass to the app
                        argList.index(TAOMESSAGE_DELIMITER, 0);
                        mpProviderEvent->setStringData(argList);
                        mpProviderEvent->signal(handle);
                        handled = TRUE;
                        break;
                }
        case TaoMessage::UNSPECIFIED:
        default:
        //assert(FALSE);
            break;
        }

        return handled;
}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus TaoEventDispatcher::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}
