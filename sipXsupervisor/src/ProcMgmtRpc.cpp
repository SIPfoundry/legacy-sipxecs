//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlBool.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "utl/UtlHashMapIterator.h" 
#include "os/OsProcessMgr.h"
#include "os/OsSysLog.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "WatchDog.h"
#include "SipxProcessManager.h"
#include "SipxProcess.h"

#include "ProcMgmtRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/*****************************************************************
 **** ProcMgmtRpcMethod contains common code for ProcMgmtRpc methods
 *****************************************************************/
const char* ProcMgmtRpcMethod::METHOD_NAME = "ProcMgmtRpc.BASE";
const char* ProcMgmtRpcMethod::PARAM_NAME_CALLING_HOST = "callingHostname";
const char* ProcMgmtRpcMethod::PARAM_NAME_ALIAS = "alias";
const char* ProcMgmtRpcMethod::PARAM_NAME_BLOCK = "blockForStateChange";
const char* ProcMgmtRpcMethod::PARAM_NAME_PID = "pid";
const char* ProcMgmtRpcMethod::PARAM_NAME_SERVICE = "service";
const char* ProcMgmtRpcMethod::PARAM_NAME_SERVICE_VERSION = "serviceVersion";
int ProcMgmtRpcMethod::SINGLE_BLOCK_MAX = 15;
int ProcMgmtRpcMethod::LIST_BLOCK_MAX = 45;


XmlRpcMethod* ProcMgmtRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void ProcMgmtRpcMethod::registerSelf(WatchDog & watchdog)
{
   assert(false);  // this should have been overridden in the subclass
}

ProcMgmtRpcMethod::ProcMgmtRpcMethod()
{
}

void ProcMgmtRpcMethod::registerMethod(const char*       methodName,
                                       XmlRpcMethod::Get getMethod,
                                       WatchDog &        watchdog
                                       )
{
   watchdog.getXmlRpcDispatch()->addMethod(methodName, getMethod, &watchdog );
}

bool ProcMgmtRpcMethod::execute(const HttpRequestContext& requestContext,
                                UtlSList& params,
                                void* userData,
                                XmlRpcResponse& response,
                                ExecutionStatus& status
                                )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool ProcMgmtRpcMethod::validCaller(const HttpRequestContext& requestContext,
                                    const UtlString&          peerName,
                                    XmlRpcResponse&           response,
                                    const WatchDog&           watchdog,
                                    const char*               callingMethod
                                    )
{
   bool result = false;

   if (!peerName.isNull() && requestContext.isTrustedPeer(peerName))
   {
      // ssl says the connection is from the named host
      if (watchdog.isAllowedPeer(peerName))
      {
         // Watchdog says it is one of the allowed peers.
         result = true;
         OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                       "ProcMgmtRpcMethod::validCaller '%s' peer authenticated for %s",
                       peerName.data(), callingMethod
                       );
      }
      else
      {
         // this peer is authenticated, but not configured, so provide a good error response
         UtlString faultMsg;
         faultMsg.append("Unconfigured calling host '");
         faultMsg.append(peerName);
         faultMsg.append("'");
         response.setFault(ProcMgmtRpcMethod::UnconfiguredPeer, faultMsg.data());
            
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "%s failed - '%s' not a configured peer",
                       callingMethod, peerName.data()
                       );
      }
   }
   else
   {
      // ssl says not authenticated - provide only a generic error
      response.setFault(XmlRpcResponse::AuthenticationRequired, "TLS Peer Authentication Failure");
            
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                    "%s failed: '%s' failed SSL authentication",
                    callingMethod, peerName.data()
                    );
   }

   return result;
}

