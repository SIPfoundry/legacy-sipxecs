//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "net/XmlRpcDispatch.h"
#include "SipxRpc.h"

#include "AlarmServer.h"
#include "AlarmRpc.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/*************************************************************************
 **** AlarmRpcMethod contains common code for AlarmRpc methods
 ************************************************************************/
const char* AlarmRpcMethod::METHOD_NAME = "AlarmRpc.BASE";
const char* AlarmRpcMethod::PARAM_NAME_CALLING_HOST = "callingHostname";
const char* AlarmRpcMethod::PARAM_NAME_ALARM_ID = "AlarmId";
const char* AlarmRpcMethod::PARAM_NAME_ALARM_PARAMS = "AlarmParams";


XmlRpcMethod* AlarmRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void AlarmRpcMethod::registerSelf(SipxRpc& sipxRpcImpl)
{
   assert(false);  // this should have been overridden in the subclass
}

AlarmRpcMethod::AlarmRpcMethod()
{
}

void AlarmRpcMethod::registerMethod(const char*       methodName,
                                    XmlRpcMethod::Get getMethod,
                                    SipxRpc&          sipxRpcImpl
                                   )
{
   sipxRpcImpl.getXmlRpcDispatch()->addMethod(methodName, getMethod, &sipxRpcImpl );
}

bool AlarmRpcMethod::execute(const HttpRequestContext& requestContext,
                             UtlSList& params,
                             void* userData,
                             XmlRpcResponse& response,
                             ExecutionStatus& status
                             )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool AlarmRpcMethod::validCaller(const HttpRequestContext& requestContext,
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
         // sipXsupervisor says it is one of the allowed peers.
         result = true;
         OsSysLog::add(FAC_ALARM, PRI_DEBUG,
                       "AlarmRpcMethod::validCaller '%s' peer authenticated for %s",
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
         response.setFault(AlarmRpcMethod::UnconfiguredPeer, faultMsg.data());

         OsSysLog::add(FAC_ALARM, PRI_ERR,
                       "%s failed - '%s' not a configured peer",
                       callingMethod, peerName.data()
                       );
      }
   }
   else
   {
      // ssl says not authenticated - provide only a generic error
      response.setFault(XmlRpcResponse::AuthenticationRequired, "TLS Peer Authentication Failure");

      OsSysLog::add(FAC_ALARM, PRI_ERR,
                    "%s failed: '%s' failed SSL authentication",
                    callingMethod, peerName.data()
                    );
   }

   return result;
}

void AlarmRpcMethod::handleMissingExecuteParam(const char* methodName,
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
   response.setFault(AlarmRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_ALARM, PRI_ERR, faultMsg);
}

void AlarmRpcMethod::handleExtraExecuteParam(const char* methodName,
                                             XmlRpcResponse& response,
                                             ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " has incorrect number of parameters";
   status = XmlRpcMethod::FAILED;
   response.setFault(AlarmRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_ALARM, PRI_ERR, faultMsg);
}


/*****************************************************************
 **** AlarmRpcRaiseAlarm
 *****************************************************************/

const char* AlarmRpcRaiseAlarm::METHOD_NAME = "Alarm.raiseAlarm";

const char* AlarmRpcRaiseAlarm::name()
{
   return METHOD_NAME;
}

AlarmRpcRaiseAlarm::AlarmRpcRaiseAlarm()
{
}

XmlRpcMethod* AlarmRpcRaiseAlarm::get()
{
   return new AlarmRpcRaiseAlarm();
}

void AlarmRpcRaiseAlarm::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, AlarmRpcRaiseAlarm::get, sipxRpcImpl);
}

bool AlarmRpcRaiseAlarm::execute(const HttpRequestContext& requestContext,
                                 UtlSList& params,
                                 void* userData,
                                 XmlRpcResponse& response,
                                 ExecutionStatus& status)
{

   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (params.entries() > 3)
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
            handleMissingExecuteParam(name(), PARAM_NAME_ALARM_ID, response, status);
         }
         else
         {
            UtlString* pAlarmId = dynamic_cast<UtlString*>(params.at(1));

            if (!params.at(2) || !params.at(2)->isInstanceOf(UtlSList::TYPE))
            {
               handleMissingExecuteParam(name(), PARAM_NAME_ALARM_PARAMS, response, status);
            }
            else
            {
               UtlSList* pAlarmParams = dynamic_cast<UtlSList*>(params.at(2));

               UtlBool method_result(false);
               SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

               if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
               {
                  OsSysLog::add(FAC_ALARM, PRI_DEBUG,
                        "AlarmRpc::raiseAlarm: host %s requested alarm '%s'",
                        pCallingHostname->data(), pAlarmId->data() );

                  method_result = cAlarmServer::getInstance()->handleAlarm(*pCallingHostname,
                        *pAlarmId, *pAlarmParams);

                  // Construct and set the response.
                  response.setResponse(&method_result);
                  status = XmlRpcMethod::OK;
                  result = true;
               }
            }
         }
      }
   }

   return result;
}

/*****************************************************************
 **** AlarmRpcGetAlarmCount
 *****************************************************************/

const char* AlarmRpcGetAlarmCount::METHOD_NAME = "Alarm.getAlarmCount";

const char* AlarmRpcGetAlarmCount::name()
{
   return METHOD_NAME;
}

AlarmRpcGetAlarmCount::AlarmRpcGetAlarmCount()
{
}

XmlRpcMethod* AlarmRpcGetAlarmCount::get()
{
   return new AlarmRpcGetAlarmCount();
}

void AlarmRpcGetAlarmCount::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, AlarmRpcGetAlarmCount::get, sipxRpcImpl);
}

bool AlarmRpcGetAlarmCount::execute(const HttpRequestContext& requestContext,
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
            OsSysLog::add(FAC_ALARM, PRI_INFO,
                          "AlarmRpc::getAlarmCount: host %s requested alarm count",
                          pCallingHostname->data()
                          );

            // Get the count of alarms since last restart
            UtlInt alarm_count;
            alarm_count = UtlInt(cAlarmServer::getInstance()->getAlarmCount());

            // Construct and set the response.
            response.setResponse(&alarm_count);
            status = XmlRpcMethod::OK;
            result = true;

         }
      }
   }

   return result;
}

/*****************************************************************
 **** AlarmRpcReloadAlarms
 *****************************************************************/

const char* AlarmRpcReloadAlarms::METHOD_NAME = "Alarm.reloadAlarms";

const char* AlarmRpcReloadAlarms::name()
{
   return METHOD_NAME;
}

AlarmRpcReloadAlarms::AlarmRpcReloadAlarms()
{
}

XmlRpcMethod* AlarmRpcReloadAlarms::get()
{
   return new AlarmRpcReloadAlarms();
}

void AlarmRpcReloadAlarms::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, AlarmRpcReloadAlarms::get, sipxRpcImpl);
}

bool AlarmRpcReloadAlarms::execute(const HttpRequestContext& requestContext,
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
            OsSysLog::add(FAC_ALARM, PRI_INFO,
                          "AlarmRpc::reloadAlarms:  host %s requested reload alarms",
                          pCallingHostname->data()
                          );

            UtlBool method_result(cAlarmServer::getInstance()->reloadAlarms());

            // Construct and set the response.
            response.setResponse(&method_result);
            status = XmlRpcMethod::OK;
            result = true;

         }
      }
   }

   return result;
}
