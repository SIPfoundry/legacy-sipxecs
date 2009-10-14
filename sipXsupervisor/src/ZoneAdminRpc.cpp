//
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
//
#include <unistd.h>
#include <sys/stat.h>
#include "utl/UtlBool.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlTokenizer.h"
#include "os/OsSysLog.h"
#include "os/OsFileSystem.h"
#include "os/OsProcess.h"
#include "os/OsProcessIterator.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "net/NetBase64Codec.h"

#include "sipXecsService/SipXecsService.h"
#include "SipxRpc.h"
#include "ZoneAdminRpc.h"
#include "SipxProcessManager.h"
#include "FileResource.h"
#include "utl/XmlContent.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//
//
const char* ZoneAdminExec = "sipx-zoneadmin.sh";
const char* ZoneAdminGenerate_cmd = "generateDns";
const char* ZoneAdminStdOut_filetype = ".output.log";
const char* ZoneAdminStdErr_filetype = ".err.log";

// STATIC VARIABLE INITIALIZATIONS

/*************************************************************************
 **** ZoneAdminRpcMethod contains common code for ZoneAdminRpc methods
 ************************************************************************/
const char* ZoneAdminRpcMethod::METHOD_NAME = "ZoneAdminRpc.BASE";
const char* ZoneAdminRpcMethod::PARAM_NAME_CALLING_HOST = "callingHostname";
const char* ZoneAdminRpcMethod::PARAM_NAME_COMMAND = "zoneCommand";


XmlRpcMethod* ZoneAdminRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void ZoneAdminRpcMethod::registerSelf(SipxRpc & sipxRpcImpl)
{
   assert(false);  // this should have been overridden in the subclass
}

ZoneAdminRpcMethod::ZoneAdminRpcMethod()
{
}

void ZoneAdminRpcMethod::registerMethod(const char*       methodName,
                                   XmlRpcMethod::Get getMethod,
                                   SipxRpc &         sipxRpcImpl
                                   )
{
   sipxRpcImpl.getXmlRpcDispatch()->addMethod(methodName, getMethod, &sipxRpcImpl );
}

