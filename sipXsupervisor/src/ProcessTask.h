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
#include "os/OsTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Task for handling asyncronous events in process managment
/**
 * 
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
