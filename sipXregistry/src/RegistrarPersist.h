//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARPERSIST_H_
#define _REGISTRARPERSIST_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsServerTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class SipRegistrarServer;


/**
 * This thread is responsible for periodically garbage-collecting and
 * persisting the registration DB.
 */
class RegistrarPersist : public OsServerTask
{
public:
   /// Constructor
   RegistrarPersist(SipRegistrar& registrar);

   /// Destructor
   virtual ~RegistrarPersist();

   /// Catch the shutdown request so that we can flush the database.
   virtual void requestShutdown(void);

   /// Start the timer that triggers garbage collection and DB persistence, if it's not running
   void scheduleCleanAndPersist();

protected:
   /// Default persist interval, in units of seconds
   static const int SIP_REGISTRAR_DEFAULT_PERSIST_INTERVAL;

   /// Mutex must be locked with OsLock to access any shared member variables
   OsBSem mLock;

   bool mIsTimerRunning;

   OsTimer mPersistTimer;

   /// The top level task - do not delete
   SipRegistrar& mRegistrar;

   /// Time interval for garbage collection and persistence
   int mPersistInterval;

   /// handle the expiration of the persist timer
   UtlBoolean handleMessage(OsMsg& eventMessage);    ///< Timer expiration msg

   /// Garbage-collection and persist the registration DB
   void cleanAndPersist();

   SipRegistrarServer& getRegistrarServer();

private:
   /// There is no copy constructor
   RegistrarPersist(const RegistrarPersist&);

   /// There is no assignment operator
   RegistrarPersist& operator=(const RegistrarPersist&);
};

#endif // _REGISTRARPERSIST_H_
