//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
#include "SwAdminRpc.h"
#include "SipxProcessManager.h"
#include "FileResource.h"
#include "utl/XmlContent.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//
//
const char* SwAdminExec = "sipx-swadmin.py";
const char* SwAdminSnapshot = "sipx-snapshot";
const char* SwAdminCheckUpdate_cmd = "check-update";
const char* SwAdminUpdate_cmd = "update";
const char* SwAdminVersion_cmd = "version";
const char* SwAdminRestart_cmd = "restart";
const char* SwAdminReboot_cmd = "reboot";
const char* SwAdminSnapshot_cmd = "snapshot";
const char* SwAdminStdOut_filetype = ".output.log";
const char* SwAdminStdErr_filetype = ".err.log";

// STATIC VARIABLE INITIALIZATIONS

/*************************************************************************
 **** SwAdminRpcMethod contains common code for SwAdminRpc methods
 ************************************************************************/
const char* SwAdminRpcMethod::METHOD_NAME = "SwAdminRpc.BASE";
const char* SwAdminRpcMethod::PARAM_NAME_CALLING_HOST = "callingHostname";
const char* SwAdminRpcMethod::PARAM_NAME_COMMAND = "swCommand";


XmlRpcMethod* SwAdminRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void SwAdminRpcMethod::registerSelf(SipxRpc & sipxRpcImpl)
{
   assert(false);  // this should have been overridden in the subclass
}

SwAdminRpcMethod::SwAdminRpcMethod()
{
}

void SwAdminRpcMethod::registerMethod(const char*       methodName,
                                   XmlRpcMethod::Get getMethod,
                                   SipxRpc &         sipxRpcImpl
                                   )
{
   sipxRpcImpl.getXmlRpcDispatch()->addMethod(methodName, getMethod, &sipxRpcImpl );
}