void ProcMgmtRpcMethod::handleMissingExecuteParam(const char* methodName,
                                                  const char* paramName,
                                                  XmlRpcResponse& response,
                                                  ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " '";
   faultMsg += paramName;
   faultMsg += "' parameter is missing or invalid type";
   status = XmlRpcMethod::FAILED;
   response.setFault(ProcMgmtRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

void ProcMgmtRpcMethod::handleExtraExecuteParam(const char* methodName,
                                                XmlRpcResponse& response,
                                                ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " has too many parameters";
   status = XmlRpcMethod::FAILED;
   response.setFault(ProcMgmtRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

bool ProcMgmtRpcMethod::executeSetUserRequestState(const HttpRequestContext& requestContext,
                                                   UtlSList& params,
                                                   void* userData,
                                                   XmlRpcResponse& response,
                                                   ExecutionStatus& status,
                                                   const int request_state
                                                   )
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
   {
      handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
   }
   else
   {
      UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

      if (!params.at(1) || !params.at(1)->isInstanceOf(UtlSList::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_ALIAS, response, status);
      }
      else
      {
         UtlSList* pAliasList = dynamic_cast<UtlSList*>(params.at(1));

         if (!params.at(2) || !params.at(2)->isInstanceOf(UtlBool::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_BLOCK, response, status);
         }
         else
         {
            UtlBool* pBlock = dynamic_cast<UtlBool*>(params.at(2));

            if (3 != params.entries())
            {
               handleExtraExecuteParam(name(), response, status);
            }
            else
            {
               WatchDog* pWatchDog = ((WatchDog *)userData);
               if (validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
               {
                  // Set the "user request state" of the specified processes to the specified
                  // state.  If successful, then possibly also wait for the state change.
                  UtlString requestedState;
                  if (!OsProcessMgr::getUserRequestedStateString(request_state, requestedState))
                  {
                     requestedState = "<unknown>";
                  }
                  OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                                "ProcMgmtRpc::setUserRequestState"
                                " host %s state '%s' %s",
                                pCallingHostname->data(),
                                requestedState.data(),
                                (  pBlock->getValue()
                                 ? "BLOCKING"
                                 : (  (USER_PROCESS_RESTART == request_state)
                                    ? "(always blocks)" : "NON-BLOCKING"))
                                );

                  UtlHashMap  process_results;
                  UtlHashMap  original_pids;
                  int         blockSecs;
                  pWatchDog->getPidsByAliasList( *pAliasList, original_pids);

                  if ( 1 == process_results.entries() )
                  {
                     blockSecs = SINGLE_BLOCK_MAX;
                  }
                  else
                  {
                     blockSecs = LIST_BLOCK_MAX;
                  }

                  if ( USER_PROCESS_RESTART == request_state ) {
                     // Stop all processes requested followed by starting the all to simulate a restart.
                     // Need to add a check on whether or not process has restart capabilities when we
                     // have the new sipXsupervisor.
                     pWatchDog->setProcessUserRequestStateList( USER_PROCESS_STOP, *pAliasList, process_results );
                     blockForProcessStateMatchList(pWatchDog, process_results, USER_PROCESS_STOP, blockSecs);
                     process_results.destroyAll();
                     pWatchDog->setProcessUserRequestStateList( USER_PROCESS_START, *pAliasList, process_results );
                     blockForProcessStateMatchList(pWatchDog, process_results, USER_PROCESS_START, blockSecs);
                  }
                  else {
                     pWatchDog->setProcessUserRequestStateList( request_state, *pAliasList, process_results );
                     if (pBlock->getValue())
                     {
                        if (USER_PROCESS_RESTART == request_state)
                        {
                           blockForProcessRestartList(pWatchDog, process_results, original_pids, blockSecs);
                        }
                        else
                        {
                           blockForProcessStateMatchList(pWatchDog, process_results, request_state, blockSecs);
                        }
                     }
                  }

                  // Construct and set the response.
                  response.setResponse(&process_results);
                  status = XmlRpcMethod::OK;
                  result = true;

                  // Delete the new'd UtlString objects (alias names and state change results.)
                  process_results.destroyAll();

               }
            }
         }
      }
   }

   return result;
}

bool ProcMgmtRpcMethod::blockForProcessStateMatch(WatchDog* pWatchDog, const UtlString& alias, const int request_state, const int max_secs)
{
   UtlHashMap process_results;
   UtlString tmp_alias(alias);
   UtlBool tmp_result(true);
   process_results.insertKeyAndValue(&tmp_alias, &tmp_result);
   
   blockForProcessStateMatchList(pWatchDog, process_results, request_state, max_secs);
   
   return tmp_result.getValue();
}

bool ProcMgmtRpcMethod::blockForProcessRestart(WatchDog* pWatchDog, const UtlString& alias, const PID original_pid, const int max_secs)
{
   UtlHashMap process_results;
   UtlString tmp_alias(alias);
   UtlBool tmp_result(true);
   process_results.insertKeyAndValue(&tmp_alias, &tmp_result);

   UtlHashMap original_pids;
   UtlInt tmp_pid(original_pid);
   original_pids.insertKeyAndValue(&tmp_alias, &tmp_pid);

   blockForProcessRestartList(pWatchDog, process_results, original_pids, max_secs);

   return tmp_result.getValue();
}

void ProcMgmtRpcMethod::blockForProcessRestartList(WatchDog* pWatchDog, 
                                                   UtlHashMap& process_results, 
                                                   const UtlHashMap& original_pids, 
                                                   const int max_secs)
{
   // Build a bag of the aliases whose user requested state was changed to RESTART.
   UtlHashBag waiting_processes;
   UtlString* pAlias;
   UtlHashMapIterator iMap(process_results);
   while ((pAlias = dynamic_cast<UtlString*>(iMap())))
   {
      // If the user requested state of this process changed then add its alias to the bag.
      UtlBool* pResult = dynamic_cast<UtlBool*>(iMap.value());
      if (pResult && pResult->getValue())
      {
         waiting_processes.insert(new UtlString(*pAlias));
      }
   }   
   
   // Block waiting for these processes to be running with different PIDs.
   int secs = 0;
   while (secs < max_secs && !waiting_processes.isEmpty())
   {
      secs++;
      OsTask::delay(1000); 

      // Loop through all the processes that we were still waiting on and
      // remove those that have undergone the expected state change.
      UtlHashBagIterator iBag(waiting_processes);      
      while ((pAlias = dynamic_cast<UtlString*>(iBag())))
      {
         // Is the current PID non-zero and different from the original PID?
         PID current_pid = pWatchDog->getPidByAlias(*pAlias);
         if (0 != current_pid)
         {
            UtlInt* pPid = dynamic_cast<UtlInt*>(original_pids.findValue(pAlias));
            if (pPid &&  current_pid != pPid->getValue())
            {
               // Yes, so this process has restarted.
               waiting_processes.destroy(pAlias); 
            }
         }
      }
   }

   // Why did the above while loop exit?
   if (!waiting_processes.isEmpty())
   {
      // There was a timeout waiting for one or more processes to undergo the   
      // expected state change.  Update the failed entries.
      UtlHashBagIterator iBag(waiting_processes);
      while ((pAlias = dynamic_cast<UtlString*>( iBag() )))
      {
         UtlBool* pResult = dynamic_cast<UtlBool*>(process_results.findValue(pAlias));
         if (pResult)
         {
            *pResult = false;
         }
      }
      waiting_processes.destroyAll();
   }
}

void ProcMgmtRpcMethod::blockForProcessStateMatchList(WatchDog* pWatchDog, 
                                                      UtlHashMap& process_results, 
                                                      const int request_state, 
                                                      const int max_secs)
{
   // Build a bag of the aliases whose user requested state was changed.
   UtlHashBag waiting_processes;
   UtlString* pAlias;
   UtlHashMapIterator iMap(process_results);
   while ((pAlias = dynamic_cast<UtlString*>(iMap())))
   {
      // If the user requested state of this process changed then add its alias to the bag.
      UtlBool* pResult = dynamic_cast<UtlBool*>(iMap.value());
      if (pResult && pResult->getValue())
      {
         waiting_processes.insert(new UtlString(*pAlias));
      }
   }   

   // Block waiting for these processes to have their state changed.
   int secs = 0;
   while (secs < max_secs && !waiting_processes.isEmpty())
   {
      secs++;
      OsTask::delay(1000); 

      // Loop through all the processes that we were still waiting on and
      // remove those that have undergone the expected state change.
      UtlHashBagIterator iBag(waiting_processes);      
      while ((pAlias = dynamic_cast<UtlString*>(iBag())))
      {
         // Does the current state match the requested state?
         if (pWatchDog->getProcessState(*pAlias) == request_state)
         {
            // Yes, so his process has changed state.
            waiting_processes.destroy(pAlias); 
         }
      }
   }

   // Why did the above while loop exit?
   if (!waiting_processes.isEmpty())
   {
      // There was a timeout waiting for one or more processes to undergo the   
      // expected state change.  Update the failed entries.
      UtlHashBagIterator iBag(waiting_processes);
      while ((pAlias = dynamic_cast<UtlString*>( iBag() )))
      {
         UtlBool* pResult = dynamic_cast<UtlBool*>(process_results.findValue(pAlias));
         if (pResult)
         {
            *pResult = false;
         }
      }
      waiting_processes.destroyAll();
   }
}

bool ProcMgmtRpcMethod::executeSetUserRequestStateAll(const HttpRequestContext& requestContext,
                                                      UtlSList& params,
                                                      void* userData,
                                                      XmlRpcResponse& response,
                                                      ExecutionStatus& status,
                                                      const int request_state
                                                      )
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
   {
      handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
   }
   else
   {
      UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

      if (1 != params.entries())
      {
         handleExtraExecuteParam(name(), response, status);
      }
      else
      {
         WatchDog* pWatchDog = ((WatchDog *)userData);

         if(validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
         {

            UtlString requestedState;
            if (!OsProcessMgr::getUserRequestedStateString(request_state, requestedState))
            {
               requestedState = "<unknown>";
            }
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                          "ProcMgmtRpc::setUserRequestStateAll"
                          " host %s requested state '%s'",
                          pCallingHostname->data(),
                          requestedState.data()
                          );


            // Set the "user request state" of the all monitored processes to the specified
            // state.   (This dynamically allocates memory.)  For those successful, also wait
            // for the state change.
            UtlHashMap original_pids;
            pWatchDog->getAllPids(original_pids);
            UtlHashMap process_results;
            pWatchDog->setProcessUserRequestStateAll(request_state, process_results);
            if (USER_PROCESS_RESTART == request_state)
            {
               blockForProcessRestartList(pWatchDog, process_results, original_pids);
            }
            else
            {
               blockForProcessStateMatchList(pWatchDog, process_results, request_state);
            }

            // Construct and set the response.
            response.setResponse(&process_results);
            status = XmlRpcMethod::OK;
            result = true;

            // Delete the new'd UtlString objects (alias names and state change results.)
            process_results.destroyAll();
         }
      }
   }

   return result;
}

/*****************************************************************
 **** ProcMgmtRpcGetStateAll
 *****************************************************************/

const char* ProcMgmtRpcGetStateAll::METHOD_NAME = "ProcMgmtRpc.getStateAll";

const char* ProcMgmtRpcGetStateAll::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcGetStateAll::ProcMgmtRpcGetStateAll()
{
}

XmlRpcMethod* ProcMgmtRpcGetStateAll::get()
{
   return new ProcMgmtRpcGetStateAll();
}

void ProcMgmtRpcGetStateAll::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetStateAll::get, watchdog);
}

