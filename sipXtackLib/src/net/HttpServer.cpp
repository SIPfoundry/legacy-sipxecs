//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <stdio.h>
#if defined(_WIN32)
#   include <io.h>
#elif defined(_VXWORKS)
#   include <unistd.h>
#   include <dirent.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#   include <stdlib.h>
#   define O_BINARY 0 // There is no notion of a "not binary" file under POSIX,
                      // so we just set O_BINARY used below to no bits in the mask.
#else
#   error Unsupported target platform.
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "os/OsDefs.h"
#include "os/OsSysLog.h"

// APPLICATION INCLUDES
#include <os/OsServerSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsConfigDb.h>
#include <utl/UtlVoidPtr.h>
#include <net/HttpMessage.h>
#include <net/HttpServer.h>
#include <net/HttpService.h>
#include <net/HttpBody.h>
#include <net/MimeBodyPart.h>
#include <net/HttpRequestContext.h>
#include <net/NetAttributeTokenizer.h>
#include <net/NetMd5Codec.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const int HttpServer::MAX_PERSISTENT_HTTP_CONNECTIONS = 5; ///< this should be a parameter

#ifdef _VXWORKS
#   define O_BINARY 0
#   define S_IREAD 0
#   define S_IWRITE 0
#endif

// STATIC VARIABLE INITIALIZATIONS

#ifdef TEST_UPLOAD_FILE_DEBUG
void incrementalCheckSum(unsigned int* checkSum, const char* buffer, ssize_t bufferLength)
{
    ssize_t integerIndex = 0;
    while(integerIndex < bufferLength)
    {
        (*checkSum) += (*buffer);
        buffer++;
        integerIndex++;
    }
}
#endif
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpServer::HttpServer(OsServerSocket *pSocket,
                       OsConfigDb* validIpAddressDB,
                       bool bPersistentConnection) :
   OsTask("HttpServer-%d"),
   httpStatus(OS_TASK_NOT_STARTED),
   mpServerSocket(pSocket),
   mpValidIpAddressDB(validIpAddressDB),
   mbPersistentConnection(bPersistentConnection),
   mHttpConnections(0),
   mpHttpConnectionList(new UtlSList)
{
   if(mpValidIpAddressDB)
   {
      loadValidIpAddrList();
   }

   if (!mpHttpConnectionList)
   {
      mbPersistentConnection = false;
      OsSysLog::add( FAC_SIP, PRI_CRIT, "HttpServer failed to allocate mpHttpConnectionList");
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "HttpServer: Using persistent connections" );
   }

}

void HttpServer::loadValidIpAddrList()
{
   //maybe there should be no reference to OsConfig DB here and the hashTable should be passed
   // as paramenter to HtppServer. If time permits I will make the change. - SDUA
   UtlString strKey;
   int i = 1;
   while(true)
   {
      char szTempBuf[32] ;
      sprintf(szTempBuf, "%d", i) ;
      UtlString ipAddress;

      if( mpValidIpAddressDB->get(szTempBuf, ipAddress) && !ipAddress.isNull())
      {
         UtlString *pMatchIpAddress = new UtlString(ipAddress);
         mValidIpAddrList.insert(pMatchIpAddress);
         i++;
         continue;
      }
      else
      {
         //no more ip addresses
         break;
      }
   }
}

// Copy constructor
HttpServer::HttpServer(const HttpServer& rHttpServer)
{
}

