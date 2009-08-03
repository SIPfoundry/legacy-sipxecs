//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "os/OsConnectionSocket.h"
#include "tao/TaoProviderListener.h"
#include "tao/TaoTransportTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoProviderListener::TaoProviderListener(PtEventMask* pMask)
: PtProviderListener(pMask)
{
        mTerminalName = 0;
        mpConnectionSocket = 0;
}

TaoProviderListener::TaoProviderListener(TaoObjHandle objId,
                                                                                 TaoObjHandle clientSocket,
                                                                                 TaoTransportTask* pSvrTransport,
                                                                                 const char * terminalName)
: PtProviderListener(NULL)
{
        mObjId = objId;
        mhClientSocket = clientSocket;
        mpSvrTransport = pSvrTransport;
        int len = strlen(terminalName);
        if (len > 0)
        {
                mTerminalName = new char[len + 1];
                strcpy(mTerminalName, terminalName);
        }
        else
        {
                mTerminalName = 0;
        }

        mpConnectionSocket = new OsConnectionSocket(DEF_TAO_EVENT_PORT, mTerminalName);

}

TaoProviderListener::~TaoProviderListener()
{
        if (mTerminalName)
        {
                delete[] mTerminalName;
                mTerminalName = 0;
        }

        if (mpConnectionSocket)
        {
                delete mpConnectionSocket;
                mpConnectionSocket = 0;
        }
}