bool ProcMgmtRpcGetStateAll::execute(const HttpRequestContext& requestContext,
                                     UtlSList& params,
                                     void* userData,
                                     XmlRpcResponse& response,
                                     ExecutionStatus& status)
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
   {
      handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
   }
   else
   {
      UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

      if (1 != params.entries())
      {
         handleExtraExecuteParam(name(), response, status);
      }
      else
      {
         WatchDog* pWatchDog = ((WatchDog *)userData);

         if(validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                          "ProcMgmtRpc::getUserRequestStateAll"
                          " host %s requested process states",
                          pCallingHostname->data()
                          );

            // Get the states of the monitored processes.  (This dynamically allocates memory.)
            UtlHashMap process_states;
            pWatchDog->getProcessStateAll(process_states);

            // Construct and set the response.
            response.setResponse(&process_states);
            status = XmlRpcMethod::OK;
            result = true;

            // Delete the new'd UtlString objects (alias names and states.)
            process_states.destroyAll();
         }
      }
   }

   return result;
}

/*****************************************************************
 **** ProcMgmtRpcStart
 *****************************************************************/

const char* ProcMgmtRpcStart::METHOD_NAME = "ProcMgmtRpc.start";

const char* ProcMgmtRpcStart::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcStart::ProcMgmtRpcStart()
{
}

