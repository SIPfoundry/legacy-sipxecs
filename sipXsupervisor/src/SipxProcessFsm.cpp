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
   OsSysLog::add(FAC_SUPERVISOR,PRI_INFO,"'%s': Received event evStopProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );

   ChangeState( impl, impl.pStopping);
}

void SipxProcessFsm::evRestartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_INFO,"'%s': Received event evRestartProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );

   ChangeState( impl, impl.pConfigurationMismatch );
}

void SipxProcessFsm::evTimeout( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': Received unexpected event evTimeout while in state '%s'",
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
   OsSysLog::add(FAC_SUPERVISOR,PRI_INFO,"'%s': Received event evConfigurationChanged while in state '%s', ignored",
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
   impl.checkThreadId();
   StateAlg::ChangeState( impl, this, targetState );
}


void Disabled::DoEntryAction( SipxProcess& impl ) const
{
   impl.checkThreadId();
   impl.clearStatusMessages();

   if (impl.isEnabled())
   {
      impl.enable();
   }
}

void Disabled::evStartProcess( SipxProcess& impl ) const
{
   // kill off any previously running process before starting up
   ChangeState( impl, impl.pStopping );
}

void Disabled::evRestartProcess( SipxProcess& impl ) const
{
   if (impl.isEnabled())
   {
      ChangeState( impl, impl.pConfigurationMismatch );
   }
}

void Disabled::evStopProcess( SipxProcess& impl ) const
{
   // NOP
}

void Disabled::evProcessStopped( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': process stopped in state '%s', ignored",
                 impl.name(), impl.GetCurrentState()->name());
}

void Disabled::evShutdown( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShutDown );
}


void ConfigurationMismatch::DoEntryAction( SipxProcess& impl ) const
{
   impl.checkThreadId();
   impl.clearStatusMessages();

   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
   else
   {
      impl.startDelayReportingTimer();
   }
}

void ConfigurationMismatch::evConfigurationChanged( SipxProcess& impl ) const
{
   impl.clearStatusMessages();
   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
}

void ConfigurationMismatch::evConfigurationVersionUpdated( SipxProcess& impl ) const
{
   impl.clearStatusMessages();
   if ( impl.configurationVersionMatches() )
   {
      ChangeState( impl, impl.pResourceRequired );
   }
}

void ConfigurationMismatch::evRestartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void ConfigurationMismatch::evTimeout( SipxProcess& impl ) const
{
   // if we're still in this state after the timeout, raise an alarm
   impl.processBlocked("PROCESS_CONFIG_MISMATCH");
}

void ConfigurationMismatch::DoExitAction( SipxProcess& impl ) const
{
   impl.cancelTimer();
}

void ResourceRequired::DoEntryAction( SipxProcess& impl ) const
{
   impl.checkThreadId();
   impl.clearStatusMessages();

   if ( impl.resourcesAreReady() )
   {
      ChangeState( impl, impl.pTesting );
   }
   else
   {
      impl.startDelayReportingTimer();
   }
}

void ResourceRequired::evConfigurationChanged( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void ResourceRequired::evRestartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void ResourceRequired::evTimeout( SipxProcess& impl ) const
{
   // if we're still in this state after the timeout, raise an alarm
   impl.processBlocked("PROCESS_RESOURCE_REQUIRED");
}

void ResourceRequired::DoExitAction( SipxProcess& impl ) const
{
   impl.cancelTimer();
}


void Testing::DoEntryAction( SipxProcess& impl ) const
{
   impl.checkThreadId();
   impl.clearStatusMessages();

   impl.startConfigTest();
}

void Testing::evConfigTestPassed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pStarting );
}

void Testing::evConfigTestFailed( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,"'%s': configuration test failed", impl.name());

   ChangeState( impl, impl.pConfigTestFailed );
}

void Testing::evRestartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': aborting configtest in order to restart",
                 impl.name());
   ChangeState( impl, impl.pStoppingConfigtestToRestart );
   impl.killConfigTest();
}

void Testing::evShutdown( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': aborting configtest in order to shutdown",
                 impl.name());
   ChangeState( impl, impl.pShuttingDown );
   impl.killConfigTest();
}

void StoppingConfigtestToRestart::evConfigTestPassed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void StoppingConfigtestToRestart::evConfigTestFailed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pConfigurationMismatch );
}