bool ZoneAdminRpcMethod::execute(const HttpRequestContext& requestContext,
                            UtlSList&                 params,
                            void*                     userData,
                            XmlRpcResponse&           response,
                            ExecutionStatus&          status
                            )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool ZoneAdminRpcMethod::validCaller(const HttpRequestContext& requestContext,
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
         OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                       "ZoneAdminRpcMethod::validCaller '%s' peer authenticated for %s",
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
         response.setFault(ZoneAdminRpcMethod::UnconfiguredPeer, faultMsg.data());

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

void ZoneAdminRpcMethod::handleMissingExecuteParam(const char* methodName,
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
   response.setFault(ZoneAdminRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

void ZoneAdminRpcMethod::handleExtraExecuteParam(const char* methodName,
                                            XmlRpcResponse& response,
                                            ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " has incorrect number of parameters";
   status = XmlRpcMethod::FAILED;
   response.setFault(ZoneAdminRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

bool ZoneAdminRpcMethod::duplicateProcess(const char*     command,
                                        XmlRpcResponse& response,
                                        ExecutionStatus& status
                                       )
{
   bool result = false;

   if (isProcessActive(command))
   {
      // process already found running.  set found to true.
      UtlString faultMsg;

      faultMsg.append("Duplicate process '");
      faultMsg.append(command);
      faultMsg.append("' found");
      response.setFault(ZoneAdminRpcMethod::DuplicateInstance, faultMsg.data());
      status = XmlRpcMethod::FAILED;
      result = true;
   }

   return result;
}

bool ZoneAdminRpcMethod::isProcessActive(const char* command)
{
   bool result = false;

   OsProcessIterator ProcessIterator;
   OsProcess runningProcess;
   OsProcessInfo procInfo;
   OsStatus procStatus;

   OsStatus iter_status = ProcessIterator.findFirst( runningProcess );
   while ( (iter_status == OS_SUCCESS) && (!result) )
   {
      procStatus = runningProcess.getInfo(procInfo);
      if (procStatus == OS_SUCCESS)
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "Process commandlines found = %s", procInfo.commandline.data());
         procInfo.name.data(), procInfo.commandline.data();
         if (procInfo.commandline.contains(command))
         {
            result = true;
         }
      }
      iter_status = ProcessIterator.findNext(runningProcess);
   }

   return result;
}

bool ZoneAdminRpcMethod::buildOutputFiles(const UtlString&     command,
                         UtlString&      stdoutfn,
                         UtlString&      stderrfn
                        )
{
   bool result = true;
   stdoutfn.remove(0);
   stderrfn.remove(0);
   stdoutfn.append(ZoneAdminGenerate_cmd);
   stderrfn.append(ZoneAdminGenerate_cmd);
   stderrfn.append(ZoneAdminStdErr_filetype);
   stdoutfn.append(ZoneAdminStdOut_filetype);

   return result;
}

/*****************************************************************
 **** ZoneAdminRpcExec
 *****************************************************************/

const char* ZoneAdminRpcExec::METHOD_NAME = "ZoneAdminRpc.generateDns";

const char* ZoneAdminRpcExec::name()
{
   return METHOD_NAME;
}

ZoneAdminRpcExec::ZoneAdminRpcExec()
{
}

XmlRpcMethod* ZoneAdminRpcExec::get()
{
   return new ZoneAdminRpcExec();
}

void ZoneAdminRpcExec::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, ZoneAdminRpcExec::get, sipxRpcImpl);
}

bool ZoneAdminRpcExec::execute(const HttpRequestContext& requestContext,
                               UtlSList&                 params,
                               void*                     userData,
                               XmlRpcResponse&           response,
                               ExecutionStatus&          status)
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
         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_COMMAND, response, status);
         }
         else
         {
            UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));
            SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

            if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlBool   method_result(true);
               UtlString arguments[3];
               OsPath    mWorkingDirectory = ".";
               OsPath    mExec = SipXecsService::Path(SipXecsService::LibExecDirType,"sipxzoneadmin");
               UtlString mStdOutFile;
               UtlString mStdErrFile;

               UtlString* pSubCommand = dynamic_cast<UtlString*>(params.at(1));

               if ( !buildOutputFiles(*pSubCommand, mStdOutFile, mStdErrFile))
               {
                    // Invalid request. Set a Fault.
                    response.setFault(ZoneAdminRpcMethod::FailureToLaunch, "Invalid command");
                    status = XmlRpcMethod::FAILED;
                    return result;
               }

               OsPath    mStdOutPath = SipXecsService::Path(SipXecsService::LogDirType, mStdOutFile.data());
               OsPath    mStdErrPath = SipXecsService::Path(SipXecsService::LogDirType, mStdErrFile.data());

               // Pass the argumenst to sipx-zoneadmin.sh
               arguments[0] = "-n";                 // non-interactive
               arguments[1] = pSubCommand->data();  // string "<primary server> -o <secondary server>"

               // Make sure that there is no other instance running.
               if (! duplicateProcess(ZoneAdminExec, response, status))
               {
                  // execute the command and return whether or not the launch was successful.
                  OsProcess* zoneCheck = new OsProcess();

                  // Setup the Standard Output and Standard Error files.
                  OsPath mStdInFile;   // Blank
                  int rc;
                  rc = zoneCheck->setIORedirect(mStdInFile, mStdOutPath, mStdErrPath);

                  // Launch the process but tell the parent to ignore the child's signals (especially on shutdown).
                  // It will let the system handle it to avoid a defunct process.
                  if ( (rc=zoneCheck->launch(mExec, &arguments[0], mWorkingDirectory,
                                   zoneCheck->NormalPriorityClass, FALSE,
                                   TRUE)) // Parent to ignore child signals.
                           == OS_SUCCESS )
                  {
                      // Construct and set the response.
                      UtlSList outputPaths;
                      outputPaths.insert(&mStdOutPath);
                      outputPaths.insert(&mStdErrPath);

                      // Add the file resources to Supervisor Process so they can be retrieved
                      FileResource::logFileResource( mStdOutPath, SipxProcessManager::getInstance()->findProcess(SUPERVISOR_PROCESS_NAME));
                      FileResource::logFileResource( mStdErrPath, SipxProcessManager::getInstance()->findProcess(SUPERVISOR_PROCESS_NAME));
                      response.setResponse(&outputPaths);
                      status = XmlRpcMethod::OK;
                      result = true;
                  }   // launch
                  else
                  {
                     // Failed to launch the command, send a fault.
                     response.setFault(ZoneAdminRpcMethod::FailureToLaunch, "Failure to launch command");
                     status = XmlRpcMethod::FAILED;
                  }

                  delete zoneCheck;
               }  // duplicateProcess
            }  // validcaller
            else
            {
               status = XmlRpcMethod::FAILED;
            }
         }  // param 1 okay
      }  // param 0 okay
   } //number of parms check

   return result;
}

