//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <sys/stat.h>
#include "utl/UtlBool.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "utl/UtlHashMapIterator.h" 
#include "utl/UtlSListIterator.h" 
#include "os/OsProcessMgr.h"
#include "os/OsSysLog.h"
#include "os/OsFileSystem.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "net/NetBase64Codec.h"
#include "WatchDog.h"

#include "FileRpc.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/*************************************************************************
 **** FileRpcMethod contains common code for FileRpc methods
 ************************************************************************/
const char* FileRpcMethod::METHOD_NAME = "FileRpc.BASE";
const char* FileRpcMethod::PARAM_NAME_CALLING_HOST = "callingHostname";
const char* FileRpcMethod::PARAM_NAME_FILE_NAME = "fileName";
const char* FileRpcMethod::PARAM_NAME_FILE_PERMISSIONS = "filePermissions";
const char* FileRpcMethod::PARAM_NAME_FILE_DATA = "filedata";


XmlRpcMethod* FileRpcMethod::get()
{
   assert(false);  // this should have been overridden in the subclass

   return NULL;
}

void FileRpcMethod::registerSelf(WatchDog & watchdog)
{
   assert(false);  // this should have been overridden in the subclass
}

FileRpcMethod::FileRpcMethod()
{
}

void FileRpcMethod::registerMethod(const char*       methodName,
                                       XmlRpcMethod::Get getMethod,
                                       WatchDog &           watchdog 
                                       )
{
   watchdog.getXmlRpcDispatch()->addMethod(methodName, getMethod, &watchdog );
}

bool FileRpcMethod::execute(const HttpRequestContext& requestContext,
                                UtlSList& params,
                                void* userData,
                                XmlRpcResponse& response,
                                ExecutionStatus& status
                                )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool FileRpcMethod::validCaller(const HttpRequestContext& requestContext,
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
         // sipXsupervisor says it is one of the allowed peers.
         result = true;
         OsSysLog::add(FAC_WATCHDOG, PRI_DEBUG,
                       "FileRpcMethod::validCaller '%s' peer authenticated for %s",
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
         response.setFault(FileRpcMethod::UnconfiguredPeer, faultMsg.data());
            
         OsSysLog::add(FAC_WATCHDOG, PRI_ERR,
                       "%s failed - '%s' not a configured peer",
                       callingMethod, peerName.data()
                       );
      }
   }
   else
   {
      // ssl says not authenticated - provide only a generic error
      response.setFault(XmlRpcResponse::AuthenticationRequired, "TLS Peer Authentication Failure");
            
      OsSysLog::add(FAC_WATCHDOG, PRI_ERR,
                    "%s failed: '%s' failed SSL authentication",
                    callingMethod, peerName.data()
                    );
   }

   return result;
}

void FileRpcMethod::handleMissingExecuteParam(const char* methodName,
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
   response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_WATCHDOG, PRI_ERR, faultMsg);
}

void FileRpcMethod::handleExtraExecuteParam(const char* methodName,
                                                XmlRpcResponse& response,
                                                ExecutionStatus& status)
{
   UtlString faultMsg;
   faultMsg += methodName;
   faultMsg += " has incorrect number of parameters";
   status = XmlRpcMethod::FAILED;
   response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
   OsSysLog::add(FAC_WATCHDOG, PRI_ERR, faultMsg);
}


/*****************************************************************
 **** FileRpcReplaceFile
 *****************************************************************/

const char* FileRpcReplaceFile::METHOD_NAME = "File.replace";

const char* FileRpcReplaceFile::name()
{
   return METHOD_NAME;
}

FileRpcReplaceFile::FileRpcReplaceFile()
{
}

XmlRpcMethod* FileRpcReplaceFile::get()
{
   return new FileRpcReplaceFile();
}

void FileRpcReplaceFile::registerSelf(WatchDog & watchdog)
{
   registerMethod(METHOD_NAME, FileRpcReplaceFile::get, watchdog);
}

