//    
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "alarm/Alarm.h"
#include "SipxProcessFsm.h"
#include "SipxProcess.h"

// DEFINES
#define RETRY_TIMER_IN_SECONDS ( 10 * 60 )

// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

const SipxProcessFsm* SipxProcessFsm::GetParent( SipxProcess& impl ) const
{
   return 0;
}

const SipxProcessFsm* SipxProcessFsm::GetInitialState( SipxProcess& impl ) const
{
   return 0;
}

void SipxProcessFsm::DoEntryAction( SipxProcess& impl ) const
{
}

void SipxProcessFsm::DoExitAction( SipxProcess& impl ) const
{
}


void SipxProcessFsm::evShutdown( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_DEBUG,"'%s': Received event evShutdown while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
   
   ChangeState( impl, impl.pShutDown);
}

void SipxProcessFsm::evStartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evStartProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evStopProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evStopProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
   
   evProcessStopped(impl);
}

void SipxProcessFsm::evRestartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evRestartProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   

   ChangeState( impl, impl.pConfigurationMismatch );
}

void SipxProcessFsm::evRetryTimeout( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evRetryTimeout while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evProcessStarted( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evProcessStarted while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evProcessStopped( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evProcessStopped while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
   
   ChangeState( impl, impl.pFailed);
}

void SipxProcessFsm::evStopCompleted( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evStopCompleted while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evConfigurationChanged( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evConfigurationChanged while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evResourceCreated( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evResourceCreated while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evConfigurationVersionUpdated( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evConfigurationVersionUpdated while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evConfigTestPassed( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evConfigTestPassed while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::evConfigTestFailed( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evConfigTestFailed while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );   
}

void SipxProcessFsm::ChangeState( SipxProcess& impl, 
                                       const SipxProcessFsm* targetState ) const
{
    StateAlg::ChangeState( impl, this, targetState );
}


void Disabled::DoEntryAction( SipxProcess& impl ) const
{
   if (impl.isEnabled())
   {
      impl.enable();
   }
}

void Disabled::evStartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}


void ConfigurationMismatch::DoEntryAction( SipxProcess& impl ) const
{
   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
}

void ConfigurationMismatch::evConfigurationChanged( SipxProcess& impl ) const
{
   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
}

void ConfigurationMismatch::evConfigurationVersionUpdated( SipxProcess& impl ) const
{
   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
}


void ResourceRequired::DoEntryAction( SipxProcess& impl ) const
{
   if ( impl.resourcesAreReady() )
   {
      ChangeState( impl, impl.pTesting );
   }
}

void ResourceRequired::evResourceCreated( SipxProcess& impl ) const
{
   if ( impl.resourcesAreReady() )
   {
      ChangeState( impl, impl.pTesting );
   }
}


void Testing::DoEntryAction( SipxProcess& impl ) const
{
   impl.startConfigTest();
}

void Testing::evConfigTestPassed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pStarting );
}

void Testing::evConfigTestFailed( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,"'%s: configuration test failed", impl.name());

   ChangeState( impl, impl.pConfigTestFailed );
}


void ConfigTestFailed::evRestartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void ConfigTestFailed::evConfigurationChanged( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void ConfigTestFailed::evConfigurationVersionUpdated( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}


void Starting::DoEntryAction( SipxProcess& impl ) const
{
   impl.startProcess();
}

void Starting::evProcessStarted( SipxProcess& impl ) const
{
   if (impl.hadProcessFailed())
   {
      Alarm::raiseAlarm("PROCESS_RESTARTED", impl.data());
   }
   ChangeState( impl, impl.pRunning );
}


void Stopping::DoEntryAction( SipxProcess& impl ) const
{
   impl.stopProcess();
}

void Stopping::evProcessStopped( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s: process stopped, now wait for stop script", impl.name());
}


void Stopping::evStopCompleted( SipxProcess& impl ) const
{
   if (impl.isEnabled())
   {
      ChangeState( impl, impl.pStarting );
   }
   else
   {
      ChangeState( impl, impl.pDisabled );
   }
}


void Failed::DoEntryAction( SipxProcess& impl ) const
{
   impl.startRetryTimer();
}

void Failed::evRestartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void Failed::evStartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void Failed::evRetryTimeout( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void Failed::DoExitAction( SipxProcess& impl ) const
{
   impl.cancelRetryTimer();
}
   

void Running::DoEntryAction( SipxProcess& impl ) const
{
}

void Running::evStopProcess( SipxProcess& impl ) const
{
   //@TODO: wait for dependents to stop?
   ChangeState( impl, impl.pStopping );
}

void Running::evRestartProcess( SipxProcess& impl ) const
{
   //@TODO: wait for dependents to stop?
   ChangeState( impl, impl.pStopping );
}

void Running::evShutdown( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShuttingDown );
}

void Running::evProcessStopped( SipxProcess& impl ) const
{
   if (impl.isEnabled())
   {
      impl.processFailed();
      ChangeState( impl, impl.pFailed);
   }
   else
   {
      ChangeState( impl, impl.pDisabled);
   }
}


void ShuttingDown::DoEntryAction( SipxProcess& impl ) const
{
   impl.stopProcess();
}

void ShuttingDown::evProcessStopped( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShutDown );
}

void ShutDown::DoEntryAction( SipxProcess& impl ) const
{   
   impl.done();
}

