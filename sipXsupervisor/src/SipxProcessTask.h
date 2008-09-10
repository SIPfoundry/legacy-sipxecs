// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXPROCESSTASK_H_
#define _SIPXPROCESSTASK_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "os/OsServerTask.h"

#include "SipxProcess.h"

// DEFINES
// CONSTANTS
// FORWARD DECLARATIONS

// TYPEDEFS

class SipxProcessMsg : public OsMsg
{
   /// Events sent to the task

  SipxProcessMsg(SipxProcess::Event event) :
   OsMsg(USER_START, event)
   {
   };
};


/// Task for handling asyncronous events in process managment
/**
 * This provides the thread from which all actual manipulation of
 * the system process takes place.
 */
class SipxProcessTask : public OsServerTask
{
  public:

   /// constructor
   SipxProcessTask(const UtlString& name, SipxProcess* process);

   /// Spawn a new task and invoke its run() method..
   virtual UtlBoolean start(void);

   virtual int run(void* pArg);

   /// destructor
   virtual ~SipxProcessTask();

  protected:

  private:

   SipxProcess* mpProcess;
   
   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcessTask(const SipxProcessTask& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcessTask& operator=(const SipxProcessTask& noassignmentoperator);
   // @endcond     
};

#endif // _SIPXPROCESSTASK_H_
