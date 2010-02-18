//
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsFS.h"
#include "utl/UtlDefs.h"
#include "net/HttpMessage.h"
#include "net/HttpRequestContext.h"
#include "net/HttpServer.h"
#include "FileResource.h"
#include "FileResourceManager.h"
#include "HttpFileAccess.h"

// DEFINES
// CONSTANTS

const size_t HttpFileAccess::MAX_FILE_CHUNK_SIZE = (16 * 1024);

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
HttpFileAccess::HttpFileAccess(HttpServer* httpServer,
                               SipxRpc*    rpcService
                               )
   : mSipxRpc(rpcService)
{
   httpServer->addHttpService("/", this); // assume responsibility for any path
};

/// Provide access to files as allowed by process definitions.
void HttpFileAccess::processRequest(const HttpRequestContext& requestContext,
                                    const HttpMessage& request,
                                    HttpMessage*& response
                                    )
{
   UtlString message;
   response = new HttpMessage();

   UtlString peerName;
   if (mSipxRpc->isAllowedPeer(requestContext, peerName))
   {
      if (requestContext.methodIs(HTTP_GET_METHOD))
      {
         UtlString path;
         requestContext.getMappedPath(path);

         FileResource* resource = FileResourceManager::getInstance()->find(path);
         if (resource)
         {
            if (resource->isReadable())
            {
               sendFile(path, peerName, requestContext, request, response);
            }
            else
            {
               message.append("resource ");
               resource->appendDescription(message);
               message.append(" does not allow write access to '");
               message.append(path);
               message.append("'");
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                             peerName.data(), message.data());

               response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                    HTTP_FORBIDDEN_CODE,
                                                    "Access denied by process definition");
               response->setBody(new HttpBody(message.data(),message.length()));
               response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
               response->setContentLength(message.length());
            }
         }
         else
         {
            message.append("File resource '");
            message.append(path);
            message.append("' not known to sipXsupervisor.");
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                          peerName.data(), message.data());

            response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                 HTTP_FILE_NOT_FOUND_CODE,
                                                 HTTP_FILE_NOT_FOUND_TEXT);
            response->setBody(new HttpBody(message.data(),message.length()));
            response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
            response->setContentLength(message.length());
         }
      }
      else if (requestContext.methodIs(HTTP_DELETE_METHOD))
      {
         UtlString path;
         requestContext.getMappedPath(path);

         FileResource* resource = FileResourceManager::getInstance()->find(path);
         if (resource)
         {
            if (resource->isWriteable())
            {
               OsPath filePath(path);
               if (OsFileSystem::exists(filePath))
               {
                  if (OS_SUCCESS
                      == OsFileSystem::remove(filePath, TRUE /* recursive */, TRUE /* force */))
                  {
                     message.append("File '");
                     message.append(path);
                     message.append("' deleted");
                     OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "HttpFileAccess::processRequest from %s %s",
                                   peerName.data(), message.data());
                  
                     response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION_1_1,
                                                          HTTP_OK_CODE, "Deleted");
                     response->setContentLength(0);

                     // tell anyone who cares that this has changed
                     resource->modified();
                  }
                  else
                  {
                     int       httpStatusCode;
                     UtlString httpStatusText;

                     switch (errno)
                     {
                     case EACCES:
                        httpStatusCode = HTTP_FORBIDDEN_CODE;
                        httpStatusText = "File Access Denied";
                        break;

                     default:
                        httpStatusCode = HTTP_SERVER_ERROR_CODE;
                        httpStatusText.append("Unknown error ");
                        httpStatusText.appendNumber(errno);
                        break;
                     }

                     message.append("File '");
                     message.append(path);
                     message.append("' errno ");
                     message.appendNumber(errno);
                     message.append(" ");

                     char errnoMsg[1024];
                     strerror_r(errno, errnoMsg, sizeof(errnoMsg));
                     message.append(errnoMsg);

                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                                   peerName.data(), message.data());

                     response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                          httpStatusCode,
                                                          httpStatusText);
                     response->setBody(new HttpBody(message.data(),message.length()));
                     response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
                     response->setContentLength(message.length());
                  }                  
               }
               else
               {
                  message.append("File to be deleted '");
                  message.append(path);
                  message.append("' does not exist");
                  OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "HttpFileAccess::processRequest from %s %s",
                                peerName.data(), message.data());
                  
                  response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION_1_1,
                                                       HTTP_OK_CODE, "File does not exist");
                  response->setContentLength(0);
               }
            }
            else
            {
               message.append("resource ");
               resource->appendDescription(message);
               message.append(" does not allow write access to '");
               message.append(path);
               message.append("'");
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                             peerName.data(), message.data());

               response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                    HTTP_FORBIDDEN_CODE,
                                                    "Access denied by process definition");
               response->setBody(new HttpBody(message.data(),message.length()));
               response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
               response->setContentLength(message.length());
            }
         }
         else
         {
            message.append("File resource '");
            message.append(path);
            message.append("' not known to sipXsupervisor.");
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                          peerName.data(), message.data());

            response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                 HTTP_FILE_NOT_FOUND_CODE,
                                                 HTTP_FILE_NOT_FOUND_TEXT);
            response->setBody(new HttpBody(message.data(),message.length()));
            response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
            response->setContentLength(message.length());
         }
      }
      else
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest %s from %s",
                       HTTP_UNSUPPORTED_METHOD_TEXT, peerName.data());

         response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                              HTTP_UNSUPPORTED_METHOD_CODE,
                                              HTTP_UNSUPPORTED_METHOD_TEXT);
      }
   }
   else
   {
      message.append("Request not supported from untrusted peer.");
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest %s",
                    message.data());

      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                           HTTP_FORBIDDEN_CODE, HTTP_FORBIDDEN_TEXT);

      response->setBody(new HttpBody(message.data(),message.length()));
      response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
      response->setContentLength(message.length());
   }
}