XmlRpcMethod* ProcMgmtRpcStart::get()
{
   return new ProcMgmtRpcStart();
}

void ProcMgmtRpcStart::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStart::get, watchdog);
}

bool ProcMgmtRpcStart::execute(const HttpRequestContext& requestContext,
                               UtlSList& params,
                               void* userData,
                               XmlRpcResponse& response,
                               ExecutionStatus& status)
{
   return executeSetUserRequestState(requestContext, params, 
             userData, response, status, USER_PROCESS_START);
}

/*****************************************************************
 **** ProcMgmtRpcStartAll
 *****************************************************************/

const char* ProcMgmtRpcStartAll::METHOD_NAME = "ProcMgmtRpc.startAll";

const char* ProcMgmtRpcStartAll::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcStartAll::ProcMgmtRpcStartAll()
{
}

XmlRpcMethod* ProcMgmtRpcStartAll::get()
{
   return new ProcMgmtRpcStartAll();
}

void ProcMgmtRpcStartAll::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStartAll::get, watchdog);
}

bool ProcMgmtRpcStartAll::execute(const HttpRequestContext& requestContext,
                                  UtlSList& params,
                                  void* userData,
                                  XmlRpcResponse& response,
                                  ExecutionStatus& status)
{
   return executeSetUserRequestStateAll(requestContext, params, 
             userData, response, status, USER_PROCESS_START);
}