bool SwAdminRpcMethod::execute(const HttpRequestContext& requestContext,
                            UtlSList&                 params,
                            void*                     userData,
                            XmlRpcResponse&           response,
                            ExecutionStatus&          status
                            )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool SwAdminRpcMethod::validCaller(const HttpRequestContext& requestContext,
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
                       "SwAdminRpcMethod::validCaller '%s' peer authenticated for %s",
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
         response.setFault(SwAdminRpcMethod::UnconfiguredPeer, faultMsg.data());

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

void SwAdminRpcMethod::handleMissingExecuteParam(const char* methodName,
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
   response.setFault(SwAdminRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

void SwAdminRpcMethod::handleExtraExecuteParam(const char* methodName,
                                            XmlRpcResponse& response,
                                            ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " has incorrect number of parameters";
   status = XmlRpcMethod::FAILED;
   response.setFault(SwAdminRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
}

bool SwAdminRpcMethod::duplicateProcess(const char*     command,
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
      response.setFault(SwAdminRpcMethod::DuplicateInstance, faultMsg.data());
      status = XmlRpcMethod::FAILED;
      result = true;
   }

   return result;
}

bool SwAdminRpcMethod::isProcessActive(const char* command)
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

bool SwAdminRpcMethod::buildOutputFiles(const UtlString&     command,
                         UtlString&      stdoutfn,
                         UtlString&      stderrfn
                        )
{
   bool result = true;
   stdoutfn.remove(0);
   stderrfn.remove(0);
   if ( command.compareTo(SwAdminVersion_cmd, UtlString::ignoreCase) == 0)
   {
       stdoutfn.append(SwAdminVersion_cmd);
       stderrfn.append(SwAdminVersion_cmd);
   }
   else
      if ( command.compareTo(SwAdminCheckUpdate_cmd, UtlString::ignoreCase) == 0)
      {
         stdoutfn.append(SwAdminCheckUpdate_cmd);
         stderrfn.append(SwAdminCheckUpdate_cmd);
      }
      else
         if ( command.compareTo(SwAdminUpdate_cmd, UtlString::ignoreCase) == 0)
         {
            stdoutfn.append(SwAdminUpdate_cmd);
            stderrfn.append(SwAdminUpdate_cmd);
         }
         else
            if ( command.compareTo(SwAdminRestart_cmd, UtlString::ignoreCase) == 0)
            {
               stdoutfn.append(SwAdminRestart_cmd);
               stderrfn.append(SwAdminRestart_cmd);
            }
            else
               if ( command.compareTo(SwAdminReboot_cmd, UtlString::ignoreCase) == 0)
               {
                  stdoutfn.append(SwAdminReboot_cmd);
                  stderrfn.append(SwAdminReboot_cmd);
               }
               else
               {
                  return false;
               }

   stderrfn.append(SwAdminStdErr_filetype);
   stdoutfn.append(SwAdminStdOut_filetype);

   return result;
}

/*****************************************************************
 **** SwAdminRpcExec
 *****************************************************************/

const char* SwAdminRpcExec::METHOD_NAME = "SwAdminRpc.exec";

const char* SwAdminRpcExec::name()
{
   return METHOD_NAME;
}

SwAdminRpcExec::SwAdminRpcExec()
{
}

XmlRpcMethod* SwAdminRpcExec::get()
{
   return new SwAdminRpcExec();
}

void SwAdminRpcExec::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, SwAdminRpcExec::get, sipxRpcImpl);
}

bool SwAdminRpcExec::execute(const HttpRequestContext& requestContext,
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
         UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));
         SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_COMMAND, response, status);
         }
         else
         {
            if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlBool   method_result(true);
               UtlString arguments[3];
               OsPath    mWorkingDirectory = ".";
               OsPath    mExec = SipXecsService::Path(SipXecsService::LibExecDirType,"sipxswadmin");
               UtlString mStdOutFile;
               UtlString mStdErrFile;

               UtlString* pSubCommand = dynamic_cast<UtlString*>(params.at(1));

               if ( !buildOutputFiles(*pSubCommand, mStdOutFile, mStdErrFile))
               {
                    // Invalid request. Set a Fault.
                    response.setFault(SwAdminRpcMethod::FailureToLaunch, "Invalid command");
                    status = XmlRpcMethod::FAILED;
                    return result;
               }

               OsPath    mStdOutPath = SipXecsService::Path(SipXecsService::LogDirType, mStdOutFile.data());
               OsPath    mStdErrPath = SipXecsService::Path(SipXecsService::LogDirType, mStdErrFile.data());

               // arguments[0] = mExec.data();
               arguments[0] = pSubCommand->data();
               arguments[1] = NULL;

               // Make sure that there is no other instance running.
               if (! duplicateProcess(SwAdminExec, response, status))
               {
                  // execute the command and return whether or not the launch was successful.
                  OsProcess* swCheck = new OsProcess();

                  // Setup the Standard Output and Standard Error files.
                  OsPath mStdInFile;   // Blank
                  int rc;
                  rc = swCheck->setIORedirect(mStdInFile, mStdOutPath, mStdErrPath);

                  // Launch the process but tell the parent to ignore the child's signals (especially on shutdown).
                  // It will let the system handle it to avoid a defunct process.
                  if ( (rc=swCheck->launch(mExec, &arguments[0], mWorkingDirectory,
                                   swCheck->NormalPriorityClass, FALSE,
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
                     response.setFault(SwAdminRpcMethod::FailureToLaunch, "Failure to launch command");
                     status = XmlRpcMethod::FAILED;
                  }

                  delete swCheck;
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

/*****************************************************************
 **** SwAdminRpcSnapshot
 *****************************************************************/

const char* SwAdminRpcSnapshot::METHOD_NAME = "SwAdminRpc.snapshot";
const char* SwAdminRpcSnapshot::OUTPUT_FILENAME = "sipx-configuration.tar.gz";

const char* SwAdminRpcSnapshot::name()
{
   return METHOD_NAME;
}

SwAdminRpcSnapshot::SwAdminRpcSnapshot()
{
}

XmlRpcMethod* SwAdminRpcSnapshot::get()
{
   return new SwAdminRpcSnapshot();
}

void SwAdminRpcSnapshot::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, SwAdminRpcSnapshot::get, sipxRpcImpl);
}

bool SwAdminRpcSnapshot::execute(const HttpRequestContext& requestContext,
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
         UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));
         SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlSList::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_COMMAND, response, status);
         }
         else
         {
            if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlSList* pArgsList = dynamic_cast<UtlSList*>(params.at(1));
               UtlSListIterator argsListIterator( *pArgsList );
               UtlString * pArg;

               // Make sure that there is no other instance running.
               if (! duplicateProcess(SwAdminSnapshot, response, status))
               {
                  UtlBool   method_result(true);
                  UtlString arguments[pArgsList->entries()+2];
                  UtlString subCommand;
                  OsPath    mWorkingDirectory = ".";
                  OsPath    mExec = SipXecsService::Path(SipXecsService::BinDirType, SwAdminSnapshot);

                  UtlString mStdOutFile(SwAdminSnapshot_cmd);
                  UtlString mStdErrFile(SwAdminSnapshot_cmd);

                  mStdOutFile.append(SwAdminStdOut_filetype);
                  mStdErrFile.append(SwAdminStdErr_filetype);

                  // Construct and set the response.
                  OsPath mStdOutPath = OsPath(SipXecsService::Path(SipXecsService::LogDirType, mStdOutFile.data()));
                  OsPath mStdErrPath = OsPath(SipXecsService::Path(SipXecsService::LogDirType, mStdErrFile.data()));
                  OsPath processOutPath = OsPath(SipXecsService::Path(SipXecsService::TmpDirType, OUTPUT_FILENAME));

                  for (int i = 0; (pArg = dynamic_cast<UtlString*>(argsListIterator())); i++)
                  {
                     XmlUnEscape(arguments[i], *pArg);
                  }

                  arguments[pArgsList->entries()] = processOutPath.data();
                  arguments[pArgsList->entries()+1] = NULL;

                  // execute the command and return whether or not the launch was successful.
                  OsProcess* swCheck = new OsProcess();

                  // Setup the Standard Output and Standard Error files.
                  OsPath mStdInFile;   // Blank
                  int rc;
                  rc = swCheck->setIORedirect(mStdInFile, mStdOutPath, mStdErrPath);

                  // Launch the process but tell the parent to ignore the child's signals (especially on shutdown).
                  // It will let the system handle it to avoid a defunct process.
                  if ( (rc=swCheck->launch(mExec, &arguments[0], mWorkingDirectory,
                                   swCheck->NormalPriorityClass, FALSE,
                                   TRUE)) // Parent to ignore child signals.
                           == OS_SUCCESS )
                  {
                     // Add the file resources to Supervisor Process so they can be retrieved
                     FileResource::logFileResource( mStdOutPath, SipxProcessManager::getInstance()->findProcess(SUPERVISOR_PROCESS_NAME));
                     FileResource::logFileResource( mStdErrPath, SipxProcessManager::getInstance()->findProcess(SUPERVISOR_PROCESS_NAME));
                     FileResource::logFileResource( processOutPath, SipxProcessManager::getInstance()->findProcess(SUPERVISOR_PROCESS_NAME));

                     UtlString outputFilename = processOutPath;

                     // Construct and set the response.
                     UtlSList outputPaths;
                     outputPaths.insert(&outputFilename);
                     outputPaths.insert(&mStdOutPath);
                     outputPaths.insert(&mStdErrPath);

                     response.setResponse(&outputPaths);
                     status = XmlRpcMethod::OK;
                     result = true;
                  } // launch
                  else
                  {
                     // Failed to launch the command, send a fault.
                     response.setFault(SwAdminRpcMethod::FailureToLaunch, "Failure to launch command");
                     status = XmlRpcMethod::FAILED;
                  }

                  delete swCheck;
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

/*****************************************************************
 **** SwAdminRpcExecQuery
 *****************************************************************/

const char* SwAdminRpcExecStatus::METHOD_NAME = "SwAdminRpc.execStatus";
const char* SwAdminRpcExecStatus::PROCESS_RUNNING = "RUNNING";
const char* SwAdminRpcExecStatus::PROCESS_NOT_RUNNING = "NOT_RUNNING";

const char* SwAdminRpcExecStatus::name()
{
   return METHOD_NAME;
}

SwAdminRpcExecStatus::SwAdminRpcExecStatus()
{
}

XmlRpcMethod* SwAdminRpcExecStatus::get()
{
   return new SwAdminRpcExecStatus();
}

void SwAdminRpcExecStatus::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, SwAdminRpcExecStatus::get, sipxRpcImpl);
}

