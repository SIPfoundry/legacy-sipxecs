//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _STDINLISTENER_H
#define _STDINLISTENER_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsTask.h"

// DEFINES
// TYPEDEFS
// CONSTANTS

/// Class defining the interface for a service to receive messages from the supervisor
class StdinListener
{
public:
   virtual ~StdinListener() {} // nominal destructor

   /// Notify the owner that input has been received
   virtual void gotInput(UtlString& stdinMsg) {};
};

/// Task which listens for input on the process's stdin and reports it to the Listener
class StdinListenerTask : public OsTask
{
public:
   StdinListenerTask(StdinListener* owner) :
      OsTask("StdinListenerTask-%d"),
      mOwner(owner)
      {}

   int run(void *pArg);

   int getInput(UtlString* stdinMsg);

private:
   /// Read all input which is available on the specified fd.
   /// Returns -1 on error, or number of bytes read
   int readAll(int fd, UtlString* message);

   StdinListener* mOwner;
};

#endif // _STDINLISTENER_H
