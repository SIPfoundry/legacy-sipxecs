// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCESSTASK_H_
#define _PROCESSTASK_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "os/OsTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ProcessMsg : public OsMsg
{
   /// Events sent to the task
   typedef enum 
   {
      Startup,
      ConfigurationChange,
      ConfigurationVersionUpdate,
      CheckState
   } MonitorEvent;

  ProcessMsg(MonitorEvent event) :
   OsMsg(USER_START, event)
   {
   };
};


/// Task for handling asyncronous events in process managment
/**
 * This provides the thread from which all actual manipulation of
 * the system process takes place.
 */
class ProcessTask : public OsTask
{
  public:

   /// constructor
   ProcessTask(const UtlString& name);

   /// Spawn a new task and invoke its run() method..
   virtual UtlBoolean start(void);

   virtual int run(void* pArg);

   /// destructor
   virtual ~ProcessTask();

  protected:

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessTask(const ProcessTask& nocopyconstructor);

   /// There is no assignment operator.
   ProcessTask& operator=(const ProcessTask& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSTASK_H_