/*****************************************************************
 **** ProcMgmtRpcStop
 *****************************************************************/

const char* ProcMgmtRpcStop::METHOD_NAME = "ProcMgmtRpc.stop";

const char* ProcMgmtRpcStop::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcStop::ProcMgmtRpcStop()
{
}

XmlRpcMethod* ProcMgmtRpcStop::get()
{
   return new ProcMgmtRpcStop();
}

void ProcMgmtRpcStop::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStop::get, watchdog);
}

bool ProcMgmtRpcStop::execute(const HttpRequestContext& requestContext,
                              UtlSList& params,
                              void* userData,
                              XmlRpcResponse& response,
                              ExecutionStatus& status)
{
   return executeSetUserRequestState(requestContext, params, 
             userData, response, status, USER_PROCESS_STOP);
}

/*****************************************************************
 **** ProcMgmtRpcStopAll
 *****************************************************************/

const char* ProcMgmtRpcStopAll::METHOD_NAME = "ProcMgmtRpc.stopAll";

const char* ProcMgmtRpcStopAll::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcStopAll::ProcMgmtRpcStopAll()
{
}

XmlRpcMethod* ProcMgmtRpcStopAll::get()
{
   return new ProcMgmtRpcStopAll();
}

void ProcMgmtRpcStopAll::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStopAll::get, watchdog);
}

bool ProcMgmtRpcStopAll::execute(const HttpRequestContext& requestContext,
                                  UtlSList& params,
                                  void* userData,
                                  XmlRpcResponse& response,
                                  ExecutionStatus& status)
{
   return executeSetUserRequestStateAll(requestContext, params, 
             userData, response, status, USER_PROCESS_STOP);
}

/*****************************************************************
 **** ProcMgmtRpcRestart
 *****************************************************************/

const char* ProcMgmtRpcRestart::METHOD_NAME = "ProcMgmtRpc.restart";