// Destructor
HttpServer::~HttpServer()
{
    if(mpServerSocket)
    {
        // Close the server socket to unblock the listener
        mpServerSocket->close();
    }

    // Wait until run exits before clobbering members
    waitUntilShutDown();

    /// mpServerSocket is not deleted - the caller of the constructor owns it

    if(mpValidIpAddressDB)
    {
       delete mpValidIpAddressDB;
       mpValidIpAddressDB = NULL;
       mValidIpAddrList.destroyAll();
    }

    mUriMaps.destroyAll();                 // Delete all of the Url mappings
    mRequestProcessorMethods.destroyAll(); // Delete all of the processor mappings

    if (!mHttpServices.isEmpty())
    {
       UtlHashMapIterator httpServices(mHttpServices);
       UtlString* serviceUri;
       while ((serviceUri = dynamic_cast<UtlString*>(httpServices())))
       {
          delete mHttpServices.removeReference(serviceUri); // remove and delete service path
          // don't delete the value - that is owned by the caller who added it.
       }
    }

    // Delete remaining HttpConnections
    if (mpHttpConnectionList)
    {
        mpHttpConnectionList->destroyAll();
        delete mpHttpConnectionList;
        mpHttpConnectionList = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HttpServer&
HttpServer::operator=(const HttpServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus HttpServer::getStatus()
{
        return httpStatus;
}

UtlBoolean HttpServer::isSocketOk() const
{
    UtlBoolean bOk = FALSE;

    if (mpServerSocket)
    {
        bOk = mpServerSocket->isOk();
    }

    return bOk;
}

int HttpServer::run(void* runArg)
{
    OsConnectionSocket* requestSocket = NULL;

    if (!mpServerSocket->isOk())
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "HttpServer: port not ok" );
        httpStatus = OS_PORT_IN_USE;
    }

    while(!isShuttingDown() && mpServerSocket->isOk())
    {
        requestSocket = mpServerSocket->accept();

        if(requestSocket)
        {
            if (mbPersistentConnection)
            {
                // Take this opportunity to check for any old HttpConnections that can be deleted
                int items = mpHttpConnectionList->entries();
                if (items != 0)
                {
                    int deleted = 0;

                    UtlSListIterator iterator(*mpHttpConnectionList);
                    HttpConnection* connection;
                    while ((connection = dynamic_cast<HttpConnection*>(iterator())))
                    {
                        if (connection->toBeDeleted())
                        {
                           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                         "HttpServer: destroying connection %p",
                                         connection);
                           mpHttpConnectionList->destroy(connection);
                           ++deleted;

                           if (mHttpConnections > 0)
                           {
                              --mHttpConnections;
                           }
                        }
                    }
                    items = mpHttpConnectionList->entries();
                    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                  "HttpServer: "
                                  "destroyed %d inactive HttpConnections, %d remaining",
                                  deleted, items);
                }
                // Create new persistent connection
                if (mHttpConnections < MAX_PERSISTENT_HTTP_CONNECTIONS)
                {
                    ++mHttpConnections;
                    HttpConnection* newConnection = new HttpConnection(requestSocket, this);
                    mpHttpConnectionList->append(newConnection);
                    OsSysLog::add(FAC_SIP, PRI_INFO,
                                  "HttpServer::run starting persistent connection %d (%p)",
                                  mHttpConnections, newConnection);
                    newConnection->start();
                }
                else
                {
                   OsSysLog::add(FAC_SIP, PRI_ERR,
                                 "HttpServer::run exceeded persistent connection limit (%d):"
                                 " sending 503",
                                 MAX_PERSISTENT_HTTP_CONNECTIONS);
                    HttpMessage request;
                    HttpMessage response;
                    // Read the http request from the socket
                    request.read(requestSocket);

                    // Send out-of-resources message
                    response.setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                                        HTTP_OUT_OF_RESOURCES_CODE,
                                                        HTTP_OUT_OF_RESOURCES_TEXT);
                    response.write(requestSocket);
                    requestSocket->close();
                    delete requestSocket;
                    requestSocket = NULL;
                }
            }
            else
            {
                HttpMessage request;
                // Read a http request from the socket
                request.read(requestSocket);

                UtlString remoteIp;
                requestSocket->getRemoteHostIp(&remoteIp);

                HttpMessage* response = NULL;

                // If request from Valid IP Address
                if( processRequestIpAddr(remoteIp, request, response))
                {
                   // If the request is authorized
                   processRequest(request, response, requestSocket);
                }

                if(response)
                {
                    response->write(requestSocket);
                    delete response;
                    response = NULL;
                }

                requestSocket->close();
                delete requestSocket;
                requestSocket = NULL;
            }
        }
        else
        {
           httpStatus = OS_PORT_IN_USE;
        }
    } // while (!isShuttingDown && mpServerSocket->isOk())

    if ( !isShuttingDown() )
    {
       OsSysLog::add( FAC_SIP, PRI_ERR, "HttpServer: exit due to port failure" );
    }

    httpStatus = OS_TASK_NOT_STARTED;

    return(TRUE);
}