bool FileRpcReplaceFile::execute(const HttpRequestContext& requestContext,
                                     UtlSList& params,
                                     void* userData,
                                     XmlRpcResponse& response,
                                     ExecutionStatus& status)
{

   const int  minPermissions = 0100;
   const int  maxPermissions = 0777;

   bool result = false;
   status = XmlRpcMethod::FAILED;

   if (4 != params.entries())
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
            handleMissingExecuteParam(name(), PARAM_NAME_FILE_NAME, response, status);
         }
         else
         {
            UtlString* pfileName = dynamic_cast<UtlString*>(params.at(1));

            if (!params.at(2) || !params.at(2)->isInstanceOf(UtlInt::TYPE))
            {
               handleMissingExecuteParam(name(), PARAM_NAME_FILE_PERMISSIONS, response, status);
            }
            else
            {
               UtlInt* pfilePermissions = dynamic_cast<UtlInt*>(params.at(2));
           
               if (!params.at(3) || !params.at(3)->isInstanceOf(UtlString::TYPE))
               {
                  handleMissingExecuteParam(name(), PARAM_NAME_FILE_DATA, response, status);
               }
               else
               {
                  UtlBool method_result(true);
                  WatchDog* pWatchDog = ((WatchDog *)userData);

                  if(validCaller(requestContext, *pCallingHostname, response, *pWatchDog, name()))
                  {
                     OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                                   "FileRpc::replaceFile"
                                   " host %s requested process states",
                                   pCallingHostname->data()
                                   );

                  }

                  // Check the resource permissions.  To be added when available.
 
                  // Check that the file permissions is within a valid range.
                  if ( ( pfilePermissions->getValue() <= minPermissions ) || ( pfilePermissions->getValue() > maxPermissions ))
                  {
                     result = false;
                     method_result.setValue(false);
                     OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                                   "FileRpc::replaceFile"
                                   " File permissions not within valid range");
                  }
                  else
                  {
                     // Write out the file.
                     UtlString* pfileData = dynamic_cast<UtlString*>(params.at(3));
                     if ( replicateFile( *pfileName, *pfilePermissions, *pfileData ) )
                     {
                        result = true;
                     }
                     else
                     {
                        // Replication failed.
                        result = false;
                        method_result.setValue(false);
                     }
                  }

                  // Construct and set the response.
                  response.setResponse(&method_result);
                  status = XmlRpcMethod::OK;
               }
            }
         }
      }
   }

   return result;
}

bool FileRpcReplaceFile::replicateFile(UtlString& path_and_name, UtlInt& file_permissions, UtlString& file_content)
{
   OsStatus rc;
   bool     result;

   result = false;

   UtlString temporaryLocation(path_and_name);
   temporaryLocation += ".new";
   OsFile temporaryFile(temporaryLocation);

   // create a new file with the specified path and file name except with a .new extension.
   if (temporaryFile.open(OsFile::CREATE) == OS_SUCCESS)
   {
      UtlString     pdecodedData;
      unsigned long bytesRead;

      if ( !NetBase64Codec::decode( file_content, pdecodedData ))
      {
         result = false;
      }
      else
      {
         rc = temporaryFile.write(pdecodedData.data(), pdecodedData.length(), bytesRead);
         temporaryFile.close();
         if (rc == OS_SUCCESS)
         {
            rc = temporaryFile.rename(path_and_name);
            if (rc == OS_SUCCESS)
            {
               int int_rc;
               OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                                   "FileRpc::replaceFile"
                                   " updated file: %s", temporaryLocation.data());

               rc = temporaryFile.remove(TRUE);

               // Change the permissions on the file as indicated.
               int_rc = chmod( path_and_name.data(), file_permissions.getValue() ); 
               result = true;
            }
            else
            {
               // Write succeeded, but rename failed.
               OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                                   "FileRpc::replaceFile"
                                   " Failure to rename temporary file: %s", temporaryLocation.data());
               result = false;
            }
         }
         else
         {
               // Write failed.
               OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                                   "FileRpc::replaceFile"
                                   " Failure to write to temporary file: %s", temporaryLocation.data());
               result = false;
         }
      }
   }
   else
   {
      // Open failed.
      OsSysLog::add(FAC_WATCHDOG, PRI_INFO,
                           "FileRpc::replaceFile"
                           " Failure to create temporary file: %s", temporaryLocation.data());
      result = false;
   }


   return result;
}

