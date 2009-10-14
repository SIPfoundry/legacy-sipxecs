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
#include "utl/UtlSListIterator.h"
#include "os/OsSysLog.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "SipxProcessManager.h"
#include "SipxProcess.h"
#include "SipxRpc.h"
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
const char* ProcMgmtRpcMethod::PARAM_NAME_SERVICE = "service";
const char* ProcMgmtRpcMethod::PARAM_NAME_SERVICE_VERSION = "serviceVersion";
int ProcMgmtRpcMethod::SINGLE_BLOCK_MAX = 15;
int ProcMgmtRpcMethod::LIST_BLOCK_MAX = 45;


XmlRpcMethod* ProcMgmtRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void ProcMgmtRpcMethod::registerSelf(SipxRpc & sipxRpcImpl)
{
   assert(false);  // this should have been overridden in the subclass
}

ProcMgmtRpcMethod::ProcMgmtRpcMethod()
{
}

void ProcMgmtRpcMethod::registerMethod(const char*       methodName,
                                       XmlRpcMethod::Get getMethod,
                                       SipxRpc &         sipxRpcImpl
                                       )
{
   sipxRpcImpl.getXmlRpcDispatch()->addMethod(methodName, getMethod, &sipxRpcImpl );
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
                                    const SipxRpc&            sipxRpcImpl,
                                    const char*               callingMethod
                                    )
{
   bool result = false;

   if (!peerName.isNull() && requestContext.isTrustedPeer(peerName))
   {
      // ssl says the connection is from the named host
      if (sipxRpcImpl.isAllowedPeer(peerName))
      {
         // SipxRpc says it is one of the allowed peers.
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
                                                   RequestedProcessState request_state
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
              // we are currently ignoring the block parameter.  Does anyone care?
//            UtlBool* pBlock = dynamic_cast<UtlBool*>(params.at(2));

            if (3 != params.entries())
            {
               handleExtraExecuteParam(name(), response, status);
            }
            else
            {
               SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
               if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
               {
                  // for each alias in the list, look up its process and send start/stop event
                  SipxProcessManager* processMgr = SipxProcessManager::getInstance();
                  UtlSListIterator aliasListIterator( *pAliasList );
                  UtlString* pAlias;
                  UtlString* tmp_alias;

                  UtlHashMap  process_results;

                  while ( (pAlias = dynamic_cast<UtlString*> (aliasListIterator())) )
                  {
                     bool result = false;
                     SipxProcess* process = processMgr->findProcess(*pAlias);
                     if ( process )
                     {
                        switch (request_state)
                        {
                        case USER_PROCESS_START:
                           result = process->enable();
                           break;

                        case USER_PROCESS_STOP:
                           result = process->disable();
                           break;

                        case USER_PROCESS_RESTART:
                           result = process->restart();
                           break;

                        default:
                           OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                                         "ProcMgmtRpcMethod::executeSetUserRequestState"
                                         "invalid request_state %u", request_state);
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                      "could not find process %s", pAlias->data());
                     }
                     tmp_alias = new UtlString(*pAlias);
                     process_results.insertKeyAndValue(tmp_alias, new UtlBool(result));
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

void ProcMgmtRpcGetStateAll::registerSelf(SipxRpc& sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetStateAll::get, sipxRpcImpl);
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
         SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

         if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                          "ProcMgmtRpc::getUserRequestStateAll"
                          " host %s requested process states",
                          pCallingHostname->data()
                          );

            // Get the states of the monitored processes.  (This dynamically allocates memory.)
            UtlHashMap process_states;
            SipxProcessManager::getInstance()->getProcessStateAll(process_states);

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

void ProcMgmtRpcStart::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStart::get, sipxRpcImpl);
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

void ProcMgmtRpcStop::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcStop::get, sipxRpcImpl);
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

void ProcMgmtRpcRestart::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcRestart::get, sipxRpcImpl);
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
 **** ProcMgmtRpcGetStatusMessage
 *****************************************************************/

const char* ProcMgmtRpcGetStatusMessage::METHOD_NAME = "ProcMgmtRpc.getStatusMessages";

const char* ProcMgmtRpcGetStatusMessage::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcGetStatusMessage::ProcMgmtRpcGetStatusMessage()
{
}

XmlRpcMethod* ProcMgmtRpcGetStatusMessage::get()
{
   return new ProcMgmtRpcGetStatusMessage();
}

void ProcMgmtRpcGetStatusMessage::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetStatusMessage::get, sipxRpcImpl);
}

bool ProcMgmtRpcGetStatusMessage::execute(const HttpRequestContext& requestContext,
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

      if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_ALIAS, response, status);
      }
      else
      {
         UtlString* pAlias = dynamic_cast<UtlString*>(params.at(1));

         if (2 != params.entries())
         {
            handleExtraExecuteParam(name(), response, status);
         }
         else
         {
            SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
            if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlSList process_results;
               UtlString statusMsg;

               SipxProcessManager* processMgr = SipxProcessManager::getInstance();
               SipxProcess* process = processMgr->findProcess(*pAlias);
               if (process)
               {
                  process->getStatusMessages(process_results);
                  // Construct and set the response.
                  response.setResponse(&process_results);
                  status = XmlRpcMethod::OK;
                  // Delete the new'd UtlString objects (status messages)
                  process_results.destroyAll();
                  result = true;
               }
               else
               {
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "could not find process %s",
                        pAlias->data());
                  handleMissingExecuteParam(name(), PARAM_NAME_ALIAS, response, status);
               }
            }
         }
      }
   }
   return result;

}

