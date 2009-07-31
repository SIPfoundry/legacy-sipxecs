//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _MyStreamQueueHistoryKeeper_h_
#define _MyStreamQueueHistoryKeeper_h_

#include "utl/UtlString.h"
#include "mp/MpQueuePlayerListener.h"

class MyStreamQueueHistoryKeeper : public MpQueuePlayerListener
{
 protected:
   UtlString mHistory ;
   UtlString mExpectedHistory;

 public:
   virtual ~MyStreamQueueHistoryKeeper(void);

   virtual void queuePlayerStarted();
   virtual void queuePlayerStopped();
   virtual void queuePlayerAdvanced();
   const char* getHistory();
};

#endif // MyStreamQueueHistoryKeeper_h_