const char* ProcMgmtRpcRestart::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcRestart::ProcMgmtRpcRestart()
{
}

XmlRpcMethod* ProcMgmtRpcRestart::get()
{
   return new ProcMgmtRpcRestart();
}

void ProcMgmtRpcRestart::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcRestart::get, watchdog);
}

bool ProcMgmtRpcRestart::execute(const HttpRequestContext& requestContext,
                                 UtlSList& params,
                                 void* userData,
                                 XmlRpcResponse& response,
                                 ExecutionStatus& status)
{
   return executeSetUserRequestState(requestContext, params, 
             userData, response, status, USER_PROCESS_RESTART);
}

/*****************************************************************
 **** ProcMgmtRpcRestartAll
 *****************************************************************/

const char* ProcMgmtRpcRestartAll::METHOD_NAME = "ProcMgmtRpc.restartAll";

const char* ProcMgmtRpcRestartAll::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcRestartAll::ProcMgmtRpcRestartAll()
{
}

XmlRpcMethod* ProcMgmtRpcRestartAll::get()
{
   return new ProcMgmtRpcRestartAll();
}

void ProcMgmtRpcRestartAll::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcRestartAll::get, watchdog);
}

bool ProcMgmtRpcRestartAll::execute(const HttpRequestContext& requestContext,
                                    UtlSList& params,
                                    void* userData,
                                    XmlRpcResponse& response,
                                    ExecutionStatus& status)
{
   return executeSetUserRequestStateAll(requestContext, params, 
             userData, response, status, USER_PROCESS_RESTART);
}

/*****************************************************************
 **** ProcMgmtRpcGetAliasByPID
 *****************************************************************/

const char* ProcMgmtRpcGetAliasByPID::METHOD_NAME = "ProcMgmtRpc.getAliasByPID";

const char* ProcMgmtRpcGetAliasByPID::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcGetAliasByPID::ProcMgmtRpcGetAliasByPID()
{
}

XmlRpcMethod* ProcMgmtRpcGetAliasByPID::get()
{
   return new ProcMgmtRpcGetAliasByPID();
}

void ProcMgmtRpcGetAliasByPID::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetAliasByPID::get, watchdog);
}

bool ProcMgmtRpcGetAliasByPID::execute(const HttpRequestContext& requestContext,
                                       UtlSList& params,
                                       void* userData,
                                       XmlRpcResponse& response,
                                       ExecutionStatus& status)
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
   {
      handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
   }
   else
   {
      UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

      if (!params.at(1) || !params.at(1)->isInstanceOf(UtlInt::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_PID, response, status);
      }
      else
      {
         UtlInt* pPID = dynamic_cast<UtlInt*>(params.at(1));

         if (2 != params.entries())
         {
            handleExtraExecuteParam(name(), response, status);
         }
         else
         {
            WatchDog* pWatchDog = ((WatchDog *)userData);

            if (validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
            {
               // Attempt to find the alias, which will be blank is there is a failure.
               UtlString alias = pWatchDog->getAliasByPid(pPID->getValue());

               OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                             "ProcMgmtRpc::getUserRequestState"
                             " host %s requested process state for %" PRIdPTR " (%s)",
                             pCallingHostname->data(),
                             pPID->getValue(), alias.data()
                             );


               // Construct and set the response.
               response.setResponse(&alias);
               status = XmlRpcMethod::OK;
               result = true;
            }
         }
      }
   }

   return result;
}

/*****************************************************************
 **** ProcMgmtRpcGetConfigVersion
 *****************************************************************/

const char* ProcMgmtRpcGetConfigVersion::METHOD_NAME = "ProcMgmtRpc.getConfigVersion";

const char* ProcMgmtRpcGetConfigVersion::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcGetConfigVersion::ProcMgmtRpcGetConfigVersion()
{
}

XmlRpcMethod* ProcMgmtRpcGetConfigVersion::get()
{
   return new ProcMgmtRpcGetConfigVersion();
}

void ProcMgmtRpcGetConfigVersion::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetConfigVersion::get, watchdog);
}