bool SwAdminRpcExecStatus::isQueryValid(const UtlString& query, UtlString& processName)
{
   if (query.compareTo(SwAdminSnapshot_cmd, UtlString::ignoreCase) ==0)
   {
      processName = SwAdminSnapshot;
      return true;
   }
   else if (query.compareTo(SwAdminCheckUpdate_cmd, UtlString::ignoreCase) == 0 ||
            query.compareTo(SwAdminUpdate_cmd, UtlString::ignoreCase) == 0 ||
            query.compareTo(SwAdminVersion_cmd, UtlString::ignoreCase) == 0)
   {
      processName = SwAdminExec;
      return true;
   }

   return false;
}

bool SwAdminRpcExecStatus::execute(const HttpRequestContext& requestContext,
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
         UtlString* pCallingHostname = dynamic_cast<UtlString*>(params.at(0));
         SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

         if (!params.at(1) || !params.at(1)->isInstanceOf(UtlString::TYPE))
         {
            handleMissingExecuteParam(name(), PARAM_NAME_COMMAND, response, status);
         }
         else
         {
            if (validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
            {
               UtlString* query = dynamic_cast<UtlString*>(params.at(1));
               UtlString processName;

               // Make sure that the query is valid
               if (isQueryValid(*query, processName))
               {
                  UtlString stat;

                  if (isProcessActive(processName))
                  {
                     stat = PROCESS_RUNNING;
                  }
                  else
                  {
                     stat = PROCESS_NOT_RUNNING;
                  }

                  response.setResponse(&stat);
                  status = XmlRpcMethod::OK;
                  result = true;

               } // wrong Query String
               else
               {
                  // Failed to launch the command, send a fault.
                  response.setFault(SwAdminRpcMethod::FailureToLaunch, "Invalid query");
                  status = XmlRpcMethod::FAILED;
               }
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