/*****************************************************************
 **** ProcMgmtRpcRunConfigtest
 *****************************************************************/

const char* ProcMgmtRpcRunConfigtest::METHOD_NAME = "ProcMgmtRpc.runConfigtest";

const char* ProcMgmtRpcRunConfigtest::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcRunConfigtest::ProcMgmtRpcRunConfigtest()
{
}

XmlRpcMethod* ProcMgmtRpcRunConfigtest::get()
{
   return new ProcMgmtRpcRunConfigtest();
}

void ProcMgmtRpcRunConfigtest::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcRunConfigtest::get, sipxRpcImpl);
}

bool ProcMgmtRpcRunConfigtest::execute(const HttpRequestContext& requestContext,
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

      if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_ALIAS, response, status);
      }
      else
      {
         UtlString* pAlias = dynamic_cast<UtlString*>(params.at(1));

         if (2 != params.entries())
         {
            handleExtraExecuteParam(name(), response, status);
         }
         else
         {
            SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
            if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlBool method_result(false);
               UtlString statusMsg;

               SipxProcessManager* processMgr = SipxProcessManager::getInstance();
               SipxProcess* process = processMgr->findProcess(*pAlias);
               if (process)
               {
                  method_result = process->runConfigtest();
                  // Construct and set the response.
                  response.setResponse(&method_result);
                  status = XmlRpcMethod::OK;
                  result = true;
               }
               else
               {
                  UtlString msg("could not find process '");
                  msg.append(*pAlias);
                  msg.append("'");
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "%s", msg.data());
                  status = XmlRpcMethod::FAILED;
                  response.setFault(ProcMgmtRpcMethod::InvalidParameter, msg);
               }
            }
         }
      }
   }
   return result;

}

/*****************************************************************
 **** ProcMgmtRpcGetConfigtestMessages
 *****************************************************************/

const char* ProcMgmtRpcGetConfigtestMessages::METHOD_NAME = "ProcMgmtRpc.getConfigtestMessages";

const char* ProcMgmtRpcGetConfigtestMessages::name()
{
   return METHOD_NAME;
}

ProcMgmtRpcGetConfigtestMessages::ProcMgmtRpcGetConfigtestMessages()
{
}

XmlRpcMethod* ProcMgmtRpcGetConfigtestMessages::get()
{
   return new ProcMgmtRpcGetConfigtestMessages();
}

void ProcMgmtRpcGetConfigtestMessages::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetConfigtestMessages::get, sipxRpcImpl);
}

bool ProcMgmtRpcGetConfigtestMessages::execute(const HttpRequestContext& requestContext,
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

      if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
      {
         handleMissingExecuteParam(name(), PARAM_NAME_ALIAS, response, status);
      }
      else
      {
         UtlString* pAlias = dynamic_cast<UtlString*>(params.at(1));

         if (2 != params.entries())
         {
            handleExtraExecuteParam(name(), response, status);
         }
         else
         {
            SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
            if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlSList process_results;
               UtlString statusMsg;

               SipxProcessManager* processMgr = SipxProcessManager::getInstance();
               SipxProcess* process = processMgr->findProcess(*pAlias);
               if (process)
               {
                  process->getConfigtestMessages(process_results);
                  // Construct and set the response.
                  response.setResponse(&process_results);
                  status = XmlRpcMethod::OK;
                  // Delete the new'd UtlString objects (status messages)
                  process_results.destroyAll();
                  result = true;
               }
               else
               {
                  UtlString msg("could not find process '");
                  msg.append(*pAlias);
                  msg.append("'");
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "%s", msg.data());
                  response.setFault(ProcMgmtRpcMethod::InvalidParameter, msg);
                  status = XmlRpcMethod::FAILED;
               }
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

void ProcMgmtRpcGetConfigVersion::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcGetConfigVersion::get, sipxRpcImpl);
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
            SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
            SipxProcessManager* pProcessManager = SipxProcessManager::getInstance();

            if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
                OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                              "ProcMgmtRpc::getConfigVersion"
                              " host %s requested service configuration version",
                              pCallingHostname->data()
                              );

                SipxProcess* pProcess = pProcessManager->findProcess( *pserviceName );
                if ( pProcess )
                {
                   /* Query the current version for the service as obtained
                    * from the service configuration file. */
                   UtlString service_version;
                   pProcess->getConfigurationVersion(service_version);

                   // Construct and set the response.
                   response.setResponse(&service_version);
                   status = XmlRpcMethod::OK;
                   result = true;
                }
                else
                {
                   // Invalid service name.
                   UtlString msg("could not find process '");
                   msg.append(*pserviceName);
                   msg.append("'");
                   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "%s", msg.data());
                   response.setFault(ProcMgmtRpcMethod::InvalidParameter, msg);
                   status = XmlRpcMethod::FAILED;
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

void ProcMgmtRpcSetConfigVersion::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ProcMgmtRpcSetConfigVersion::get, sipxRpcImpl);
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

               SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);
               SipxProcessManager* pProcessManager = SipxProcessManager::getInstance();

               if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
               {
                   OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                                 "ProcMgmtRpc::setConfigVersion"
                                 " host %s setting service '%s' configuration version to '%s'",
                                 pCallingHostname->data(), pserviceName->data(),
                                 pserviceVersion->data()
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
                      UtlString msg("could not find process '");
                      msg.append(*pserviceName);
                      msg.append("'");
                      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "%s", msg.data());
                      response.setFault(ProcMgmtRpcMethod::InvalidParameter, msg);
                      status = XmlRpcMethod::FAILED;
                   }
               }
            }
         }
      }
   }  // param.entries

   return result;
}