bool ProcMgmtRpcGetConfigVersion::execute(const HttpRequestContext& requestContext,
                                       UtlSList& params,
                                       void* userData,
                                       XmlRpcResponse& response,
                                       ExecutionStatus& status)
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (2 != params.entries())
   {
      handleExtraExecuteParam(name(), response, status);
   }
   else
   {
      if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
      }
      else
      {
         UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_SERVICE, response, status);
         }
         else
         {
            UtlString* pserviceName = dynamic_cast<UtlString*>(params.at(1));
            WatchDog* pWatchDog = ((WatchDog *)userData);
            SipxProcessManager* pProcessManager = SipxProcessManager::getInstance();

            if(validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
            {
                OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                              "ProcMgmtRpc::getConfigVersion"
                              " host %s requested service configuration version",
                              pCallingHostname->data()
                              );

                SipxProcess* pProcess = pProcessManager->findProcess( *pserviceName );
                if ( pProcess )
                {
                   // Query the current version for the service as obtained from the service configuration file.
                   UtlString service_version = pProcess->getConfigurationVersion();

                   // Construct and set the response.
                   response.setResponse(&service_version);
                   status = XmlRpcMethod::OK;
                   result = true;
                }
                else
                {
                   // Invalid service name.
                   handleMissingExecuteParam(name(), PARAM_NAME_SERVICE, response, status);
                }
            }
         }
      }
   }  // param.entries

   return result;

}

/*****************************************************************
 **** ProcMgmtRpcSetConfigVersion
 *****************************************************************/

const char* ProcMgmtRpcSetConfigVersion::METHOD_NAME = "ProcMgmtRpc.setConfigVersion";

const char* ProcMgmtRpcSetConfigVersion::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcSetConfigVersion::ProcMgmtRpcSetConfigVersion()
{
}

XmlRpcMethod* ProcMgmtRpcSetConfigVersion::get()
{
   return new ProcMgmtRpcSetConfigVersion();
}

void ProcMgmtRpcSetConfigVersion::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcSetConfigVersion::get, watchdog);
}

bool ProcMgmtRpcSetConfigVersion::execute(const HttpRequestContext& requestContext,
                                       UtlSList& params,
                                       void* userData,
                                       XmlRpcResponse& response,
                                       ExecutionStatus& status)
{
   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (3 != params.entries())
   {
      handleExtraExecuteParam(name(), response, status);
   }
   else
   {
      UtlBool service_version_setting(false);
      if (!params.at(0) || !params.at(0)->isInstanceOf(UtlString::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_CALLING_HOST, response, status);
      }
      else
      {
         UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));

         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_SERVICE, response, status);
         }
         else
         {
            UtlString* pserviceName = dynamic_cast<UtlString*>(params.at(1));

            if (!params.at(2) || !params.at(2)->isInstanceOf(UtlString::TYPE))
            {
               handleMissingExecuteParam(name(), PARAM_NAME_SERVICE_VERSION, response, status);
            }
            else
            {
               UtlString* pserviceVersion = dynamic_cast<UtlString*>(params.at(2));

               WatchDog* pWatchDog = ((WatchDog *)userData);
               SipxProcessManager* pProcessManager = SipxProcessManager::getInstance();

               if(validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
               {
                   OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                                 "ProcMgmtRpc::setConfigVersion"
                                 " host %s requested setting service configuration version for %s",
                                 pCallingHostname->data(), pserviceName->data()
                                 );

                   SipxProcess* pProcess = pProcessManager->findProcess( *pserviceName );
                   if ( pProcess )
                   {
                      // Write the version number to the appropriate process.
                      pProcess->setConfigurationVersion( *pserviceVersion ); 

                      UtlBool method_result(true);
                      response.setResponse(&method_result);
                      status = XmlRpcMethod::OK;
                      result = true;
                   }
                   else
                   {
                      // Invalid service name.
                      handleMissingExecuteParam(name(), PARAM_NAME_SERVICE, response, status);
                   }
               }
            }
         }
      }
   }  // param.entries

   return result;
}
