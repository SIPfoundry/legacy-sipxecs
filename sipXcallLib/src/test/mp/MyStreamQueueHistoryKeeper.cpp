//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include "test/mp/MyStreamQueueHistoryKeeper.h"

MyStreamQueueHistoryKeeper::~MyStreamQueueHistoryKeeper()
{
}

void MyStreamQueueHistoryKeeper::queuePlayerStarted()
{
    mHistory.append("queuePlayerStarted ") ;
}


void MyStreamQueueHistoryKeeper::queuePlayerStopped()
{
    mHistory.append("queuePlayerStopped ") ;
}


void MyStreamQueueHistoryKeeper::queuePlayerAdvanced()
{
    mHistory.append("queuePlayerAdvanced ") ;
}

const char* MyStreamQueueHistoryKeeper::getHistory()
{
    return mHistory.data() ;
}