UtlBoolean HttpServer::processRequestIpAddr(const UtlString& remoteIp,
                                      const HttpMessage& request,
                                      HttpMessage*& response)
{
   UtlBoolean isValidIp = FALSE;
   UtlString remoteAddress(remoteIp);
   UtlString matchIp(remoteAddress);

   if(mValidIpAddrList.isEmpty() || mValidIpAddrList.find(&matchIp))
   {
      isValidIp = TRUE;
   }
   else
   {
      response = new HttpMessage();
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
      HTTP_FORBIDDEN_CODE, HTTP_FORBIDDEN_TEXT);

      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "HTTP Request from non-allowed IP address: %s disallowed",
                    remoteAddress.data());

   }
   return  isValidIp ;
}

void HttpServer::processRequest(const HttpMessage& request,
                                HttpMessage*& response,
                                OsConnectionSocket* connection
                                )
{
    UtlString method;
    response = NULL;

    if(true) // used to be authorization check, but I don't want to change all indenting
    {
        request.getRequestMethod(&method);
        method.toUpper();
        UtlString uri;
        request.getRequestUri(&uri);

        UtlString uriFileName(uri);
        ssize_t fileNameEnd = -1;
        if(method.compareTo(HTTP_GET_METHOD) == 0)
        {
            fileNameEnd = uriFileName.first('?');
            if(fileNameEnd > 0)
            {
               uriFileName.remove(fileNameEnd);
            }
        }

        UtlString mappedUriFileName;
        if (uriFileName.contains(".."))
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "HttpServer::processRequest "
                          "Disallowing URI: '%s' because it contains '..'",
                          uriFileName.data());

            // Disallow relative path names going up for security reasons
            mappedUriFileName.append("/");
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_INFO, "HttpServer::processRequest "
                          "%s '%s'", method.data(), uriFileName.data());

            // Map the file name
            mapUri(mUriMaps, uriFileName.data(), mappedUriFileName);
        }

        // Build the request context
        HttpRequestContext requestContext(method.data(),
                                          uri.data(),
                                          mappedUriFileName.data(),
                                          NULL,
                                          NULL, // was userid
                                          connection
                                          );

        if(requestContext.methodIs(HTTP_POST_METHOD))
        {
            //Need to get the CGI/form variables from the body.
            const HttpBody* body = request.getBody();
            if(body  && !body->isMultipart())
            {
                requestContext.extractPostCgiVariables(*body);
            }
        }

        RequestProcessor* requestProcessorPtr = NULL;
        HttpService* pService = NULL;

        if(   (   requestContext.methodIs(HTTP_GET_METHOD)
               || requestContext.methodIs(HTTP_POST_METHOD)
               )
           && findRequestProcessor(uriFileName.data(), requestProcessorPtr))
        {
            // There is a request processor for this URI
           requestProcessorPtr(requestContext, request, response);
        }
        else if (   (   requestContext.methodIs(HTTP_GET_METHOD)
                     || requestContext.methodIs(HTTP_POST_METHOD)
                     || requestContext.methodIs(HTTP_PUT_METHOD)
                     || requestContext.methodIs(HTTP_DELETE_METHOD)
                     )
                 && findHttpService(uriFileName.data(), pService))
        {
           pService->processRequest(requestContext, request, response);
        }
        else
        {
           processNotSupportedRequest(requestContext, request, response);
        }
    }
}


void HttpServer::processNotSupportedRequest(const HttpRequestContext& requestContext,
                                            const HttpMessage& request,
                                            HttpMessage*& response)
{
    // Method not supported
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_UNSUPPORTED_METHOD_CODE,
        HTTP_UNSUPPORTED_METHOD_TEXT);
    const char* text = "<HTML><BODY>Not Implemented</BODY></HTML>\n";
    HttpBody* body = new HttpBody(text, -1,
        CONTENT_TYPE_TEXT_HTML);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
            response->setContentLength(strlen(text));
}

