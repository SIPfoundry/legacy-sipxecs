//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SipxProcessFsmS_H_
#define _SipxProcessFsmS_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#define LOG_STATE_CHANGES
#include "utl/UtlFsm.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxProcess;

class SipxProcessFsm
{
public:
   /***
    * Returns the parent state of this state. As required by StateAlg.
    */
   virtual const SipxProcessFsm* GetParent( SipxProcess& impl ) const;

   /**
    * Returns the initial state contained in this state. As required by StateAlg
    */
   virtual const SipxProcessFsm* GetInitialState( SipxProcess& impl ) const;

   /**
    * Called whenever this state is entered. As required by StateAlg
    */
   virtual void DoEntryAction( SipxProcess& impl ) const;

   /**
    * Called whenever this state is exited. As required by StateAlg
    */
   virtual void DoExitAction( SipxProcess& impl ) const;

   virtual const char* name( void ) const {return "SipxProcessFsm";}

   virtual ~SipxProcessFsm(){};

   // State machine events
   virtual void evConfigurationChanged( SipxProcess& impl ) const;
   virtual void evConfigurationVersionUpdated( SipxProcess& impl ) const;
   virtual void evResourceCreated( SipxProcess& impl ) const;
   virtual void evConfigTestPassed( SipxProcess& impl ) const;
   virtual void evConfigTestFailed( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evStartProcess( SipxProcess& impl ) const;
   virtual void evStopProcess( SipxProcess& impl ) const;
   virtual void evTimeout( SipxProcess& impl ) const;
   virtual void evProcessStarted( SipxProcess& impl ) const;
   virtual void evProcessStopped( SipxProcess& impl ) const;
   virtual void evStopCompleted( SipxProcess& impl ) const;


protected:
   void ChangeState( SipxProcess& impl,
                      const SipxProcessFsm* targetState ) const;

};

class Undefined: public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Undefined";}
   virtual ~Undefined(){};

};


class Disabled : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Disabled";}
   virtual ~Disabled(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evStartProcess( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evStopProcess( SipxProcess& impl ) const;
   virtual void evProcessStopped( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
};

class ConfigurationMismatch: public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "ConfigurationMismatch";}
   virtual ~ConfigurationMismatch(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evConfigurationChanged( SipxProcess& impl ) const;
   virtual void evConfigurationVersionUpdated( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evTimeout( SipxProcess& impl ) const;
   virtual void DoExitAction( SipxProcess& impl ) const;
};

class ResourceRequired : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "ResourceRequired";}
   virtual ~ResourceRequired(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evConfigurationChanged( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evTimeout( SipxProcess& impl ) const;
   virtual void DoExitAction( SipxProcess& impl ) const;
};


class Testing : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Testing";}
   virtual ~Testing(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evConfigTestPassed( SipxProcess& impl ) const;
   virtual void evConfigTestFailed( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
};

class StoppingConfigtestToRestart : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "StoppingConfigtestToRestart";}
   virtual ~StoppingConfigtestToRestart(){};

   // State machine events relevant for this state
   virtual void evConfigTestPassed( SipxProcess& impl ) const;
   virtual void evConfigTestFailed( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
};

class ConfigTestFailed: public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "ConfigurationTestFailed";}
   virtual ~ConfigTestFailed(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evConfigurationChanged( SipxProcess& impl ) const;
   virtual void evConfigurationVersionUpdated( SipxProcess& impl ) const;
};

class Starting : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Starting";}
   virtual ~Starting(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evProcessStarted( SipxProcess& impl ) const;
};


class Stopping : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Stopping";}
   virtual ~Stopping(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void DoExitAction( SipxProcess& impl ) const;
   virtual void evTimeout( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evProcessStopped( SipxProcess& impl ) const;
   virtual void evStopCompleted( SipxProcess& impl ) const;
};


class Failed : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Failed";}
   virtual ~Failed(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void DoExitAction( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evTimeout( SipxProcess& impl ) const;
   virtual void evStartProcess( SipxProcess& impl ) const;
};

class Running : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "Running";}
   virtual ~Running(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evConfigurationChanged( SipxProcess& impl ) const;
   virtual void evRestartProcess( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
   virtual void evStopProcess( SipxProcess& impl ) const;
   virtual void evProcessStopped( SipxProcess& impl ) const;
};


class ShuttingDown : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "ShuttingDown";}
   virtual ~ShuttingDown(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evProcessStopped( SipxProcess& impl ) const;
   virtual void evStopCompleted( SipxProcess& impl ) const;
   virtual void evConfigTestPassed( SipxProcess& impl ) const;
   virtual void evConfigTestFailed( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
};


class ShutDown : public SipxProcessFsm
{
public:
   virtual const char* name( void ) const {return "ShutDown";}
   virtual ~ShutDown(){};

   // State machine events relevant for this state
   virtual void DoEntryAction( SipxProcess& impl ) const;
   virtual void evStopCompleted( SipxProcess& impl ) const;
   virtual void evShutdown( SipxProcess& impl ) const;
};

#endif // _SipxProcessFsmS_H_




