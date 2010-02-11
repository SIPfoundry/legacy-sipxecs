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
#include "os/OsSysLog.h"
#include "os/OsFileSystem.h"
#include "net/XmlRpcDispatch.h"
#include "net/XmlRpcMethod.h"
#include "net/XmlRpcRequest.h"
#include "net/NetBase64Codec.h"

#include "SipxRpc.h"
#include "FileResource.h"
#include "FileResourceManager.h"
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

void FileRpcMethod::registerSelf(SipxRpc & sipxRpcImpl)
{
   assert(false);  // this should have been overridden in the subclass
}

FileRpcMethod::FileRpcMethod()
{
}

void FileRpcMethod::registerMethod(const char*       methodName,
                                   XmlRpcMethod::Get getMethod,
                                   SipxRpc &         sipxRpcImpl
                                   )
{
   sipxRpcImpl.getXmlRpcDispatch()->addMethod(methodName, getMethod, &sipxRpcImpl );
}

bool FileRpcMethod::execute(const HttpRequestContext& requestContext,
                            UtlSList&                 params,
                            void*                     userData,
                            XmlRpcResponse&           response,
                            ExecutionStatus&          status
                            )
{
   assert(false); // this should have been overridden in the subclass

   return false;
}

bool FileRpcMethod::validCaller(const HttpRequestContext& requestContext,
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
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
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
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, faultMsg);
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

void FileRpcReplaceFile::registerSelf(SipxRpc & sipxRpcImpl)
{
   registerMethod(METHOD_NAME, FileRpcReplaceFile::get, sipxRpcImpl);
}

bool FileRpcReplaceFile::execute(const HttpRequestContext& requestContext,
                                 UtlSList&                 params,
                                 void*                     userData,
                                 XmlRpcResponse&           response,
                                 ExecutionStatus&          status)
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
                  SipxRpc* pSipxRpcImpl = ((SipxRpc *)userData);

                  if(validCaller(requestContext, *pCallingHostname, response, *pSipxRpcImpl, name()))
                  {
                     // Check the resource permissions.  To be added when available.
                     FileResource* fileResource =
                        FileResourceManager::getInstance()->find(pfileName->data());
                     if (!fileResource)
                     {
                        UtlString faultMsg;
                        faultMsg.append("File '");
                        faultMsg.append(*pfileName);
                        faultMsg.append("' not declared as a resource by any sipXecs process");
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileRpc::replaceFile %s",
                                      faultMsg.data());
                        result=false;
                        response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
                     }
                     else if (!fileResource->isWriteable())
                     {
                        UtlString faultMsg;
                        faultMsg.append("Write access to '");
                        faultMsg.append(*pfileName);
                        faultMsg.append("' denied by process resource ");
                        fileResource->appendDescription(faultMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileRpc::replaceFile %s",
                                      faultMsg.data());
                        result=false;
                        response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
                     }
                     else if (   ( pfilePermissions->getValue() <= minPermissions )
                              || ( pfilePermissions->getValue() > maxPermissions ))
                     {
                        UtlString faultMsg;
                        faultMsg.appendNumber(pfilePermissions->getValue(),"File permissions %04o");
                        faultMsg.appendNumber(minPermissions,"not within valid range (%04o - ");
                        faultMsg.appendNumber(maxPermissions,"%04o)");
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "FileRpc::replaceFile %s",
                                      faultMsg.data());
                        result = false;
                        response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
                     }
                     else
                     {
                        // Write out the file.
                        UtlString* pfileData = dynamic_cast<UtlString*>(params.at(3));
                        UtlString faultMsg;
                        if((result=replicateFile(*pfileName,*pfilePermissions,*pfileData,faultMsg )))
                        {
                           // Construct and set the response.
                           response.setResponse(&method_result);
                           status = XmlRpcMethod::OK;

                           // Tell anyone who cares that this changed
                           fileResource->modified();
                        }
                        else
                        {
                           // Replication failed.
                           response.setFault(FileRpcMethod::InvalidParameter, faultMsg);
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return result;
}

bool FileRpcReplaceFile::replicateFile(UtlString& path_and_name,
                                       UtlInt&    file_permissions,
                                       UtlString& file_content,
                                       UtlString& errorMsg
                                       )
{
   OsStatus rc;
   bool     result = false;
   errorMsg.remove(0);

   UtlString temporaryLocation(path_and_name);
   temporaryLocation += ".new";
   OsFile temporaryFile(temporaryLocation);

   // create a new file with the specified path and file name except with a .new extension.
   if (temporaryFile.open(OsFile::CREATE) == OS_SUCCESS)
   {
      UtlString     pdecodedData;
      size_t bytesRead;

      if ( !NetBase64Codec::decode( file_content, pdecodedData ))
      {
         // Write failed.
         errorMsg.append("Failed to decode file data from base64 for '");
         errorMsg.append(path_and_name);
         errorMsg.append("'");
         OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "FileRpcReplaceFile::replicateFile %s",
                       errorMsg.data());
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
               OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "FileRpcReplaceFile::replicateFile"
                                   " updated file '%s'", path_and_name.data());
               rc = temporaryFile.remove(TRUE);

               // Change the permissions on the file as indicated.
               int_rc = chmod( path_and_name.data(), file_permissions.getValue() );
               result = true;
            }
            else
            {
               // Write succeeded, but rename failed.
               errorMsg.append("Failed to rename temporary file from '");
               errorMsg.append(temporaryLocation);
               errorMsg.append("' to '");
               errorMsg.append(path_and_name);
               errorMsg.append("'");
               OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "FileRpcReplaceFile::replicateFile %s",
                             errorMsg.data());
            }
         }
         else
         {
            // Write failed.
            errorMsg.append("Failed to write to temporary file '");
            errorMsg.append(temporaryLocation);
            errorMsg.append("'");
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "FileRpcReplaceFile::replicateFile %s",
                          errorMsg.data());
         }
      }
   }
   else
   {
      // Open failed.
      errorMsg.append("Failed to create temporary file '");
      errorMsg.append(temporaryLocation);
      errorMsg.append("'");
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "FileRpcReplaceFile::replicateFile %s",
                    errorMsg.data());
   }

   return result;
}