void HttpFileAccess::sendFile(const UtlString& path,
                              const UtlString& peerName,
                              const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response
                              )
{
   UtlString message;
   
   int file = open(path.data(), O_RDONLY);
   if (0 <= file)
   {
      UtlString fileBuffer;
      if (fileBuffer.capacity(MAX_FILE_CHUNK_SIZE + 1) >= MAX_FILE_CHUNK_SIZE + 1)
      {
         response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION_1_1,
                                              HTTP_OK_CODE, HTTP_OK_TEXT);
         response->useChunkedBody(true);

         // @TODO Annotate file resources with a mime type?

         // add a Content-Disposition header to suggest saving as the basename of the file
         response->setContentType("application/octet-stream");
         UtlString dispositionValue;
         UtlString basename;
         ssize_t finalSlash = path.last('/');
         if (UTL_NOT_FOUND != finalSlash)
         {
            basename.append(path, finalSlash+1, path.length()-finalSlash);
         }
         else
         {
            basename.append(path); // don't think this should ever be true....
         }
         dispositionValue.append("attachment; filename=\"");
         dispositionValue.append(basename);
         dispositionValue.append("\"");
         response->setHeaderValue(HTTP_CONTENT_DISPOSITION_FIELD,dispositionValue.data());

         bool    writtenOk = response->writeHeaders(requestContext.socket());
         ssize_t bytes;
         Int64   totalBytes = 0;
         Int64   chunks = 0;
         while (   writtenOk
                && (bytes = read(file, (void*)fileBuffer.data(),
                                 MAX_FILE_CHUNK_SIZE)))
         {
            fileBuffer.setLength(bytes);
            writtenOk = response->writeChunk(requestContext.socket(), fileBuffer);
            if (writtenOk)
            {
               totalBytes += bytes;
               chunks++;
            }
            OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                          "file block %"FORMAT_INTLL"d %zd %s", chunks, bytes,
                          writtenOk ? "ok" : "failed");
         }
         if (writtenOk)
         {
            response->writeEndOfChunks(requestContext.socket());

            message.append(" sent file '");
            message.append(path);
            message.append("' (");
            message.appendNumber(totalBytes,"%"FORMAT_INTLL"d");
            message.append(" bytes in ");
            message.appendNumber(chunks,"%"FORMAT_INTLL"d");
            message.append(" chunks) to peer ");
            message.append(peerName);
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "HttpFileAccess::processRequest %s",
                          message.data());
         }
         else
         {
            message.append("error writing response after ");
            message.appendNumber(totalBytes,"%"FORMAT_INTLL"d");
            message.append(" body bytes in ");
            message.appendNumber(chunks,"%"FORMAT_INTLL"d");
            message.append(" chunks) to peer ");
            message.append(peerName);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest %s",
                          message.data());
         }

         /*
          * We've already written the response, so prevent HttpServer from sending it by
          * not passing it back.
          */
         delete response;
         response = NULL;
      }
      else
      {
         // Send out-of-resources message
         message.append("Supervisor Buffer Exhausted");
         response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                              HTTP_OUT_OF_RESOURCES_CODE,
                                              message.data());
         response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
         response->setBody(new HttpBody(message.data(),message.length()));
         response->setContentLength(message.length());

         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "HttpFileAccess::processRequest from %s %s",
                       peerName.data(), message.data());
      }
   }
   else
   {
      int       httpStatusCode;
      UtlString httpStatusText;

      switch (errno)
      {
      case EACCES:
         httpStatusCode = HTTP_FORBIDDEN_CODE;
         httpStatusText = "File Access Denied";
         break;

      case ENOENT:
         httpStatusCode = HTTP_FILE_NOT_FOUND_CODE;
         httpStatusText = HTTP_FILE_NOT_FOUND_TEXT;
         break;

      default:
         httpStatusCode = HTTP_SERVER_ERROR_CODE;
         httpStatusText = HTTP_SERVER_ERROR_TEXT;
         break;
      }

      message.append("File '");
      message.append(path);
      message.append("' errno ");
      message.appendNumber(errno);
      message.append(" ");

      char errnoMsg[1024];
      strerror_r(errno, errnoMsg, sizeof(errnoMsg));
      message.append(errnoMsg);

      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "HttpFileAccess::processRequest from %s %s",
                    peerName.data(), message.data());

      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                           httpStatusCode,
                                           httpStatusText);
      response->setBody(new HttpBody(message.data(),message.length()));
      response->setContentType(CONTENT_TYPE_TEXT_PLAIN);
      response->setContentLength(message.length());
   }
}

/// destructor
HttpFileAccess::~HttpFileAccess()
{
   // do not delete mSipxRpc - it is owned by sipXsupervisor
};
