// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxProcessTask.h"

// DEFINES
// CONSTANTS
const char* SipxProcessEventName[] =
{
   "Startup",
   "ConfigurationChange",
   "ConfigurationVersionUpdate",
   "CheckState",
   "ProcessRunning",
   "ProcessExit"
};

// TYPEDEFS
// FORWARD DECLARATIONS

const char* sipxProcessEventName(SipxProcess::Event event)
{
   const char* name;
   size_t check = static_cast<size_t>(event);
   if (check <= SipxProcess::ProcessExit)
   {
      name = SipxProcessEventName[event];
   }
   else
   {
      name = "OUT-OF-RANGE";
   }
   return name;
}

/// constructor
SipxProcessTask::SipxProcessTask(const UtlString& name, SipxProcess* process) :
   OsServerTask(name, process)
{
   
};


UtlBoolean handleMessage(OsMsg& rMsg)
{
   return false;                // @TODO 
}



/// destructor
SipxProcessTask::~SipxProcessTask()
{
};