void HttpServer::processFileNotFound(const HttpRequestContext& requestContext,
                                     const HttpMessage& request,
                                     HttpMessage*& response)
{
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        HTTP_FILE_NOT_FOUND_CODE,
        HTTP_FILE_NOT_FOUND_TEXT);
    const char* text = "<HTML><BODY>File Not Found</BODY></HTML>\n";
    HttpBody* body = new HttpBody(text, -1 ,
        CONTENT_TYPE_TEXT_HTML);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(strlen(text));
}


void HttpServer::testCgiRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response)
{
    UtlString url;
    UtlString value;
    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_RAW_URL,
        url);
    UtlString cgiDump("<HTML>\n<TITLE>\n");
    cgiDump.append(url);
    cgiDump.append(" dump\n</TITLE>\n<BODY>\n<H3>Environment Variables\n</H3>\n");
    cgiDump.append("<TABLE BORDER=1>\n<TR>\n<TH ALIGN=LEFT>Name</TH>\n<TH ALIGN=LEFT>Value</TH>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_RAW_URL,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_RAW_URL</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_UNMAPPED_FILE,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_UNMAPPED_FILE</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_MAPPED_FILE,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_MAPPED_FILE</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_QUERY_STRING,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_QUERY_STRING</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_SERVER_NAME,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_SERVER_NAME</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_REQUEST_METHOD,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_REQUEST_METHOD</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n");

    requestContext.getEnvironmentVariable(HttpRequestContext::HTTP_ENV_USER,
        value);
    cgiDump.append("<TR>\n<TD ALIGN=LEFT>HTTP_ENV_USER</TD>\n<TD ALIGN=LEFT>");
    cgiDump.append(value);
    cgiDump.append("</TD>\n</TR>\n</TABLE>\n");


    cgiDump.append("<H3>CGI/Form Variables\n</H3>\n");
    cgiDump.append("<TABLE BORDER=1>\n<TR>\n<TH>Name</TH>\n<TH>Value</TH>\n</TR>\n");

    int index = 0;
    UtlString name;
    while(requestContext.getCgiVariable(index, name, value))
    {
      cgiDump.append("<TR>\n<TD  ALIGN=LEFT>");
        cgiDump.append(name);
        cgiDump.append("</TD>\n<TD ALIGN=LEFT>");
        cgiDump.append(value);
        cgiDump.append("</TD>\n</TR>\n");
        index++;
    }
    cgiDump.append("</TABLE>\n");

    createHtmlResponse(HTTP_OK_CODE, HTTP_OK_TEXT, cgiDump.data(), response);

    url.remove(0);
    value.remove(0);
    cgiDump.remove(0);
    name.remove(0);
}

void HttpServer::createHtmlResponse(int responseCode, const char* responseCodeText,
                   const char* htmlBodyText, HttpMessage*& response)
{
    response = new HttpMessage();
    response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
        responseCode,
        responseCodeText);
    HttpBody* body = new HttpBody(htmlBodyText, -1,
        CONTENT_TYPE_TEXT_HTML);
    response->setBody(body);
    response->setContentType(CONTENT_TYPE_TEXT_HTML);
    response->setContentLength(strlen(htmlBodyText));
}

void HttpServer::addUriMap(const char* fromUri, const char* toUri)
{
   if ( '/' == *fromUri && '/' == *toUri )
   {
      UtlString* fromPath = new UtlString(fromUri);
      UtlString* toPath   = new UtlString(toUri);
      if (mUriMaps.insertKeyAndValue(fromPath, toPath))
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "HttpServer::addUriMap added '%s' to '%s'",
                       fromUri, toUri);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT, "HttpServer::addUriMap conflict with '%s' to '%s'",
                       fromUri, toUri);
         assert(false);
         delete fromPath;
         delete toPath;
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "HttpServer::addUriMap invalid mapping '%s' to '%s'\n"
                    "   both from and to must begin with '/'" ,
                    fromUri, toUri);
      assert(false);
   }
}