void StoppingConfigtestToRestart::evRestartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': already aborting configtest in order to restart",
                 impl.name());
}

void ConfigTestFailed::DoEntryAction( SipxProcess& impl ) const
{
   impl.processBlocked("PROCESS_CONFIGTEST_FAILED");
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
   impl.checkThreadId();
   impl.startProcess();
}

void Starting::evProcessStarted( SipxProcess& impl ) const
{
   if (impl.hadProcessFailed())
   {
      Alarm::raiseAlarm("PROCESS_RESTARTED", impl.data());
   }
   else if (impl.hadProcessBlocked())
   {
      Alarm::raiseAlarm("PROCESS_STARTED", impl.data());
      impl.clearProcessBlocked();
   }
   ChangeState( impl, impl.pRunning );
}


void Stopping::DoEntryAction( SipxProcess& impl ) const
{
   impl.checkThreadId();
   impl.startStopTimer();
   impl.stopProcess();
}

void Stopping::evTimeout( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_WARNING,"'%s': process did not stop after 120 seconds, trying kill",
         impl.name() );
   impl.killProcess();
}

void Stopping::DoExitAction( SipxProcess& impl ) const
{
   impl.cancelTimer();
}

void Stopping::evRestartProcess( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR,PRI_INFO,"'%s': Retrying event evRestartProcess while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );
   ChangeState( impl, impl.pStopping );
}

// We need both the actual process, and the script which stops it, to finish
// before we move on.  These events can come in any order.
void Stopping::evProcessStopped( SipxProcess& impl ) const
{
   impl.checkThreadId();
   if (impl.isCompletelyStopped())
   {
      if (impl.isEnabled())
      {
         ChangeState( impl, impl.pConfigurationMismatch );
      }
      else
      {
         ChangeState( impl, impl.pDisabled );
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': process stopped, now wait for stop script", impl.name());
   }
}


void Stopping::evStopCompleted( SipxProcess& impl ) const
{
   impl.checkThreadId();
   if (impl.isCompletelyStopped())
   {
      if (impl.isEnabled())
      {
         ChangeState( impl, impl.pConfigurationMismatch );
      }
      else
      {
         ChangeState( impl, impl.pDisabled );
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': stop completed, now wait for process stopped", impl.name());
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

void Failed::evTimeout( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pStopping );
}

void Failed::DoExitAction( SipxProcess& impl ) const
{
   impl.cancelTimer();
}


void Running::DoEntryAction( SipxProcess& impl ) const
{
   impl.clearStatusMessages();
   impl.notifyProcessRunning();
}

void Running::evConfigurationChanged( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,"'%s': configuration changed in state '%s', ignored",
                 impl.name(), impl.GetCurrentState()->name());
}

void Running::evStopProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pStopping );
}

void Running::evRestartProcess( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pStopping );
}

void Running::evShutdown( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShuttingDown );
}

void Running::evProcessStopped( SipxProcess& impl ) const
{
   impl.checkThreadId();
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

// We need both the actual process, and the script which stops it, to finish
// before we move on.  These events can come in any order.
void ShuttingDown::evProcessStopped( SipxProcess& impl ) const
{
   if (impl.isCompletelyStopped())
   {
      ChangeState( impl, impl.pShutDown );
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': process stopped, now wait for stop script", impl.name());
   }
}


void ShuttingDown::evStopCompleted( SipxProcess& impl ) const
{
   if (impl.isCompletelyStopped())
   {
      ChangeState( impl, impl.pShutDown );
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': stop completed, now wait for process stopped", impl.name());
   }
}

void ShuttingDown::evConfigTestPassed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShutDown );
}

void ShuttingDown::evConfigTestFailed( SipxProcess& impl ) const
{
   ChangeState( impl, impl.pShutDown );
}

void ShuttingDown::evShutdown( SipxProcess& impl ) const
{
   // NOP
}

void ShutDown::DoEntryAction( SipxProcess& impl ) const
{
   impl.done();
}

void ShutDown::evStopCompleted( SipxProcess& impl ) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s': evStopCompleted in state '%s', ignored",
                 impl.name(), impl.GetCurrentState()->name());
}

void ShutDown::evShutdown( SipxProcess& impl ) const
{
   // NOP
}
