//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIGNALTASK_H_
#define _SIGNALTASK_H_

// SYSTEM INCLUDES
#include <signal.h>
#include <sys/poll.h>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// DEFINES
// TYPEDEFS

// CONSTANTS

class ChildSignalTask : public OsTask
{
public:
   ChildSignalTask() : OsTask("ChildSignalTask-%d"), mShutdownFlag(false) {}

   int run(void *pArg);

   void       setShutdownFlag(UtlBoolean state) {mShutdownFlag = state;}
   UtlBoolean getShutdownFlag() {return mShutdownFlag;}

private:
   UtlBoolean mShutdownFlag;
} ;

#endif // _SIGNALTASK_H_