void HttpServer::addRequestProcessor(const char* fileUrl,
                                     RequestProcessor* requestProcessor
                                     )
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "HttpServer::addRequestProcessor '%s' to %p",
                 fileUrl, requestProcessor);

   addUriMap( fileUrl, fileUrl );

   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr((void*)requestProcessor);
   mRequestProcessorMethods.insertKeyAndValue(name, value);
}

void HttpServer::addHttpService(const char* fileUrl, HttpService* service)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "HttpServer::addHttpService '%s' to %p",
                 fileUrl, service);

   UtlString* name = new UtlString(fileUrl);
   mHttpServices.insertKeyAndValue(name, service);
}


UtlBoolean HttpServer::findRequestProcessor(const char* fileUri,
                                            RequestProcessor* &requestProcessor
                                            )
{
    UtlString uriCollectable(fileUri);
    UtlVoidPtr* processorCollectable;

    requestProcessor = NULL;
    processorCollectable =
        (UtlVoidPtr*) mRequestProcessorMethods.findValue(&uriCollectable);
    if(processorCollectable)
    {
        requestProcessor = (RequestProcessor*)processorCollectable->getValue();
    }

    return(requestProcessor != NULL);
}

UtlBoolean HttpServer::findHttpService(const char* fileUri, HttpService*& pService)
{
    UtlString path(fileUri);
    pService = NULL;

    while (   !pService
           && !path.isNull()
           && !(pService = dynamic_cast<HttpService*>(mHttpServices.findValue(&path))))
    {
       ssize_t lastSlash = path.last('/');;
       if (lastSlash > 1)
       {
          path.remove(lastSlash); // take one level off (eg from "/xyz/abc" to "/xyz")
       }
       else
       {
          // we are either at "/xyz" or "/"
          if (path.length() > 1)
          {
             path.remove(1); // reduce to just "/"
          }
          else
          {
             // path was "/" and no match found, so empty the string to exit search
             path.remove(0);
          }
       }
    }

    return(pService != NULL);
}

UtlBoolean HttpServer::mapUri(UtlHashMap& uriMaps, const char* uri, UtlString& mappedUri)
{
    UtlBoolean mapFound = FALSE;
    mappedUri.remove(0);

    if(uri)
    {
        UtlString originalUri(uri);
        UtlString mapFromUri(uri);
        UtlString* mapToUri;
        ssize_t dirSeparatorIndex;

        do
        {
            mapToUri = dynamic_cast<UtlString*>(uriMaps.findValue(&mapFromUri));
            if(mapToUri)
            {
                mappedUri.append(mapToUri->data());
                if (mapFromUri.length() < originalUri.length())
                {
                   if(mappedUri.length() == 1)
                   {
                      // this is a mapping from some path to "/", so remove what we just added
                      mappedUri.remove(0);
                   }
                   else if(   mappedUri.data()[mappedUri.length() - 1] != '/'
                           && uri[mapFromUri.length()] != '/'
                           )
                   {
                      // Need a directory separator
                      mappedUri.append('/');
                   }
                   mappedUri.append(originalUri, mapFromUri.length(),
                                    UtlString::UTLSTRING_TO_END);
                }
                else
                {
                   mappedUri.append(originalUri, mapFromUri.length(),
                                    UtlString::UTLSTRING_TO_END);
                }
                mapFound = TRUE;
            }
            else
            {
               dirSeparatorIndex = mapFromUri.last('/');
               if(dirSeparatorIndex == 0 && mapFromUri.length() > 1)
               {
                  // we're down to "/string" - remove the "string"
                  mapFromUri.remove(1);
               }
               else if(dirSeparatorIndex >= 0)
               {
                  // remove the topmost path and the separator
                  mapFromUri.remove(dirSeparatorIndex);
               }
               else
               {
                  // no separator found - empty the from string to exit the loop
                  mapFromUri.remove(0);
               }
            }
        } while(!mapFound && !mapFromUri.isNull());

        if (!mapFound)
        {
           mappedUri.append(uri);
        }
    }

    OsSysLog::add(FAC_SIP,
                  mapFound ? PRI_INFO : PRI_DEBUG,
                  "HttpServer::mapUri %s '%s' -> '%s'",
                  mapFound ? "mapped" : "no mapping",
                  uri, mappedUri.data());

    return(mapFound);
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
