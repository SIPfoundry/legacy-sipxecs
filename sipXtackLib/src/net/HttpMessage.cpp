//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <string.h>
#include <ctype.h>
#ifdef _VXWORKS
#include <resparse/vxw/hd_string.h>
#endif

#ifdef __pingtel_on_posix__  //needed by linux
#include <wctype.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "utl/UtlDListIterator.h"

// APPLICATION INCLUDES
#include <net/HttpMessage.h>
#include <net/NameValuePair.h>
// Needed for SIP_SHORT_CONTENT_LENGTH_FIELD.
#include <net/SipMessage.h>

#include <net/NameValueTokenizer.h>
#include <net/NetAttributeTokenizer.h>
#include <os/OsDateTime.h>
#include <os/OsUtil.h>
#include <os/OsConnectionSocket.h>
#include <utl/UtlVoidPtr.h>
#ifdef HAVE_SSL
#include <os/OsSSLConnectionSocket.h>
#endif /* HAVE_SSL */
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <net/NetBase64Codec.h>
#include <net/NetMd5Codec.h>
#include <net/HttpConnectionMap.h>

#include <net/Url.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HTTP_READ_TIMEOUT_MSECS  30000
#define MAX_UDP_MESSAGE 65536

// :TODO: need this to be cleaned up - there are at least three controls here
#undef MSG_DEBUG
#undef TEST_PRINT
#undef TEST

// STATIC VARIABLE INITIALIZATIONS
int HttpMessage::smHttpMessageCount = 0;

// LOCAL MACROS
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifdef _VXWORKS
#define iswspace(a) ((((a) >= 0x09) && ((a) <= 0x0D)) || ((a) == 0x20))
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpMessage::HttpMessage(const char* messageBytes, int byteCount)
{
    smHttpMessageCount++;

    mHeaderCacheClean = FALSE;

        //nameValues = new UtlHashBag(100);
        body = NULL;
        transportTimeStamp = 0;
        lastResendDuration = 0;
        timesSent = 0;
        transportProtocol = OsSocket::UNKNOWN;
    mFirstSent = FALSE;
    mSendPort = PORT_NONE;
    mpResponseListenerQueue = NULL;
    mResponseListenerData = NULL;
#ifdef HTTP_TIMELOG
    mTimeLog.addEvent("CREATED");
#endif


        parseMessage(messageBytes, byteCount);
}

HttpMessage::HttpMessage(OsSocket* inSocket, int bufferSize)
{
    smHttpMessageCount++;

    mHeaderCacheClean = FALSE;

        //mNameValues = new UtlHashBag(100);
        body = NULL;
        transportTimeStamp = 0;
        lastResendDuration = 0;
        timesSent = 0;
        transportProtocol = OsSocket::UNKNOWN;
    mFirstSent = FALSE;
    mSendPort = PORT_NONE;
    mpResponseListenerQueue = NULL;
    mResponseListenerData = NULL;
#ifdef HTTP_TIMELOG
    mTimeLog.addEvent("READ FROM SOCKET");
#endif

        read(inSocket, bufferSize);
}


// Copy constructor
HttpMessage::HttpMessage(const HttpMessage& rHttpMessage)
{
    smHttpMessageCount++;

    mHeaderCacheClean = rHttpMessage.mHeaderCacheClean;
    mFirstHeaderLine = rHttpMessage.mFirstHeaderLine;
    body = NULL;
    if(rHttpMessage.body)
    {
        body = rHttpMessage.body->copy();
    }
    //nameValues = new UtlHashBag(100);
    transportTimeStamp = rHttpMessage.transportTimeStamp;
    lastResendDuration = rHttpMessage.lastResendDuration;
    transportProtocol = rHttpMessage.transportProtocol;
    timesSent = rHttpMessage.timesSent;
    mFirstSent = rHttpMessage.mFirstSent;
    mSendPort = rHttpMessage.mSendPort;
    mpResponseListenerQueue = rHttpMessage.mpResponseListenerQueue;
    mResponseListenerData = rHttpMessage.mResponseListenerData;

    NameValuePair* headerField;
    NameValuePair* copiedHeader = NULL;
    UtlDListIterator iterator((UtlDList&)rHttpMessage.mNameValues);
    while((headerField = (NameValuePair*) iterator()))
    {
        copiedHeader = new NameValuePair(*headerField);
        mNameValues.append(copiedHeader);
    }

#ifdef HTTP_TIMELOG
    mTimeLog = rHttpMessage.mTimeLog;
#endif
    mSendAddress = rHttpMessage.mSendAddress;
    mSendPort = rHttpMessage.mSendPort;
}

// Destructor
HttpMessage::~HttpMessage()
{
    smHttpMessageCount--;
    NameValuePair* headerField = NULL;

    mHeaderCacheClean = FALSE;

    // For each name value:
    while((headerField = (NameValuePair*) mNameValues.get()))
    {
        delete headerField;
        headerField = NULL;
    }

    if(body)
    {
            delete body;
            body = 0;
    }
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
HttpMessage&
HttpMessage::operator=(const HttpMessage& rHttpMessage)
{
   if (this == &rHttpMessage)            // handle the assignment to self case
      return *this;
   else
   {
       smHttpMessageCount--;
       mHeaderCacheClean = rHttpMessage.mHeaderCacheClean;
       mFirstHeaderLine = rHttpMessage.mFirstHeaderLine;
           //nameValues.destroyAll();
       // Get rid of any headers which exist in this message
       NameValuePair* headerField = NULL;
       while((headerField = (NameValuePair*) mNameValues.get()))
       {
           delete headerField;
           headerField = NULL;
       }

       if(body)
       {
           delete body;
           body = NULL;
       }

       if(rHttpMessage.body)
       {
           body = rHttpMessage.body->copy();
       }

      //use copy constructor to copy values
       smHttpMessageCount++;
       transportTimeStamp = rHttpMessage.transportTimeStamp;
       lastResendDuration = rHttpMessage.lastResendDuration;
       transportProtocol = rHttpMessage.transportProtocol;
       timesSent = rHttpMessage.timesSent;
       mFirstSent = rHttpMessage.mFirstSent;
       mSendPort = rHttpMessage.mSendPort;
       mpResponseListenerQueue = rHttpMessage.mpResponseListenerQueue;
       mResponseListenerData = rHttpMessage.mResponseListenerData;

       NameValuePair* copiedHeader = NULL;
       UtlDListIterator iterator((UtlDList&)rHttpMessage.mNameValues);
       while((headerField = (NameValuePair*) iterator()))
       {
           copiedHeader = new NameValuePair(*headerField);
           mNameValues.append(copiedHeader);
       }

#ifdef HTTP_TIMELOG
       mTimeLog = rHttpMessage.mTimeLog;
#endif
       mSendAddress = rHttpMessage.mSendAddress;
       mSendPort = rHttpMessage.mSendPort;
   }

   return *this;
}

/* void HttpMessage::convertToPlatformPath(const char* uri,
                                        UtlString& platformFilePath)
{
    UtlString uriString(uri);

#ifdef _WIN32

    int pathSeparatorIndex;
    while((pathSeparatorIndex = uriString.first('/')) >= 0)
    {
        uriString.replace(pathSeparatorIndex, 1, "\\");
    }

#endif
    platformFilePath = uriString;
}*/

int HttpMessage::parseFirstLine(const char* messageBytesPtr, int byteCount)
{
   mHeaderCacheClean = FALSE;
   mFirstHeaderLine = OsUtil::NULL_OS_STRING;
   int bytesConsumed = 0;

    // Read the first header line
   int nextLineOffset;
        int headerLineLength =
      NameValueTokenizer::findNextLineTerminator(messageBytesPtr,
                                                                                        byteCount,
                                                                                        &nextLineOffset);

        if(headerLineLength < 0)
        {
                headerLineLength = byteCount;
        }

        if(headerLineLength > 0)
        {
                mFirstHeaderLine.append(messageBytesPtr, headerLineLength);

                if(nextLineOffset > 0)
                {
                        bytesConsumed += nextLineOffset;
                }
                else
                {
                        bytesConsumed = byteCount;
      }
   }

   return(bytesConsumed);
}

// Parse message out of byte bucket
// Note: this should be broken up into smaller methods as
// the need arrises for it atomic functionality
void HttpMessage::parseMessage(const char* messageBytes, int byteCount)
{
    mHeaderCacheClean = FALSE;

    if(byteCount <= 0)
    {
       if(messageBytes)
       {
                    byteCount = strlen(messageBytes);
       }
       else
       {
                    byteCount = 0;
                        mFirstHeaderLine = OsUtil::NULL_OS_STRING;
            if(body) delete body;
                    body = NULL;
       }
    }

        if(byteCount > 0)
        {
                int bytesConsumed = 0;
                const char* messageBytesPtr = messageBytes;

                // Read the first header line
        bytesConsumed = parseFirstLine(messageBytes, byteCount);

        // Parse the headers out and add them to the list
                bytesConsumed += parseHeaders(messageBytes + bytesConsumed, byteCount - bytesConsumed,
            mNameValues);

                // Create the body if there is stuff left
                if(byteCount > bytesConsumed)
                {
                        messageBytesPtr = messageBytes + bytesConsumed;

                        if(body)
                        {
                                delete body;
                        }

            // Construct the body from the remaining bytes
            parseBody(messageBytesPtr, byteCount - bytesConsumed);

        }
        }
}

void HttpMessage::parseBody(const char* messageBytesPtr, int bodyLength)
{
    if (bodyLength <= 1 && 
        messageBytesPtr && 
        (messageBytesPtr[0] == '\n' || 
         messageBytesPtr[0] == '\r'))
    {
        // do nothing
    }

    // Need to use a body factory
    const char* contentType = getHeaderValue(0, HTTP_CONTENT_TYPE_FIELD);
    if (contentType == NULL)
    {
        contentType = getHeaderValue(0, "C");
    }

    const char* contentEncodingString = 
            getHeaderValue(0, HTTP_CONTENT_TRANSFER_ENCODING_FIELD);
    if (contentEncodingString == NULL)
    {
        contentEncodingString = getHeaderValue(0, "E");
    }

    body = HttpBody::createBody(messageBytesPtr,
                                bodyLength,
                                contentType,
                                contentEncodingString);
}

int HttpMessage::findHeaderEnd(const char* headerBytes, int messageLength)
{
    int lineLength = 0;
    int nextLineIndex = 0;
    int bytesConsumed = 0;
    while(messageLength - bytesConsumed > 0 &&
          (lineLength =
            NameValueTokenizer::findNextLineTerminator(&headerBytes[bytesConsumed],
                messageLength - bytesConsumed, &nextLineIndex)))
    {
            if(nextLineIndex > 0)
            {
                    bytesConsumed += nextLineIndex;
            }
            else
            {
            if(lineLength < 0)
            {
                bytesConsumed += messageLength - bytesConsumed;
            }
            else
            {
                    bytesConsumed += lineLength;
            }
            }
    }

    // If we found a blank line:
    if(nextLineIndex == 1 && (headerBytes[bytesConsumed] == NEWLINE ||
        headerBytes[bytesConsumed] == CARRIAGE_RETURN))
    {
        bytesConsumed++;
    }
    else if(nextLineIndex == 2 && (headerBytes[bytesConsumed] == NEWLINE ||
        headerBytes[bytesConsumed] == CARRIAGE_RETURN) &&
        ((headerBytes[bytesConsumed + 1] == NEWLINE ||
        headerBytes[bytesConsumed + 1] == CARRIAGE_RETURN)))
    {
        bytesConsumed += 2;
    }

    // If we did not find a terminator, there is no explicit end to the
    // headers
    else
        bytesConsumed = -1;

    return(bytesConsumed);
}

int HttpMessage::parseHeaders(const char* headerBytes, int messageLength,
                              UtlDList& headerNameValues)
{
        UtlString name;
        UtlString value;
        char nameFirstChar;
        NameValuePair* headerField = NULL;
        NameValuePair* previousHeaderField = NULL;
        NameValueTokenizer parser(headerBytes, messageLength);
        int nameFound;

        // If this is a zero length line the rest is the body
        do
        {
                nameFound = parser.getNextPair(HTTP_NAME_VALUE_DELIMITER,
                        &name, & value);
                if(nameFound)
                {
                        // If this is a line continuation
                        nameFirstChar = name(0);
                        if(previousHeaderField != NULL &&
                                (nameFirstChar == ' ' || nameFirstChar == '\t'))
                        {
                                // Re-join the name and value if there is anything
                                // in value
                                if(!value.isNull())
                                {
                                        name.append(HTTP_NAME_VALUE_DELIMITER);
                                        name.append(value.data());
                                }

                                // Append this to the previous headers's value
                                name.insert(0, previousHeaderField->getValue());
                                previousHeaderField->setValue(name.data());
                        }

                        // Create a new name value pair for the header line
                        else
                        {
                                name.toUpper();

                                // Remove trailing white space
                                NameValueTokenizer::backTrim(&name, " \t");

                                headerField = new NameValuePair(name.data(), value.data());
                                headerNameValues.append(headerField);
                                previousHeaderField = headerField;
                        }
                }
                //name = OsUtil::NULL_OS_STRING;
                //value = OsUtil::NULL_OS_STRING;
        }
        while(nameFound);

        //name = OsUtil::NULL_OS_STRING;
        //value= OsUtil::NULL_OS_STRING;

        return(parser.getProcessedIndex());
}

int HttpMessage::get(Url& httpUrl,
                     int maxWaitMilliSeconds,
                     bool bPersistent)
{
    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get(2) httpUrl = '%s'",
                  httpUrl.toString().data());

    HttpMessage request;
    UtlString uriString;
    
    httpUrl.getPath(uriString, TRUE);
    
    request.setRequestFirstHeaderLine(HTTP_GET_METHOD,
                                      uriString,
                                      HTTP_PROTOCOL_VERSION);
    return(get(httpUrl, request, maxWaitMilliSeconds, bPersistent));
}

OsStatus HttpMessage::get(Url& httpUrl,
                          int iMaxWaitMilliSeconds,
                          GetDataCallbackProc pCallbackProc,
                          void* pOptionalData,
                          OsConnectionSocket** socket)
{
   OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get(5) httpUrl = '%s'",
                 httpUrl.toString().data());
   OsStatus rc = OS_SUCCESS ;
   UtlString uriString;
   httpUrl.getPath(uriString, TRUE); // Put CGI variable in PATH as this is GET

   // Construnct a request
   HttpMessage request;
   request.setRequestFirstHeaderLine(HTTP_GET_METHOD,
                                     uriString,
                                     HTTP_PROTOCOL_VERSION);

   // Construct the socket to send request and get response
   int httpPort;
   UtlString httpHost;
   httpUrl.getHostAddress(httpHost);
   UtlString hostPort(httpHost);
   httpPort = httpUrl.getHostPort();

   UtlString urlType;
   httpUrl.getUrlType(urlType);

   if (!portIsValid(httpPort))
   {
      if (httpUrl.getScheme() == Url::HttpsUrlScheme)
         httpPort = 443;
      else
         httpPort = 80;

      hostPort.append(":");
      char tmpportbuf[10];
      sprintf(tmpportbuf,"%d",httpPort);
      hostPort += tmpportbuf;
   }

   request.addHeaderField("Host", hostPort.data());
   request.addHeaderField("Accept", "*/*");
   OsConnectionSocket *httpSocket = NULL;

   int tries = 0;
   int connected = 0;
   int exp = 1;
   while (tries++ < HttpMessageRetries)
   {
      if (httpUrl.getScheme() == Url::HttpsUrlScheme)
      {
#ifdef HAVE_SSL
         httpSocket = (OsConnectionSocket *)new OsSSLConnectionSocket(httpPort, httpHost, iMaxWaitMilliSeconds/1000);
#else /* ! HAVE_SSL */
         // SSL is not configured in, so we cannot do https: gets.
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "HttpMessage::get(Url&,int,...) SSL not configured; "
                       "cannot get URL '%s'", httpUrl.toString().data());
         httpSocket = NULL;
#endif /* HAVE_SSL */
      }
      else
      {
         httpSocket = new OsConnectionSocket(httpPort, httpHost);
      }
      if (httpSocket)
      {
         connected = httpSocket->isConnected();
         if (!connected)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "HttpMessage::get socket connection to %s:%d failed, try again %d ...",
                          httpHost.data(), httpPort, tries);
            delete httpSocket;
            httpSocket = 0;
            OsTask::delay(20*exp);
            exp = exp*2;
         }
         else
         {
            break;
         }
      }
   }

   if (!connected)
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "HttpMessage::get socket connection to %s:%d failed, give up...",
                    httpHost.data(), httpPort);
      return OS_FAILED;
   }

   if (socket != NULL)
   {
      *socket = httpSocket ;
   }

    // Send the request
    int bytesSent = 0;
    if (httpSocket->isReadyToWrite(iMaxWaitMilliSeconds))
        bytesSent = request.write(httpSocket);

   // Handle the response
   if(bytesSent > 0 && httpSocket->isReadyToRead(iMaxWaitMilliSeconds))
   {
      UtlString buffer ;

      int iRead = readHeader(httpSocket, buffer) ;
      if (iRead > 0)
      {
         mHeaderCacheClean = FALSE;
         int iHeaderLength = parseFirstLine(buffer.data(), iRead) ;
         parseHeaders(&buffer.data()[iHeaderLength], iRead-iHeaderLength, mNameValues) ;

         int iContentLength = getContentLength() ;
         if (iContentLength > 0)
            iRead = readBody(httpSocket, iContentLength, pCallbackProc, pOptionalData) ;
      }
   }
   else
      rc = OS_NO_MORE_DATA ;

   if (socket == NULL)
   {
      delete httpSocket ;
   }

   return rc ;
}

int HttpMessage::get(Url& httpUrl,
                     HttpMessage& request,
                     int maxWaitMilliSeconds,
                     bool bPersistent)
{
    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get(3) httpUrl = '%s'",
                  httpUrl.toString().data());
    HttpConnectionMap *pConnectionMap = NULL;
    HttpConnectionMapEntry* pConnectionMapEntry = NULL;
    UtlString uriString;
    httpUrl.getPath(uriString,
                    TRUE); // Put CGI variable in PATH as this is GET

    // Construct the socket to send request and get response
    int httpPort;
    httpPort = httpUrl.getHostPort();

    UtlString httpHost;
    httpUrl.getHostAddress(httpHost);

    UtlString urlType;
    httpUrl.getUrlType(urlType);
    
    // Construct the key for the persistent connection mapping
    UtlString key;    
    if (bPersistent)
    { 

    }
 
    // preserve these fields if they are already set
    if (request.getHeaderValue(0, HTTP_HOST_FIELD) == NULL)
    {
        UtlString hostPort(httpHost);
        httpPort = httpUrl.getHostPort();
        if (httpPort == PORT_NONE)
        {
            if (httpUrl.getScheme() == Url::HttpUrlScheme)
                httpPort = 80;
            else
                httpPort = 443;

            hostPort.append(":");
            char tmpportbuf[10];
            sprintf(tmpportbuf,"%d",httpPort);
            hostPort += tmpportbuf;
        }
        request.addHeaderField(HTTP_HOST_FIELD, hostPort.data());
    }

    if ( request.getHeaderValue(0, HTTP_ACCEPT_FIELD) == NULL)
    {
        request.addHeaderField(HTTP_ACCEPT_FIELD, "*/*");
    }

    OsConnectionSocket *httpSocket = NULL;
    int connected = 0;    
    int httpStatus = -1;
   
    int bytesRead = 0;
    int bytesSent = 0;    
    int sendTries = 0;
    // Set connection header to keep-alive, get a map entry for the URI with the asscoiated socket.
    // If there is no existing map entry, one will be created with the httpSocket set to NULL.
    if (bPersistent)
    {
        pConnectionMap = HttpConnectionMap::getHttpConnectionMap();
                
        request.setHeaderValue(HTTP_CONNECTION_FIELD, "Keep-Alive");         
        pConnectionMapEntry = pConnectionMap->getPersistentConnection(httpUrl, httpSocket);
    }            
    // Try to send request at least once, on persistent connections retry once if it fails.
    // Retry on persistent connections because the getPersistentConnection call may return
    // a non-NULL socket, assuming the connection is persistent when the other side is not.
    while (sendTries < HttpMessageRetries && bytesRead == 0)
    {        
        if (httpSocket == NULL)
        {
            int tries = 0;
    
            int exp = 1;
            while (tries++ < HttpMessageRetries)
            {
               if (urlType == "https")
               {
        #ifdef HAVE_SSL
                  httpSocket = (OsConnectionSocket *)new OsSSLConnectionSocket(httpPort, httpHost, maxWaitMilliSeconds/1000);
        #else /* ! HAVE_SSL */
                  // SSL is not configured in, so we cannot do https: gets.
                  OsSysLog::add(FAC_SIP, PRI_CRIT,
                                "HttpMessage::get(Url&,HttpMessage&,int) SSL not configured; "
                                "cannot get URL '%s'", httpUrl.toString().data());
                  httpSocket = NULL;
        #endif /* HAVE_SSL */
               }
               else
               {
                  httpSocket = new OsConnectionSocket(httpPort, httpHost);
               }
               if (httpSocket)
               {
                  connected = httpSocket->isConnected();
                  if (!connected)
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR,
                                   "HttpMessage::get socket connection to %s:%d failed, try again %d ...\n",
                                   httpHost.data(), httpPort, tries);
                     delete httpSocket;
                     httpSocket = 0;
                     OsTask::delay(20*exp);
                     exp = exp*2;
                  }
                  else
                  {
                     break;
                  }
               }
            }
            // If we created a new connection and are persistent then remember the socket in the map
            // and mark connection as being used
            if (pConnectionMapEntry)
            {
                pConnectionMapEntry->mpSocket = httpSocket;
                pConnectionMapEntry->mbInUse = true;
            }
        }
        else
        {
            connected = httpSocket->isConnected();          
        }
        if (!connected)
        {
           OsSysLog::add(FAC_SIP, PRI_ERR,
                         "HttpMessage::get socket connection to %s:%d failed, give up...\n",
                         httpHost.data(), httpPort);
           if (pConnectionMap)
           {
               // Release lock on persistent connection
               pConnectionMapEntry->mLock.release();             
           }              
           return httpStatus;
        }
      
        // Send the request - most of the time returns 1 for some reason, 0 indicates problem
        if (httpSocket->isReadyToWrite(maxWaitMilliSeconds))
        {
            bytesSent = request.write(httpSocket);
        }

        if (bytesSent == 0)            
        {
            if (pConnectionMap)
            {
                // No bytes were sent .. if this is a persistent connection and it failed on retry
                // mark it unused in the connection map. Set socket to NULL
                if (sendTries == HttpMessageRetries-1)
                { 
                    pConnectionMapEntry->mbInUse = false;
                }
                // Close socket to avoid timeouts in subsequent calls
                httpSocket->close();
                delete httpSocket;
                pConnectionMapEntry->mpSocket = NULL;
                httpSocket = NULL;
                OsSysLog::add(FAC_HTTP, PRI_DEBUG, 
                              "HttpMessage::get Sending failed sending on persistent connection on try %d",
                              sendTries);      
            }
        }
        else if(   bytesSent > 0
                && httpSocket->isReadyToRead(maxWaitMilliSeconds))
        {
            bytesRead = read(httpSocket);

            // Close a non-persistent connection
            if (pConnectionMap == NULL)
            {
                httpSocket->close();
            }
            else
            {
                if (bytesRead == 0)
                {
                    // No bytes were read .. if this is a persistent connection
                    // and it failed on retry mark it unused  
                    // in the connection map. Set socket to NULL
                    if (sendTries == HttpMessageRetries-1)                    
                    {
                        pConnectionMapEntry->mbInUse = false;
                    }
                    httpSocket->close();
                    delete httpSocket;
                    pConnectionMapEntry->mpSocket = NULL;
                    httpSocket = NULL;
                    OsSysLog::add(FAC_HTTP, PRI_DEBUG, 
                                  "HttpMessage::get Receiving failed on persistent connection on try %d",
                                  sendTries);
                }
                else
                {
                    // On success don't retry for persistent connection, set sendTries 
                    // to mamximum to break out of loop
                    sendTries = HttpMessageRetries;
                }                
            }
        }
        ++sendTries;
    }
    if (pConnectionMapEntry)
    {        
        // Release lock on persistent connection
        pConnectionMapEntry->mLock.release(); 
    }    
    if(bytesRead > 0)
    {
        httpStatus = getResponseStatusCode();
        int authEntity = SERVER;

        if(httpStatus == HTTP_UNAUTHORIZED_CODE)
        {
            authEntity = SERVER;
        }

        else if(httpStatus == HTTP_PROXY_UNAUTHORIZED_CODE)
        {
            authEntity = PROXY;
        }
        UtlString authScheme;
        getAuthenticationScheme(&authScheme, authEntity);

        if(authScheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
        {
            // if we have a password add the credentials to the
            // request
            UtlString user;
            UtlString password;
            httpUrl.getUserId(user);
            httpUrl.getPassword(password);
            if(! user.isNull())
            {
                request.setBasicAuthorization(user, password, authEntity);

                // Construct a new socket
                OsConnectionSocket *httpAuthSocket = NULL;

                int httpStatus = -1;
                int tries = 0;
                int connected = 0;
                int exp = 1;
                while (tries++ < 6)
                {
                   if (urlType == "https")
                   {
#ifdef HAVE_SSL
                      httpAuthSocket = (OsConnectionSocket *)new OsSSLConnectionSocket(httpPort, httpHost, maxWaitMilliSeconds/1000);
#else /* ! HAVE_SSL */
                      // SSL is not configured in, so we cannot do https: gets.
                      OsSysLog::add(FAC_SIP, PRI_CRIT,
                                    "HttpMessage::get(Url&,HttpMessage&,int) SSL not configured; "
                                    "cannot get URL '%s'", httpUrl.toString().data());
                      httpAuthSocket = NULL;
#endif /* HAVE_SSL */
                   }
                   else
                      httpAuthSocket = new OsConnectionSocket(httpPort, httpHost);
                   if (httpAuthSocket)
                   {
                      connected = httpAuthSocket->isConnected();
                      if (!connected)
                      {
                         OsSysLog::add(FAC_SIP, PRI_ERR,
                                       "HttpMessage::get socket connection to %s:%d failed, try again %d ...\n",
                                       httpHost.data(), httpPort, tries);
                         delete httpAuthSocket;
                         httpAuthSocket = 0;
                         OsTask::delay(20*exp);
                         exp = exp*2;
                      }
                      else
                      {
                         break;
                      }
                   }
                }

                if (!connected)
                {
                   OsSysLog::add(FAC_SIP, PRI_ERR,
                                 "HttpMessage::get socket connection to %s:%d failed, give up...\n",
                                 httpHost.data(), httpPort);
                   return httpStatus;
                }

                // Sent the request again
                if (httpAuthSocket->isReadyToWrite(maxWaitMilliSeconds))
                  bytesSent = request.write(httpAuthSocket);
                bytesRead = 0;

                // Clear out the data in the previous response
                mHeaderCacheClean = FALSE;
                mNameValues.destroyAll();
                    if(body)
                    {
                            delete body;
                            body = 0;
                    }

                // Wait for the response
                if(bytesSent > 0 &&
                    httpAuthSocket->isReadyToRead(maxWaitMilliSeconds))
                {
                    bytesRead = read(httpAuthSocket);
                    httpAuthSocket->close();
                }

                // Get the response
                if(bytesRead > 0)
                {
                    httpStatus = getResponseStatusCode();
                }

                if (httpAuthSocket)
                    delete httpAuthSocket;

            } // end if auth. retry

        } // End if Basic Auth.

    }  // End if first response was returned

    if (httpSocket && !bPersistent)
    {
        delete httpSocket;
        httpSocket = 0;
    }
    return(httpStatus);
}

int HttpMessage::readHeader(OsSocket* inSocket, UtlString& buffer)
{
   char      ch ;
   int       iBytesRead = 0 ;
   int       iRead;
   OsSocket::IpProtocolSocketType socketType = inSocket->getIpProtocol();
   UtlString  remoteHost;
        int       remotePort;
   UtlBoolean bLastWasCr = FALSE;
   UtlBoolean bIsCrLfFormat = FALSE ;

   setSendProtocol(socketType);

   // If there are no residual bytes in the buffer
   if (inSocket->isOk() &&
       ((socketType != OsSocket::TCP && socketType != OsSocket::SSL_SOCKET) ||
        inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS)))
   {
      while (inSocket->isOk() &&
             inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS))
      {
         iRead = inSocket->read(&ch, 1, &remoteHost, &remotePort);
         if (iRead == 1)
         {
            buffer.append(ch) ;
            iBytesRead++ ;

            if ((bLastWasCr) && (ch == CARRIAGE_RETURN))
            {
               // Read Lf is we are in crlf format
               if (bIsCrLfFormat)
               {
                  iRead = inSocket->read(&ch, 1, &remoteHost, &remotePort);
               }

               // Time to quit, we have hit the end of the header
               break ;
            }
            else if ((bLastWasCr) && (ch == NEWLINE))
            {
               bIsCrLfFormat = TRUE ;
            }
            else
            {
               if (ch == CARRIAGE_RETURN)
                  bLastWasCr = TRUE ;
               if (!iswspace(ch))
                  bLastWasCr = FALSE ;
            }
         }
         else
            break ;
      }
   }


        return(iBytesRead);
}


int HttpMessage::readBody(OsSocket* inSocket, int iLength, GetDataCallbackProc pCallbackProc, void* pOptionalData)
{
   char         buffer[4096] ;
   int          iBytesRead = 0 ;
   unsigned int iRead;
   OsSocket::IpProtocolSocketType socketType = inSocket->getIpProtocol();
   UtlString    remoteHost;
   int          remotePort;


   // If there are no residual bytes in the buffer
   if (inSocket->isOk() && ((socketType != OsSocket::TCP && socketType != OsSocket::SSL_SOCKET ) ||
            inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS)))
   {
      while (inSocket->isOk() && inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS) &&
            (iBytesRead < iLength))
      {
         int iMaxRead = MIN(sizeof(buffer), (unsigned int) (iLength - iBytesRead)) ;
         iRead = inSocket->read(buffer, iMaxRead, &remoteHost, &remotePort);
         if (iRead > 0)
         {
            iBytesRead += iRead ;

            UtlBoolean bRC = (*pCallbackProc)(buffer, (unsigned long) iRead, pOptionalData, this) ;
            if (bRC == FALSE)
            {
               break ;
            }
         }
         else
            break ;
      }
   }

    // Signal callback proc that data transfer is complete
    (*pCallbackProc)(NULL, -1, pOptionalData, this) ;

        return(iBytesRead);
}




int HttpMessage::read(OsSocket* inSocket, int bufferSize,
                      UtlString* externalBuffer,
                      int maxContentLength)
{
   //
   // Initialize state variables that keep track of what we have seen
   // of the incoming HTTP message.
   //

   mHeaderCacheClean = FALSE;
   // Remember to empty the list of parsed header values, as we will use it
   // to parse the headers on the HTTP response we are going to read.
   mNameValues.destroyAll();

   //the following code if enabled will test the effect of messages coming in a
   //fragmented way.  This should NOT be enabled in a released build as
   //performance will be severely affected.
   #ifdef FRAGMENTATION_TEST
   #ifdef _WIN32
      #pragma message("!!!!!!!!!!  WARNING: Buffer size set to 1.  DO NOT build release builds this way. Very SLOW! Please change this in: " __FILE__)
   #else
      #warning !!!!!!!!!!! Buffer size set to 1.  DO NOT build release builds this way. Very SLOW   !!!!!!!!!!!!!!!!!!
   #endif

      bufferSize = 1;
   #endif

   // allBytes is the buffer we will be using to assemble the message.
   // If externalBuffer is supplied, use that, otherwise use localBuffer.
   UtlString localBuffer;
   UtlString* allBytes =
      externalBuffer != NULL ? externalBuffer : &localBuffer;
   int returnMessageLength = 0;

   // Attempt to minimize the number of times that the string gets
   // re-allocated and copied by setting the initial capacity to the
   // requested approximate message size, bufferSize.
   int byteCapacity = allBytes->capacity(bufferSize);
   if (byteCapacity >= bufferSize)
   {
      // Reallocating allBytes was successful.

      // Get information about the socket.
      UtlString remoteHost;
      int remotePort;
      inSocket->getRemoteHostIp(&remoteHost, &remotePort);
      OsSocket::IpProtocolSocketType socketType = inSocket->getIpProtocol();
      setSendProtocol(socketType);

#ifdef TEST
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "HttpMessage::read socket: %p "
                    "getIpProtocol: %d remoteHost: %s remotePort: %d\n",
                    inSocket, socketType,
                    remoteHost.data(), remotePort);
#endif

      // Initialize control variables.
      char* buffer = new char[bufferSize];
      int bytesTotal = allBytes->length();
      int bytesRead = 0;
      // The byte offset of the end of the header.  -1 means the end
      // has not yet been seen.
      int headerEnd = -1;
      // The length of the content.  -1 means the end is not yet known.
      int contentLength = -1;
      UtlBoolean contentLengthSet = FALSE;
      // Assume content-type is set until we read all of the headers.
      UtlBoolean contentTypeSet = TRUE;
      UtlString contentType;

      //
      // Read the HTTP message.
      //

      // If there are no residual bytes in the buffer, read from the socket.
      // :TODO:  This is probably not quite right.  I suspect that its
      // main effect is to have the first read from the socket use 4
      // arguments (to get the remote host/port), whereas the read at
      // the bottom of the do-while uses 2 arguments (because it
      // usually does later reads, and don't need to get the remote
      // ost/port again).  But this scheme doesn't work so well, as
      // there is a third fetch of remote host/port inside the body of
      // the do-while.
      if (bytesTotal <= 0 &&
          inSocket->isOk() &&
          (OsSocket::isFramed(socketType) ||
           inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS)))
      {
         bytesRead = inSocket->read(buffer, bufferSize,
                                    &remoteHost, &remotePort);

         setSendAddress(remoteHost.data(), remotePort);
      }

      do
      {
         if (bytesRead > 0)
         {
            if (allBytes->append(buffer, bytesRead).length() > 0)
            {
               bytesTotal += bytesRead;
            }
            else
            {
               osPrintf("ERROR: Error appending data. possible out of memory error.\n");
               bytesRead = 0;
               break;
            }
         }

         //osPrintf("HttpMessage::read buffer capacity: %d buff addr: %X\n", allBytes.capacity(), allBytes.data());

         // If we have not yet found the end of the headers
         if (headerEnd < 0)
         {
            headerEnd = findHeaderEnd(allBytes->data(), bytesTotal);

            // UDP and Multicast UDP you can only do one read
            // The fragmentation is handled at the socket layer
            // If we did not get it all we are not going to get any more
            if (OsSocket::isFramed(socketType) && headerEnd <= 0)
            {
               headerEnd = bytesTotal;
            }

            // We found the end of all the headers.  Parse them.
            if (headerEnd > 0)
            {
               // Parse the first line
               int endOfFirstLine = parseFirstLine(allBytes->data(),
                                                   headerEnd);
               // Parse all of the headers
               parseHeaders(&(allBytes->data()[endOfFirstLine]),
                            headerEnd - endOfFirstLine,
                            mNameValues);

#ifdef TEST
               // Print out the header values as we have extracted them.
               if (OsSysLog::getLoggingPriority() <= PRI_DEBUG)
               {
                  UtlDListIterator iterator((UtlDList&) mNameValues);
                  NameValuePair* headerField;

#                 ifdef MSG_DEBUG
                  // :TODO: Commented out for the time being to reduce
                  // the volume of debug messages.  We really need
                  // finer-grained control over debugging messages
                  // than we have now.
                  while ((headerField = (NameValuePair*) iterator()) != NULL) 
                  {
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "HttpMessage::read Found header '%s': '%s'",
                                   headerField->data(),
                                   headerField->getValue());
                  }
#                 endif /* MSG_DEBUG */
               }

#endif
               // Get the content length
               {
                  const char* value;

                  if ((value =
                       getHeaderValue(0, HTTP_CONTENT_LENGTH_FIELD)) != NULL)
                  {
                     contentLengthSet = TRUE;
                     contentLength = atoi(value);
                  } else if ((value =
                              getHeaderValue(0, SIP_SHORT_CONTENT_LENGTH_FIELD)) != NULL)
                  {
                     contentLengthSet = TRUE;
                     contentLength = atoi(value);
                  } 
               }

               // Get the content type
               contentTypeSet = getContentType(&contentType);

#ifdef TEST
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "HttpMessage::read contentLengthSet %d, "
                             "contentLength %d, contentTypeSet %d, "
                             "contentType '%s'",
                             contentLengthSet, contentLength, contentTypeSet,
                             contentType.data());
#endif

               // If this is UDP we know that the message has
               // a maximum size of 64K
               if (socketType == OsSocket::UDP &&
                  contentLength > MAX_UDP_MESSAGE)
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "HttpMessage::read Content-Length too big for "
                                "UDP: %d, from %s:%d assuming: %d",
                                contentLength, remoteHost.data(), remotePort,
                                MAX_UDP_MESSAGE);
                  contentLength = MAX_UDP_MESSAGE;
               }

               // Make sure we do not try to allocate something
               // outrageously large.
               if (contentLength > maxContentLength)
               {
                  contentLengthSet = FALSE;
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "HttpMessage::read Content-Length too big: %d,"
                                "closing socket type: %d to %s:%d",
                                contentLength, socketType, remoteHost.data(),
                                remotePort);
                  // Shut it all down, because it may be an abusive sender.
                  inSocket->close();
                  allBytes->remove(0);
                  break;
               }

               // If a content length is set adjust the capacity
               // to minimize the number of resizing and memory
               // shuffling operations
               else if (contentLength > 0)
               {
                  byteCapacity = headerEnd + contentLength + 100;

                  allBytes->capacity(headerEnd + contentLength + 100);
               }
            }
         }

         // If we know we have all of the message, exit this loop.
         // To know we have all of the message, we need to have seen the end of the headers.
         // If there was a Content-Length, we need to have read that many more bytes.
         // If there was no Content-Length, this socket must have a framed protocol.
         if (headerEnd > 0 &&
             (contentLengthSet ? 
              contentLength + headerEnd <= ((int) allBytes->length()) :
              OsSocket::isFramed(socketType)))
         {
            break;
         }

         // Check to see if this is a non-framed protocol, we have seen the end of the headers,
         // but we haven't seen a Content-Length header.  That is an error condition.
         if (!OsSocket::isFramed(socketType) &&
             headerEnd > 0 &&
             !contentLengthSet)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "HttpMessage::read Message has no Content-Length "
                          "on unframed socket type: %d\n",
                          socketType);
            // Exit the loop with the defective message, because we have no way to find its end.
            break;
         }

         // Check to see if this is a framed protocol, because to get here, the first read
         // of the message did not get to its end.  That is an error condition.
         if (OsSocket::isFramed(socketType))
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "HttpMessage::read Attempt to do second read for a message "
                          "on framed socket type: %d\n",
                          socketType);
            // Exit the loop with the defective message, because we have no way to find its end.
            break;
         }

         // Make sure we set the source of the bytes
         if (remoteHost.isNull())
         {
            inSocket->getRemoteHostIp(&remoteHost);
            remotePort = inSocket->getRemoteHostPort();
            setSendAddress(remoteHost.data(), remotePort);
         }

         // Read more of the message and continue processing it.
      } while (inSocket->isOk() &&
               (OsSocket::isFramed(socketType) ||
                inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS)) &&
               (bytesRead = inSocket->read(buffer, bufferSize)) > 0);

      //
      // We have reached one of the conditions that indicates to stop
      // reading the HTTP message.  Decide whether we have finished
      // reading successfully.
      //

      // Calculate the end of the message & body length
      unsigned int bodyLength = 0;
      unsigned int messageLength = 0;
      if (headerEnd > 0 &&
          (!contentLengthSet ||
           OsSocket::isFramed(socketType)))
      {
         messageLength = allBytes->length();
         bodyLength = messageLength - headerEnd;
      }
      else if (headerEnd > 0 && contentLengthSet)
      {
         // We have found a Content-Length header.

         //only if the total bytes read is as expected
         //ie equal or greater than (contentLength + headerEnd)
         if (bytesTotal - headerEnd >=  contentLength)
         {
            // We have the entire expected length of the message.
            bodyLength = contentLength;
            messageLength = headerEnd + contentLength;

            // There is residual bytes for the next message
            if (messageLength < allBytes->length())
            {
#ifdef TEST_PRINT
               int residual = allBytes->length() - messageLength;
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "HttpMessage::read bytes left over: %d\n",
                             residual);
#endif

               // Get rid of initial white space as the residual is
               // supposed to be the beginning of the next message
               int endOfWhiteSpace = FALSE;
               while(messageLength < allBytes->length() &&
                     !endOfWhiteSpace)
               {
                  switch((allBytes->data())[messageLength])
                  {
                  case ' ':
                  case '\n':
                  case '\r':
                  case '\t':
                     messageLength++;
                     break;
                  default:
                     endOfWhiteSpace = TRUE;
                     break;
                  }
               }
#ifdef TEST_PRINT
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "HttpMessage::read trimming whitespace from "
                             "residual: %d\n",
                             residual + messageLength - allBytes->length());
#endif
            }
         }
         else
         {
            // We do not have the entire expected length of the message.
            bodyLength = bytesTotal - headerEnd;
#           ifdef MSG_DEBUG
            // At this point, the entire message should have been read
            // (in multiple reads if necessary).
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "HttpMessage::read Not all content data "
                          "successfully read: received %d body bytes but "
                          "Content-Length was %d",
                          bodyLength, contentLength);
#           endif /* MSG_DEBUG */
#ifdef TEST
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "HttpMessage::read protocol %d, remoteHost %s, "
                          "remotePort %d",
                          socketType, remoteHost.data(), remotePort);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "HttpMessage::read Content:\n>>>%.*s<<<\n",
                          allBytes->length(), allBytes->data());
#endif
         }
      }
      else if (allBytes->length() > 0)
      {
         // We have not found the end of headers, or we have not found
         // a Content-Length header.

#        ifdef MSG_DEBUG
         // This should not happen because the message will have been
         // fetched with multiple reads if necessary.
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "HttpMessage::read End of headers not found.  "
                       "%d bytes read.  Content:\n>>>%.*s<<<\n",
                       allBytes->length(), allBytes->length(),
                       allBytes->data());
#        endif /* MSG_DEBUG */
      }

      // If there is a body.
      if (headerEnd > 0 && bodyLength)
      {
         // All that is left is the body to deal with
         parseBody(&(allBytes->data()[headerEnd]),
                   bodyLength);
      }

      delete[] buffer;

      returnMessageLength = messageLength;
   }
   else
   {
      // Attempt to resize the input buffer to the requested
      // approximate size (allBytes->capacity(bufferSize)) failed, so
      // return an error.
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "HttpMessage::read allBytes->capacity(%d) failed, "
                    "returning %d",
                    bufferSize, byteCapacity);
      returnMessageLength = 0;
   }

   return(returnMessageLength);
}

UtlBoolean HttpMessage::write(OsSocket* outSocket) const
{
        UtlString buffer;
        int bufferLen;
        int bytesWritten;

        getBytes(&buffer, &bufferLen);
        bytesWritten = outSocket->write(buffer.data(), bufferLen);
        return(bytesWritten == bufferLen);
}

void HttpMessage::unescape(UtlString& escapedText)
{
    //UtlString unescapedText;
    int numUnescapedChars = 0;
    const char* unescapedTextPtr = escapedText;
    // The number of unescaped characters is always less than
    // or equal to the number of escaped characters.  Therefore
    // we will cheat a little and used the escapedText as
    // the destiniation to directly write characters in place
    // as the append method is very expensive
    char* resultPtr = new char[escapedText.length() + 1];
    int escapedChar = -1;

    while(*unescapedTextPtr)
    {
        // Substitute escaped space
        if(*unescapedTextPtr == '+')
        {
            //unescapedText.append(' ');
            resultPtr[numUnescapedChars] = ' ';
            numUnescapedChars++;
        }

        // Substitute escaped hex char
        else if(*unescapedTextPtr == '%')
        {
            unescapedTextPtr++;
            if(*unescapedTextPtr)
            {
                if(*unescapedTextPtr >= '0' &&
                    *unescapedTextPtr <= '9')
                {
                    escapedChar = (*unescapedTextPtr - '0') * 16;
                }
                else if(*unescapedTextPtr >= 'a' &&
                    *unescapedTextPtr <= 'f')
                {
                    escapedChar = (*unescapedTextPtr - 'W') * 16;
                }
                else if(*unescapedTextPtr >= 'A' &&
                    *unescapedTextPtr <= 'F')
                {
                    escapedChar = (*unescapedTextPtr - '7') * 16;
                }

                unescapedTextPtr++;
                if(*unescapedTextPtr)
                {
                    if(*unescapedTextPtr >= '0' &&
                        *unescapedTextPtr <= '9')
                    {
                        escapedChar += (*unescapedTextPtr - '0');
                    }
                    else if(*unescapedTextPtr >= 'a' &&
                        *unescapedTextPtr <= 'f')
                    {
                        escapedChar += (*unescapedTextPtr - 'W');
                    }
                    else if(*unescapedTextPtr >= 'A' &&
                        *unescapedTextPtr <= 'F')
                    {
                        escapedChar += (*unescapedTextPtr - '7');
                    }
#ifdef TEST_PRINT
                    osPrintf("HttpMessage::unescape char: %d\n", escapedChar);
#endif
                    //unescapedText.append((char) escapedChar);
                    resultPtr[numUnescapedChars] = (char) escapedChar;
                    numUnescapedChars++;
                }
                else
                {
#ifdef TEST_PRINT
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Bogus dude: escaped char wo/ 2 hex digits: %s\n",
                        escapedText.data());
#endif
                    break;
                }
            }
            else
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "Bogus dude: escaped char wo/ 2 hex digits: %s\n",
                    escapedText.data());
#endif
                break;
            }
        }

        // Char is face value
        else
        {
            //unescapedText.append(unescapedTextPtr, 1);
            resultPtr[numUnescapedChars] = *unescapedTextPtr;
            numUnescapedChars++;
        }

        // Go to the next character
        unescapedTextPtr++;
    }

    //escapedText = unescapedText;

    resultPtr[numUnescapedChars] = '\0';
    escapedText.replace(0, numUnescapedChars, resultPtr);
    escapedText.remove(numUnescapedChars);
    delete[] resultPtr;
}

void HttpMessage::escape(UtlString& unEscapedText)
{
    UtlString escapedText;
    escapedText.capacity((size_t) unEscapedText.length());
    const char* unescapedTextPtr = unEscapedText.data();
    char unEscapedChar;
    char escapedChar[4];

    while(*unescapedTextPtr)
    {
        unEscapedChar = *unescapedTextPtr;
        if((unEscapedChar >= 'a' && unEscapedChar <= 'z') ||
            (unEscapedChar >= 'A' && unEscapedChar <= 'Z') ||
            (unEscapedChar >= '0' && unEscapedChar <= '9') ||
            unEscapedChar == '-' || //dash
                        unEscapedChar == '_' || //underscore
                        unEscapedChar == '.' )
        {
            escapedText.append(&unEscapedChar, 1);
        }
        else if(unEscapedChar == ' ')
        {
            escapedText.append("+", 1);
        }
        else
        {
            sprintf(escapedChar, "%%%02X", (int)(unEscapedChar & 0xff));
#ifdef TEST_PRINT
            osPrintf("%d escaped: %s\n", (int) unEscapedChar,
                escapedChar);
#endif
            escapedText.append(escapedChar);
        }
                unescapedTextPtr++;
    }
    unEscapedText = escapedText;
}

void HttpMessage::escapeOneChar(UtlString& unEscapedText, char tobeEscapedChar)
{
    UtlString escapedText;
    escapedText.capacity((size_t) unEscapedText.length());
    const char* unescapedTextPtr = unEscapedText.data();
    char unEscapedChar;
    char escapedChar[4];

    while(*unescapedTextPtr)
    {
        unEscapedChar = *unescapedTextPtr;
        if(unEscapedChar == tobeEscapedChar )
        {
            sprintf(escapedChar, "%%%02X", (int) (unEscapedChar & 0xff));
#ifdef TEST_PRINT
            osPrintf("%d escaped: %s\n", (int) unEscapedChar,
                escapedChar);
#endif
            escapedText.append(escapedChar);
        }
        else
            escapedText.append(&unEscapedChar, 1);

                unescapedTextPtr++;
    }
    unEscapedText = escapedText;
}


void HttpMessage::escapeChars(UtlString& unEscapedText, UtlString& tobeEscapedChars)
{
   const char* tobeEscapedcharsPtr = tobeEscapedChars.data();
   char unEscapedChar;
   while (*tobeEscapedcharsPtr)
   {
      unEscapedChar = *tobeEscapedcharsPtr;
      escapeOneChar(unEscapedText, unEscapedChar);
      tobeEscapedcharsPtr++;
   }
}

void HttpMessage::cannonizeToken(UtlString& token)
{
    int len = token.length();
    UtlBoolean capNextChar = TRUE;
    const char* tokenPtr = token.data();
    char thisChar;
    for(int capCharIndex = 0; capCharIndex < len; capCharIndex++)
    {
        thisChar = tokenPtr[capCharIndex];
        if(capNextChar)
        {
            if(thisChar >= 'a' &&
               thisChar <= 'z')
            {
               token.replaceAt(capCharIndex, toupper(thisChar));
            }
            capNextChar = FALSE;
        }
        else if(thisChar >= 'A' &&
               thisChar <= 'Z')
        {
            token.replaceAt(capCharIndex, tolower(thisChar));
        }

        if(thisChar == '-')
        {
            capNextChar = TRUE;
        }
    }

#ifdef TEST_PRINT
    //osPrintf("HttpMessage::cannonizeToken \"%s\"\n",
    //    token.data());
#endif
}

void HttpMessage::logTimeEvent(const char* eventName)
{
#ifdef HTTP_TIMELOG
    mTimeLog.addEvent(eventName);
#endif
}

void HttpMessage::dumpTimeLog() const
{
#ifdef HTTP_TIMELOG
    mTimeLog.dumpLog();
#endif
}
/* ============================ ACCESSORS ================================= */
int HttpMessage::getHttpMessageCount()
{
    return(smHttpMessageCount);
}

const char* HttpMessage::getFirstHeaderLine() const
{
        return(mFirstHeaderLine.data());
}

void HttpMessage::setFirstHeaderLine(const char* newHeaderLine)
{
    mHeaderCacheClean = FALSE;
        if(newHeaderLine)
    {
        mFirstHeaderLine.remove(0);
        mFirstHeaderLine.append(newHeaderLine);
    }
    else
    {
        mFirstHeaderLine.remove(0);
    }
}

void HttpMessage::setFirstHeaderLine(const char* subfield0, const char* subfield1,
                                                const char* subfield2)
{
        UtlString headerValue;
        headerValue.append(subfield0);
        headerValue.append(HEADER_LINE_PART_DELIMITER);
        headerValue.append(subfield1);
        headerValue.append(HEADER_LINE_PART_DELIMITER);
        headerValue.append(subfield2);

        setFirstHeaderLine(headerValue.data());
}


void HttpMessage::setTransportTime(long timeStamp)
{
        transportTimeStamp = timeStamp;
}

void HttpMessage::touchTransportTime()
{
   //long now = OsDateTime::getSecsSinceEpoch();
   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   setTransportTime(time.seconds());
}

long HttpMessage::getTransportTime() const
{
        return(transportTimeStamp);
}

void HttpMessage::setResendDuration(int resendMSec)
{
        lastResendDuration = resendMSec;
}

int HttpMessage::getResendDuration() const
{
        return(lastResendDuration);
}

int HttpMessage::getTimesSent() const
{
        return(timesSent);
}

void HttpMessage::incrementTimesSent()
{
        timesSent++;
}

void HttpMessage::setTimesSent(int times)
{
        timesSent = times;
}

void HttpMessage::setSendProtocol(OsSocket::IpProtocolSocketType protocol)
{
        transportProtocol = protocol;
}

OsSocket::IpProtocolSocketType HttpMessage::getSendProtocol() const
{
        return(transportProtocol);
}

void HttpMessage::setFirstSent()
{
    mFirstSent = TRUE;
}

void HttpMessage::setSendAddress(const char* address, int port)
{
    mSendAddress.remove(0);
    if(address) mSendAddress.append(address);
    mSendPort = port;
}

void HttpMessage::getSendAddress(UtlString* address, int* port) const
{
    *address = mSendAddress;
    *port = mSendPort;
}

void HttpMessage::resetTransport()
{
    transportTimeStamp = 0;
        lastResendDuration = 0;
        timesSent = 0;
        transportProtocol = OsSocket::UNKNOWN;
    mFirstSent = FALSE;
    mSendAddress = "";
    mSendPort = PORT_NONE;
}

OsMsgQ* HttpMessage::getResponseListenerQueue()
{
    return(mpResponseListenerQueue);
}

void HttpMessage::setResponseListenerQueue(OsMsgQ* responseListenerQueue)
{
    mpResponseListenerQueue = responseListenerQueue;
}

void* HttpMessage::getResponseListenerData()
{
    return(mResponseListenerData);
}

void HttpMessage::setResponseListenerData(void* responseListenerData)
{
    mResponseListenerData = responseListenerData;
}

int HttpMessage::getCountHeaderFields(const char* name) const
{
        int fieldCount;
        if(name)
        {
                UtlString nameString(name);
                nameString.toUpper();
                UtlString nameCollectable(nameString);
                fieldCount = mNameValues.occurrencesOf(&nameCollectable);
        }
        else
        {
                fieldCount = mNameValues.entries();
        }
        return(fieldCount);
}

NameValuePair* HttpMessage::getHeaderField(int index, const char* name) const
{
        UtlDListIterator iterator((UtlDList&)mNameValues);
        //NameValuePair* headerFieldName = NULL;
        NameValuePair* headerField = NULL;
        int fieldIndex = 0;

        //iterator.reset();

        //if(name)
        //{
                //UtlString upperCaseName(name);
                //upperCaseName.toUpper();
    //UtlString headerFieldName(name ? name : "");
    //headerFieldName.toUpper();
        //}

    //int nameLength = 0;
    //if(name)
    //{
    //    nameLength = strlen(name);
    //}

        // For each name value:
        while(fieldIndex <= index)
        {
                // Go to the next header field
                if(name)
                {
                   // Too slow and too much overhead (i.e. needs UtlContainable string
                   // to be constructed and destroyed)
                   // headerField = (NameValuePair*) iterator.findNext(&headerFieldName);

                   // Find a header with a matching name
                   do
                   {
                      headerField = (NameValuePair*) iterator();
                   }
                   while(headerField &&
                         strcasecmp(name, headerField->data()) != 0);
                }

                else
                {
                        headerField = (NameValuePair*) iterator();
                }


                if(!headerField)
                {
                        break;
                }
                fieldIndex++;
        }


        //if(headerFieldName)
        //{
        //      delete headerFieldName;
        //      headerFieldName = NULL;
        //}
        return(headerField);
}

const char* HttpMessage::getHeaderValue(int index, const char* name) const
{
        const char* value = NULL;
        NameValuePair* headerField = getHeaderField(index, name);

        if(headerField)
        {
                value = headerField->getValue();
        }

        return(value);
}

void HttpMessage::setHeaderValue(const char* name, const char* newValue, int index)
{
    mHeaderCacheClean = FALSE;
        NameValuePair* headerField = getHeaderField(index, name);

        if(headerField)
        {
                headerField->setValue(newValue);
        }
        else
        {
                addHeaderField(name, newValue);
        }

}

UtlBoolean HttpMessage::removeHeader(const char* name, int index)
{
   mHeaderCacheClean = FALSE;
   UtlBoolean foundHeader = FALSE;
   UtlDListIterator iterator((UtlDList&)mNameValues);
   NameValuePair* headerFieldName = NULL;
   NameValuePair* headerField = NULL;
   int fieldIndex = 0;

   if(name)
   {
      headerFieldName = new NameValuePair(name);
      headerFieldName->toUpper();
   }

   // For each name value:
   while(fieldIndex <= index)
   {
      // Go to the next header field
      if(name)
      {
         headerField = (NameValuePair*) iterator.findNext(headerFieldName);
      }
      else
      {
         headerField = (NameValuePair*) iterator();
      }


      if(!headerField)
      {
         break;
      }
      fieldIndex++;
   }


   if(headerFieldName)
   {
      delete headerFieldName;
      headerFieldName = NULL;
   }
   if(headerField)
   {
      mNameValues.removeReference(headerField);
      delete headerField;
      foundHeader = TRUE;
   }

   return(foundHeader);
}

void HttpMessage::addHeaderField(const char* name, const char* value)
{
    mHeaderCacheClean = FALSE;
    NameValuePair* headerField =
        new NameValuePair(name ? name : "", value);
    headerField->toUpper();
        mNameValues.insert(headerField);
}

void HttpMessage::insertHeaderField(const char* name,
                                    const char* value,
                                    int index)
{
    mHeaderCacheClean = FALSE;
    NameValuePair* headerField =
        new NameValuePair(name ? name : "", value);
    headerField->toUpper();
        mNameValues.insertAt(index, headerField);
}

const HttpBody* HttpMessage::getBody() const
{
        return(body);
}

void HttpMessage::setBody(HttpBody* newBody)
{
        if(body)
        {
                delete body;
        }
        body = newBody;
}

UtlBoolean HttpMessage::getContentType(UtlString* contentTypeString) const
{
        const char* contentType = getHeaderValue(0, HTTP_CONTENT_TYPE_FIELD);
        contentTypeString->remove(0);
        if(contentType)
        {
                contentTypeString->append(contentType);
                contentTypeString->strip(UtlString::both);
        }
        return(contentType != NULL);
}

void HttpMessage::setContentType(const char* contentTypeString)
{
        setHeaderValue(HTTP_CONTENT_TYPE_FIELD, contentTypeString);
}

void HttpMessage::setDateField()
{
    OsDateTime now;
    OsDateTime::getCurTime(now);
    UtlString nowString;
    now.getHttpTimeString(nowString);
    setHeaderValue(HTTP_DATE_FIELD, nowString.data());
}

int HttpMessage::getContentLength() const
{
        const char* contentLength = getHeaderValue(0, HTTP_CONTENT_LENGTH_FIELD);
        int length = 0;
        if (contentLength)
                length = atoi(contentLength);
        return(length);
}

void HttpMessage::setContentLength(int contentLength)
{
        char contentLengthString[HTTP_LONG_INT_CHARS];
        sprintf(contentLengthString, "%d", contentLength);
        setHeaderValue(HTTP_CONTENT_LENGTH_FIELD, contentLengthString);
}

void HttpMessage::getUserAgentField(UtlString* userAgentField) const
{
        const char* userAgent = getHeaderValue(0, HTTP_USER_AGENT_FIELD);
        userAgentField->remove(0);
        if(userAgent)
        {
                userAgentField->append(userAgent);
        }
}

void HttpMessage::setUserAgentField(const char* userAgentField)
{
        setHeaderValue(HTTP_USER_AGENT_FIELD, userAgentField);
}

void HttpMessage::setRefresh(int seconds, const char* refreshUrl)
{
    char refreshLenString[HTTP_LONG_INT_CHARS];
    sprintf(refreshLenString, "%d", seconds);
    UtlString refreshField(refreshLenString);
    refreshField.append(" ");
    if(refreshUrl && refreshUrl[0])
    {
        refreshField.append("URL=");
        refreshField.append(refreshUrl);
    }
    setHeaderValue(HTTP_REFRESH_FIELD, refreshField.data());
}

UtlBoolean HttpMessage::getDateField(long* epochDate) const
{
    const char* dateField = getHeaderValue(0, HTTP_DATE_FIELD);
    if(dateField)
    {
        *epochDate = OsDateTime::convertHttpDateToEpoch(dateField);
        // returns zero if the format is not understood
        if(! *epochDate)
        {
#ifdef TEST_PRINT
            osPrintf("WARNING: unsupported date format\n");
            osPrintf("Date field: \"%s\"\n", dateField);
            osPrintf("epoch date: %d\n", *epochDate);
#endif
        }
    }

    return(dateField != NULL && *epochDate);
}

UtlBoolean HttpMessage::getAcceptField(UtlString& acceptValue) const
{
    const char* value = getHeaderValue(0, HTTP_ACCEPT_FIELD);
    acceptValue.remove(0);
    if(value) 
    {
        acceptValue = value;
    }
    return(value != NULL);
}

void HttpMessage::getAcceptLanguageField(UtlString* acceptLanaguageFieldValue) const
{
        const char* language = getHeaderValue(0, HTTP_ACCEPT_LANGUAGE_FIELD);
        acceptLanaguageFieldValue->remove(0);
        if(language)
        {
                acceptLanaguageFieldValue->append(language);
        }
}

void HttpMessage::setAcceptLanguageField(const char* acceptLanaguageFieldValue)
{
        setHeaderValue(HTTP_ACCEPT_LANGUAGE_FIELD, acceptLanaguageFieldValue);
}

void HttpMessage::setLocationField(const char* locationField)
{
    setHeaderValue(HTTP_LOCATION_FIELD, locationField);
}

void HttpMessage::getBytes(UtlString* bufferString, int* length) const
{
        *length = 0;
        UtlString name;
        const char* value;

        *bufferString = mFirstHeaderLine;

        bufferString->append(END_OF_LINE_DELIMITOR);

        UtlDListIterator iterator((UtlDList&)mNameValues);
        NameValuePair* headerField;
    UtlBoolean foundContentLengthHeader = FALSE;
        int bodyLen = 0;
        UtlString bodyBytes;
        if(body)
        {
                body->getBytes(&bodyBytes, &bodyLen);
    }

    int oldLen = 0 ;
    if(mHeaderCacheClean &&
        bodyLen == (oldLen = getContentLength()))
    {
        // We have a clean serialized cache of the first header line
        // and headers, use it instead of serializing it

        // Check if the content length is explicitly set
        //if(oldLen > 0 ||  // avoid getHeaderField call if content length set
        //   getHeaderField(0, HTTP_CONTENT_LENGTH_FIELD))
        //{
        //    foundContentLengthHeader = TRUE;
        //}

        // dummy code to allow instrumentation and setting of break points
        oldLen = 0;
    }
    else
    {
        // we have to serialize the headers and cache the result
        // This cast is a bit of hack for this instrumention so that
        // the const signature does not have to change
        ((HttpMessage*)this)->mHeaderCacheClean = TRUE;

        // dummy code to allow instrumentation and setting of break points
        oldLen = 0;
    }

        // For each name value:
        while((headerField = (NameValuePair*) iterator()))
        {
                // Do not free up name and data as this are contained
                // in the NameValuePair
                name = *headerField;
        cannonizeToken(name);
                value = headerField->getValue();

        // Keep track while we are looping through if we see a
        // content-length header or not
        if(name.compareTo(HTTP_CONTENT_LENGTH_FIELD, UtlString::ignoreCase) == 0)
        {
            foundContentLengthHeader = TRUE;
            int fieldBodyLengthValue = atoi(value ? value : "");
            if(fieldBodyLengthValue != bodyLen)
            {
                char bodyLengthString[40];
                sprintf(bodyLengthString, "%d", bodyLen);
                OsSysLog::add(FAC_SIP, PRI_WARNING, "HttpMessage::getBytes content-length: %s wrong setting to: %s",
                    value ? value : "", bodyLengthString);
                headerField->setValue(bodyLengthString);
                value = headerField->getValue();
            }
        }

                bufferString->append(name);
                bufferString->append(HTTP_NAME_VALUE_DELIMITER);
                bufferString->append(" ");
                if(value)
                {
                        bufferString->append(value);
                }
                bufferString->append(END_OF_LINE_DELIMITOR);
        }

        // Make sure the content length is set
        if(!foundContentLengthHeader)
        {
                UtlString ContentLen(HTTP_CONTENT_LENGTH_FIELD);
                cannonizeToken(ContentLen);
                bufferString->append(ContentLen);
                bufferString->append(HTTP_NAME_VALUE_DELIMITER);
        char bodyLengthString[40];
        sprintf(bodyLengthString, " %d", bodyLen);
                bufferString->append(bodyLengthString);
                bufferString->append(END_OF_LINE_DELIMITOR);
        }

        bufferString->append(END_OF_LINE_DELIMITOR);

        if(body)
        {
                bufferString->append(bodyBytes.data(), bodyLen);
        }

        *length = bufferString->length();
}

void HttpMessage::getFirstHeaderLinePart(int partIndex, UtlString* part, char separator) const
{
        const char* partStart = mFirstHeaderLine.data();
    // Tolerate separators in the begining
    while(*partStart == separator) partStart++;

        const char* partEnd;
        int index = 0;
        part->remove(0);

        // Find the begining
        while(partStart && index < partIndex)
        {
                partStart = strchr(partStart, separator);
                if(partStart == NULL) break;
                partStart++;
        // Tolerate multiple consecutive separators
        while(*partStart == separator) partStart++;

                index++;
        }

        // If there is a begining find the end
        if(partStart)
        {
                if(partIndex < 2)
                {
                        partEnd = strchr(partStart, separator);
                        if(partEnd == NULL)
                        {
                                partEnd = partStart + strlen(partStart);
                        }

                        int len = partEnd - partStart;
                        part->append(partStart, len);
                        //part->append("",1);
                }

                // This is the third part take the whole thing
                else
                {
                        part->append(partStart);
                }
        }
}

// Response access methods
   void HttpMessage::setResponseFirstHeaderLine(const char* protocol,
           int statusCode, const char* statusText)
   {
           char codeBuffer[HTTP_LONG_INT_CHARS];
           sprintf(codeBuffer, "%d", statusCode);

           setFirstHeaderLine(protocol, codeBuffer, statusText);
   }

   void HttpMessage::getResponseProtocol(UtlString* protocol) const
   {
           getFirstHeaderLinePart(0, protocol);
   }

        int HttpMessage::getResponseStatusCode() const
        {
                UtlString codeString;
           getFirstHeaderLinePart(1, &codeString);
                int ret = atoi(codeString.data());
           return ret;
        }

        void HttpMessage::getResponseStatusText(UtlString* text) const
        {
           getFirstHeaderLinePart(2, text);
                *text = text->strip(UtlString::both);
        }

// Request access methods
void HttpMessage::setRequestFirstHeaderLine(const char* method,
                                            const char* uri,
                                            const char* protocol)
{
   if (uri[0] == '<')
   {
      OsSysLog::add(FAC_SIP,
                    PRI_ERR,
                    "HttpMessage::setRequestFirstHeaderLine(3) request URI has <>: '%s'",
                    uri);
   }
   setFirstHeaderLine(method, uri, protocol);
}

     void HttpMessage::getRequestMethod(UtlString* method) const
         {
                getFirstHeaderLinePart(0, method);
                *method =method->strip(UtlString::both);
         }

     void HttpMessage::getRequestUri(UtlString* uri) const
         {
                getFirstHeaderLinePart(1, uri);
         }

     void HttpMessage::getRequestProtocol(UtlString* protocol) const
         {
                getFirstHeaderLinePart(2, protocol);
                *protocol = protocol->strip(UtlString::both);

         }

         void HttpMessage::changeRequestUri(const char* newUri)
         {
                 UtlString method;
                 UtlString protocol;
                 getFirstHeaderLinePart(0, &method);
                 getFirstHeaderLinePart(2, &protocol);
                 setRequestFirstHeaderLine(method.data(), newUri,
                         protocol.data());
         }

UtlBoolean HttpMessage::getAuthenticationScheme(UtlString* scheme,
                                               int authorizationEntity) const
{
    const char* fieldValue = NULL;
    if(authorizationEntity == SERVER)
    {
        fieldValue = getHeaderValue(0, HTTP_WWW_AUTHENTICATE_FIELD);
    }
    else if(authorizationEntity == PROXY)
    {
        fieldValue = getHeaderValue(0, HTTP_PROXY_AUTHENTICATE_FIELD);
    }

    if(fieldValue)
    {
        NetAttributeTokenizer tokenizer(fieldValue);
        UtlString dummy;

        tokenizer.getNextAttribute(*scheme, dummy);
        cannonizeToken(*scheme);
    }

    return(fieldValue != NULL);
}

void HttpMessage::setAuthenticationData(const char* scheme,
                                        const char* realm,
                                        const char* nonce,
                                        const char* opaque,
                                        const char* domain,
                                        enum HttpEndpointEnum authEntity)
{
    UtlString schemeString;
    UtlString authField;

    if(scheme)
    {
        schemeString.append(scheme);
        authField.append(scheme);
        cannonizeToken(authField);
    }
    else
    {
        authField.append(HTTP_BASIC_AUTHENTICATION); // :TBD: should be Digest
    }
    authField.append(' ');
    authField.append(HTTP_AUTHENTICATION_REALM_TOKEN);
    authField.append('=');
    if(realm)
    {
        authField.append('\"');
        authField.append(realm);
        authField.append('\"');
    }
    else
    {
        OsSysLog::add( FAC_SIP, PRI_ERR,
                      "HttpMessage::setAuthenticationData: no realm specified"
                      );
    }

    if( 0 == schemeString.compareTo(HTTP_DIGEST_AUTHENTICATION,
                                    UtlString::ignoreCase
                                    )
       )
    {
        if(domain && *domain)
        {
            authField.append(", ");
            authField.append(HTTP_AUTHENTICATION_DOMAIN_TOKEN);
            authField.append("=\"");
            authField.append(domain);
            authField.append('\"');
        }
        if(nonce && *nonce)
        {
            authField.append(", ");
            authField.append(HTTP_AUTHENTICATION_NONCE_TOKEN);
            authField.append("=\"");
            authField.append(nonce);
            authField.append('\"');
        }
        if(opaque && *opaque)
        {
            authField.append(", ");
            authField.append(HTTP_AUTHENTICATION_OPAQUE_TOKEN);
            authField.append("=\"");
            authField.append(opaque);
            authField.append('\"');
        }

        // :TBD: should add qop
    }

    //setHeaderValue(HTTP_WWW_AUTHENTICATE_FIELD, authField.data(), 0);
    addAuthenticationField(authField.data(), authEntity);
}

UtlBoolean HttpMessage::getAuthenticationData(UtlString* scheme,
                                             UtlString* realm,
                                             UtlString* nonce,
                                             UtlString* opaque,
                                             UtlString* algorithm, // MD5 or MD5-sess
                                             UtlString* qop, // may be multiple values
                                             int authorizationEntity) const
{
    const char* fieldValue = NULL;
    if(authorizationEntity == SERVER)
    {
        fieldValue = getHeaderValue(0, HTTP_WWW_AUTHENTICATE_FIELD);
    }
    else if(authorizationEntity == PROXY)
    {
        fieldValue = getHeaderValue(0, HTTP_PROXY_AUTHENTICATE_FIELD);
    }

    if(fieldValue)
    {
        NetAttributeTokenizer tokenizer(fieldValue);
        UtlString name;
        UtlString value;

        if(realm) realm->remove(0);
        if(nonce) nonce->remove(0);
        if(opaque) opaque->remove(0);
        if(algorithm) algorithm->remove(0);
        if(qop) qop->remove(0);

        tokenizer.getNextAttribute(*scheme, value);
        cannonizeToken(*scheme);

        // Search for tokens independent of order
        while(tokenizer.getNextAttribute(name, value))
        {
            name.toLower();
            if(   realm
               && name.compareTo(HTTP_AUTHENTICATION_REALM_TOKEN, UtlString::ignoreCase) == 0
               )
            {
                realm->append(value.data());
            }
            else if(nonce && name.compareTo(HTTP_AUTHENTICATION_NONCE_TOKEN, UtlString::ignoreCase) == 0)
            {
                nonce->append(value.data());
            }
            else if(opaque && name.compareTo(HTTP_AUTHENTICATION_OPAQUE_TOKEN, UtlString::ignoreCase) == 0)
            {
                opaque->append(value.data());
            }
            else if(algorithm && name.compareTo(HTTP_AUTHENTICATION_ALGORITHM_TOKEN, UtlString::ignoreCase) == 0)
            {
                algorithm->append(value.data());
            }
            else if(qop && name.compareTo(HTTP_AUTHENTICATION_QOP_TOKEN, UtlString::ignoreCase) == 0)
            {
                qop->append(value.data());
            }
        }
    }

    return(fieldValue != NULL);
}

UtlBoolean HttpMessage::getAuthorizationUser(UtlString* userId) const
{
    UtlBoolean foundUserId = FALSE;
    UtlString scheme;
    UtlString dummy;

    getAuthorizationScheme(&scheme);
#ifdef TEST
    osPrintf("HttpMessage::getAuthorizationUser authorization scheme: \"%s\"\n",
        scheme.data());
#endif

    // Basic
    if(scheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
    {
        foundUserId = getBasicAuthorizationData(userId, &dummy);
#ifdef TEST_PRINT
        if(foundUserId)
        {
            osPrintf("HttpMessage::getAuthorizationUser userId: \"%s\" from message\n",
                userId->data());
        }
        else
        {
            osPrintf("HttpMessage::getAuthorizationUser failed to get userId from message\n");
        }
#endif
    }

    // Digest
    else if(scheme.compareTo(HTTP_DIGEST_AUTHENTICATION, UtlString::ignoreCase) == 0)
    {
        getDigestAuthorizationData(userId);
    }

    // scheme not supported
    else
    {
        userId->remove(0);
    }

    return(foundUserId);
}

UtlBoolean HttpMessage::getAuthorizationField(UtlString* authenticationField,
                                             int authorizationEntity) const
{
    const char* fieldValue = NULL;
    if(authorizationEntity == SERVER)
    {
        fieldValue = getHeaderValue(0, HTTP_AUTHORIZATION_FIELD);
    }
    else if(authorizationEntity == PROXY)
    {
        fieldValue = getHeaderValue(0, HTTP_PROXY_AUTHORIZATION_FIELD);
    }

    authenticationField->remove(0);

    if(fieldValue)
    {
        authenticationField->append(fieldValue);
    }

    return(fieldValue != NULL);
}

UtlBoolean HttpMessage::getDigestAuthorizationData(UtlString* user,
                                                  UtlString* realm,
                                                  UtlString* nonce,
                                                  UtlString* opaque,
                                                  UtlString* response,
                                                  UtlString* uri,
                                                                                                  int authorizationEntity,
                                                  int index) const
{

   const char* value = NULL;
   value = getHeaderValue(index, HTTP_PROXY_AUTHORIZATION_FIELD);

   if(!value)
   {
                value = getHeaderValue(index, HTTP_AUTHORIZATION_FIELD);
        }
    if(value)
    {
        NetAttributeTokenizer tokenizer(value);
        UtlString name;
        UtlString value;
        UtlString scheme;

        if(realm) realm->remove(0);
        if(nonce) nonce->remove(0);
        if(opaque) opaque->remove(0);
        if(user) user->remove(0);
        if(uri) uri->remove(0);
        if(response) response->remove(0);

        // If this is a digest response
        tokenizer.getNextAttribute(scheme, value);
        if( 0 == scheme.compareTo(HTTP_DIGEST_AUTHENTICATION,
                                  UtlString::ignoreCase
                                  )
           )
        {
            // Search for tokens independent of order
            while(tokenizer.getNextAttribute(name, value))
            {
                name.toUpper();
                if(realm && name.compareTo(HTTP_AUTHENTICATION_REALM_TOKEN, UtlString::ignoreCase) == 0)
                {
                    realm->append(value.data());
                }
                else if(nonce && name.compareTo(HTTP_AUTHENTICATION_NONCE_TOKEN, UtlString::ignoreCase) == 0)
                {
                    nonce->append(value.data());
                }
                else if(opaque && name.compareTo(HTTP_AUTHENTICATION_OPAQUE_TOKEN, UtlString::ignoreCase) == 0)
                {
                    opaque->append(value.data());
                }
                else if(user && name.compareTo(HTTP_AUTHENTICATION_USERNAME_TOKEN, UtlString::ignoreCase) == 0)
                {
                    user->append(value.data());
                }
                else if(response && name.compareTo(HTTP_AUTHENTICATION_RESPONSE_TOKEN, UtlString::ignoreCase) == 0)
                {
                    response->append(value.data());
                }
                else if(uri && name.compareTo(HTTP_AUTHENTICATION_URI_TOKEN, UtlString::ignoreCase) == 0)
                {
                    uri->append(value.data());
                }
            }
        }
   }
   return(value != NULL);
}


void HttpMessage::addAuthenticationField(const char* authenticationField,
                                         enum HttpEndpointEnum authEntity)
{
    const char* fieldName = "bad-auth-entity";
    if(authEntity == PROXY)
    {
        fieldName = HTTP_PROXY_AUTHENTICATE_FIELD;
    }

    else if(authEntity == SERVER)
    {
        fieldName = HTTP_WWW_AUTHENTICATE_FIELD;
    }

    addHeaderField(fieldName, authenticationField);
}

UtlBoolean HttpMessage::getAuthenticationField(int index,
                                         enum HttpEndpointEnum authEntity,
                                         const char* authenticationField) const
{
    const char* fieldName = "bad-auth-entity";
    if(authEntity == SERVER)
    {
        fieldName = HTTP_PROXY_AUTHENTICATE_FIELD;
    }

    else if(authEntity == PROXY)
    {
        fieldName = HTTP_WWW_AUTHENTICATE_FIELD;
    }

    const char* value = getHeaderValue(index, fieldName);
    authenticationField = value ? value : "";

    return(value != NULL);
}

void HttpMessage::addAuthenticationField(const char * AuthorizeField,
                                         const char * AuthorizeValue,
                                         UtlBoolean otherAuthentications)
{
  /* NameValuePair* nv = new NameValuePair(AuthorizeField, AuthorizeValue);
    // Look for other via fields
    int fieldIndex = nameValues.index(nv);

    if(fieldIndex == UTL_NOT_FOUND || !afterOtherVias)
    {
        nameValues.insert(nv);
    }
    else
    {
        nameValues.insertAt(fieldIndex, nv);
    }*/
}

void HttpMessage::setDigestAuthorizationData(const char* user,
                                    const char* realm,
                                    const char* nonce,
                                    const char* uri,
                                    const char* response,
                                    const char* algorithm,
                                    const char* cnonce,
                                    const char* opaque,
                                    const char* qop,
                                    int nonceCount,
                                    int authorizationEntity)
{
    UtlString schemeString;
    UtlString authField;

    authField.append(HTTP_DIGEST_AUTHENTICATION);
    if(user && strlen(user))
    {
        authField.append(' ');
        authField.append(HTTP_AUTHENTICATION_USERNAME_TOKEN);
        authField.append("=\"");
        authField.append(user);
        authField.append('\"');
    }
    if(realm)
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_REALM_TOKEN);
        authField.append("=\"");
        authField.append(realm);
        authField.append('\"');
    }
    if(nonce && strlen(nonce))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_NONCE_TOKEN);
        authField.append("=\"");
        authField.append(nonce);
        authField.append('\"');
    }
    if(uri && strlen(uri))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_URI_TOKEN);
        authField.append("=\"");
        authField.append(uri);
        authField.append('\"');
    }
    if(response && strlen(response))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_RESPONSE_TOKEN);
        authField.append("=\"");
        authField.append(response);
        authField.append('\"');
    }
    if(algorithm && strlen(algorithm))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_ALGORITHM_TOKEN);
        authField.append("=");
        authField.append(algorithm);
    }
    UtlString alg(algorithm ? algorithm : "");
    if(cnonce && strlen(cnonce) &&
        ((qop && strlen(qop)) ||
         (algorithm &&  alg.compareTo(HTTP_MD5_SESSION_ALGORITHM, UtlString::ignoreCase) == 0)))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_CNONCE_TOKEN);
        authField.append("=\"");
        authField.append(cnonce);
        authField.append('\"');
    }
    if(opaque && strlen(opaque))
    {
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_OPAQUE_TOKEN);
        authField.append("=\"");
        authField.append(opaque);
        authField.append('\"');
    }
    if(qop && strlen(qop))
    {
        UtlString qopString(qop);
        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_QOP_TOKEN);
        authField.append("=");
        int qopIntIndex = qopString.index(HTTP_QOP_AUTH_INTEGRITY, 0, UtlString::ignoreCase);
        int qopIndex = qopString.index(HTTP_QOP_AUTH, 0, UtlString::ignoreCase);
        if(qopIntIndex >= 0)
        {
            authField.append(HTTP_QOP_AUTH_INTEGRITY);
        }
        else if(qopIndex >= 0)
        {
            authField.append(HTTP_QOP_AUTH);
        }
    }
    if(nonceCount > 0 &&
       qop && strlen(qop))
    {
        char nonceCountBuffer[20];
        sprintf(nonceCountBuffer, "%.8x", nonceCount);
        UtlString nonceCountString(nonceCountBuffer);
        nonceCountString.toLower();

        authField.append(", ");
        authField.append(HTTP_AUTHENTICATION_NONCE_COUNT_TOKEN);
        authField.append('=');
        authField.append(nonceCountString);
    }

    if(authorizationEntity == SERVER)
    {
        addHeaderField(HTTP_AUTHORIZATION_FIELD, authField.data());
    }
    else if(authorizationEntity == PROXY)
    {
        addHeaderField(HTTP_PROXY_AUTHORIZATION_FIELD, authField.data());
    }
}

void HttpMessage::buildMd5UserPasswordDigest(const char* user,
                                             const char* realm,
                                             const char* password,
                                             UtlString& userPasswordDigest)
{
    // Construct A1
    UtlString a1Buffer;

    if(user) a1Buffer.append(user);
    a1Buffer.append(':');
    if(realm) a1Buffer.append(realm);
    a1Buffer.append(':');
    if(password) a1Buffer.append(password);

    // Encode A1
    NetMd5Codec::encode(a1Buffer.data(), userPasswordDigest);
}

/*void HttpMessage::buildMd5Digest(const char* user, const char* password,
                                  const char* realm, const char* nonce,
                                  const char* uri, const char* method,
                                  UtlString* responseToken)
{
    UtlString encodedA1;
    buildMd5UserPasswordDigest(user, realm, password, encodedA1);


    buildMd5Digest(encodedA1.data(),
        NULL, //algorithm
        nonce,
        NULL, // cnonce
        0, // nonceCount
        NULL, // qop
        method,
        uri,
        NULL, // bodyDigest
        responseToken);
}*/

void HttpMessage::buildMd5Digest(const char* userPasswordDigest,
                                 const char* algorithm,
                                 const char* nonce,
                                 const char* cnonce,
                                 int nonceCount,
                                 const char* qop,
                                 const char* method,
                                 const char* uri,
                                 const char* bodyDigest,
                                 UtlString* responseToken)
{
    // Construct A1
    UtlString encodedA1;
    UtlString alg(algorithm ? algorithm : "");
    if(alg.compareTo(HTTP_MD5_SESSION_ALGORITHM, UtlString::ignoreCase) == 0)
    {
        UtlString a1Buffer(userPasswordDigest);
        a1Buffer.append(':');
        if(nonce) a1Buffer.append(nonce);
        a1Buffer.append(':');
        if(cnonce) a1Buffer.append(cnonce);
        NetMd5Codec::encode(a1Buffer.data(), encodedA1);

    }
    else
    {
        encodedA1 = userPasswordDigest;
    }

    // Construct A2
    UtlString a2Buffer;
    UtlString encodedA2;
    if(method) a2Buffer.append(method);
    a2Buffer.append(':');
    if(uri) a2Buffer.append(uri);
    UtlString qopString(qop ? qop : "");
    UtlBoolean qopInt = FALSE;
    int qopIndex = qopString.index(HTTP_QOP_AUTH_INTEGRITY, 0, UtlString::ignoreCase);
    if(qopIndex >= 0)
    {
        qopInt = TRUE;
        a2Buffer.append(':');
        if(bodyDigest) a2Buffer.append(bodyDigest);
    }

    // Encode A2
    NetMd5Codec::encode(a2Buffer.data(), encodedA2);

    // Construct buffer
    UtlString buffer(encodedA1);
    buffer.append(':');
    if(nonce) buffer.append(nonce);
    qopIndex = qopString.index(HTTP_QOP_AUTH, 0, UtlString::ignoreCase);
    if(qopIndex >= 0)
    {
        char nonceCountBuffer[20];
        sprintf(nonceCountBuffer, "%.8x", nonceCount);
        UtlString nonceCountString(nonceCountBuffer);
        nonceCountString.toLower();

        buffer.append(':');
        buffer.append(nonceCountString);
        buffer.append(':');
        if(cnonce) buffer.append(cnonce);
        buffer.append(':');
        if(qopInt)
        {
            buffer.append(HTTP_QOP_AUTH_INTEGRITY);
        }
        else
        {
            buffer.append(HTTP_QOP_AUTH);
        }
    }
    buffer.append(':');
    buffer.append(encodedA2);

    // Encode buffer
    NetMd5Codec::encode(buffer.data(), *responseToken);

#ifdef TEST_PRINT
    osPrintf("HttpMessage::buildMd5Digest expecting authorization:\n\tuserPasswordDigest: '%s'\n\tnonce: '%s'\n\tmethod: '%s'\n\turi: '%s'\n\tresponse: '%s'\n",
        userPasswordDigest, nonce, method, uri, responseToken->data());
#endif
}

UtlBoolean HttpMessage::verifyMd5Authorization(const char* userId,
                                             const char* password,
                                             const char* nonce,
                                             const char* realm,
                                             const char* thisMessageMethod,
                                             const char* thisMessageUri,
                                             enum HttpEndpointEnum authEntity) const
{
    UtlBoolean allowed = FALSE;
    UtlString uri;
    UtlString method;
    UtlString referenceHash;
    UtlString msgUser;
    UtlString msgRealm;
    UtlString msgNonce;
    UtlString msgOpaque;
    UtlString msgDigestHash;
    UtlString msgUri;

    if(thisMessageUri && *thisMessageUri)
    {
        uri.append(thisMessageUri);
    }
    else
    {
        getRequestUri(&uri);
    }
    if(thisMessageMethod && *thisMessageMethod)
    {
        method.append(thisMessageMethod);
    }
    else
    {
        getRequestMethod(&method);
    }

        // Build a digest hash for the reference
    buildMd5Digest(password,
                   NULL, // algorithm
                   nonce,
                   NULL, // cnonce
                   0, // nonceCount
                   NULL, // qop
                   method.data(),
                   uri.data(),
                   NULL, // body digest
                   &referenceHash);


    // Build a digest hash for the reference
   /* buildMd5Digest(password,"",nonce,"", uri.data(),
                   method.data(), &referenceHash);*/

    // Get the digest hash given in the message
    int authIndex = 0;
    while(getDigestAuthorizationData(&msgUser,
                                     &msgRealm,
                                     &msgNonce,
                                     &msgOpaque,
                                     &msgDigestHash,
                                     &msgUri,
                                     authEntity,
                                     authIndex))
    {
        if((referenceHash.compareTo(msgDigestHash) == 0))
        {
            allowed = TRUE;
            break;
        }
        authIndex++;
    }

#ifdef TEST
    OsSysLog::add(FAC_SIP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization got digest response: \"%s\"\n", msgDigestHash.data());
    OsSysLog::add(FAC_SIP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization wanted     response: \"%s\"\n", referenceHash.data());
#endif
    return(allowed);
}

UtlBoolean HttpMessage::verifyMd5Authorization(const char* userPasswordDigest,
                                             const char* nonce,
                                             const char* thisMessageMethod,
                                             const char* thisMessageUri) const
{
    UtlBoolean allowed;
    UtlString uri;
    UtlString method;
    UtlString referenceHash;
    UtlString msgUser;
    UtlString msgRealm;
    UtlString msgNonce;
    UtlString msgOpaque;
    UtlString msgDigestHash;
    UtlString msgUri;

    if(thisMessageUri && *thisMessageUri)
    {
        uri.append(thisMessageUri);
    }
    else
    {
        getRequestUri(&uri);
    }
    if(thisMessageMethod && *thisMessageMethod)
    {
        method.append(thisMessageMethod);
    }
    else
    {
        getRequestMethod(&method);
    }

    // Build a digest hash for the reference
    buildMd5Digest(userPasswordDigest,
                   NULL, // algorithm
                   nonce,
                   NULL, // cnonce
                   0, // nonceCount
                   NULL, // qop
                   method.data(),
                   uri.data(),
                   NULL, // body digest
                   &referenceHash);

    // Get the digest hash given in the message
    allowed = getDigestAuthorizationData(&msgUser, &msgRealm, &msgNonce, &msgOpaque,
                                         &msgDigestHash, &msgUri);
    if(allowed)
    {
        allowed = (referenceHash.compareTo(msgDigestHash) == 0);
    }

#ifdef TEST
    OsSysLog::add(FAC_SIP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization got digest response: \"%s\"\n", msgDigestHash.data());
    OsSysLog::add(FAC_SIP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization wanted     response: \"%s\"\n", referenceHash.data());
#endif
    return(allowed);
}

void HttpMessage::buildBasicAuthorizationCookie(const char* user,
                                                const char* password,
                                                UtlString* cookie)
{
    UtlString clearToken;

    // Create the base64 encoded user:password cookie
    if(user)
    {
        clearToken.append(user);
    }
    clearToken.append(':');
    if(password)
    {
        clearToken.append(password);
    }
    NetBase64Codec::encode(clearToken.length(), clearToken.data(), *cookie);
}

void HttpMessage::setRequestUnauthorized(const HttpMessage* request,
                            const char* authenticationScheme,
                            const char* authenticationRealm,
                            const char* authenticationNonce,
                            const char* authenticationOpaque,
                            const char* authenticationDomain)
{
    setResponseFirstHeaderLine("HTTP/1.1", HTTP_UNAUTHORIZED_CODE, HTTP_UNAUTHORIZED_TEXT);
    setAuthenticationData(authenticationScheme, authenticationRealm,
                          authenticationNonce, authenticationOpaque,
                          authenticationDomain);
}

void HttpMessage::setBasicAuthorization(const char* user, const char* password,
                                        int authorizationEntity)
{
    UtlString fieldValue(HTTP_BASIC_AUTHENTICATION);
    UtlString encodedToken;

    // Create the encoded user:password cookie
    buildBasicAuthorizationCookie(user, password, &encodedToken);

    //  Add the cookie to the field
    fieldValue.append(' ');
    fieldValue.append(encodedToken.data());

    // Set the header field
    if(authorizationEntity == SERVER)
    {
        setHeaderValue(HTTP_AUTHORIZATION_FIELD, fieldValue.data(), 0);
    }
    else if(authorizationEntity == PROXY)
    {
        setHeaderValue(HTTP_PROXY_AUTHORIZATION_FIELD, fieldValue.data(), 0);
    }
}

UtlBoolean HttpMessage::getAuthorizationScheme(UtlString* scheme) const
{
    UtlString fieldValue;
    UtlBoolean fieldSet = getAuthorizationField(&fieldValue, SERVER);

    NameValueTokenizer::getSubField(fieldValue.data(), 0, " \t",
        scheme);
    scheme->toUpper();
    return(fieldSet);
}

UtlBoolean HttpMessage::getBasicAuthorizationData(UtlString* encodedCookie) const
{
    UtlString fieldValue;
    UtlBoolean fieldSet = getAuthorizationField(&fieldValue, SERVER);
    UtlString scheme;

    NameValueTokenizer::getSubField(fieldValue.data(), 0, " \t",
        &scheme);
    scheme.toUpper();

    // If the scheme is not basic, then the second token is probably not a cookie
    if(scheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
    {
        NameValueTokenizer::getSubField(fieldValue.data(), 1, " \t",
            encodedCookie);
    }
    else
    {
        encodedCookie->remove(0);
    }

    return(fieldSet);
}

UtlBoolean HttpMessage::getBasicAuthorizationData(UtlString* userId,
                                                UtlString* password) const
{
    UtlString cookie;
    UtlBoolean cookieFound = getBasicAuthorizationData(&cookie);
    userId->remove(0);
    password->remove(0);

    if(cookieFound)
    {
#ifdef TEST
        osPrintf("HttpMessage::getBasicAuthorizationData cookie: \"%s\"\n",
            cookie.data());
#endif
        int decodedLength = NetBase64Codec::decodedSize(cookie.length(), cookie.data());
        char* decodedCookie = new char[decodedLength + 1];
        NetBase64Codec::decode(cookie.length(), cookie.data(),
                                decodedLength, decodedCookie);

#ifdef TEST_PRINT
        decodedCookie[decodedLength] = 0;
        osPrintf("HttpMessage::getBasicAuthorizationData decoded cookie: \"%s\"\n",
            decodedCookie);
#endif
        // Parse out the userId and password
        int userPasswordSeparatorIndex = (int) strchr(decodedCookie, ':');
        if(userPasswordSeparatorIndex)
        {
            userPasswordSeparatorIndex -= (int) decodedCookie;

            userId->append(decodedCookie, userPasswordSeparatorIndex);
            password->append(&decodedCookie[userPasswordSeparatorIndex + 1],
                            decodedLength - (userPasswordSeparatorIndex + 1));
#ifdef TEST
            osPrintf("HttpMessage::getBasicAuthorizationData user/password separator index: %d\n",
                userPasswordSeparatorIndex);
            osPrintf("HttpMessage::getBasicAuthorizationData user: \"%s\" password: \"%s\"\n",
                userId->data(), password->data());
#endif
        }
        // No separator, assume the whole thing is a user Id
        else
        {
#ifdef TEST_PRINT
            osPrintf("HttpMessage::getBasicAuthorizationData no user/password separator found\n");
#endif
            userId->append(decodedCookie, decodedLength);
        }

        delete[] decodedCookie;
        decodedCookie = NULL;
    }
#ifdef TEST
    else
    {
        osPrintf("HttpMessage::getBasicAuthorizationData cookie not found in message\n");
    }
#endif

    return(cookieFound);
}

UtlBoolean HttpMessage::verifyBasicAuthorization(const char* user,
                                                const char* password) const
{
    UtlString referenceCookie;
    UtlString givenCookie;
    UtlBoolean userAllowed = TRUE;

    if(user == NULL || strcmp(user, "") == 0)
    {
#ifdef TEST
        osPrintf("HttpMessage::verifyBasicAuthorization no db user id given\n");
#endif
        userAllowed = FALSE;
    }
#ifdef TEST
    else
    {
        osPrintf("HttpMessage::verifyBasicAuthorization user: \"%s\" password: \"%s\"\n",
            user, password);
    }
#endif

    if(userAllowed)
    {
        // Create the reference encoded user:password cookie
        buildBasicAuthorizationCookie(user, password, &referenceCookie);

        // Get the user:password cookie provided in this message
        userAllowed = getBasicAuthorizationData(&givenCookie);
#ifdef TEST
        osPrintf("HttpMessage::verifyBasicAuthorization user: \"%s\" password: \"%s\"\n",
            user, password);
        osPrintf("HttpMessage::verifyBasicAuthorization ref. cookie: \"%s\"\n",
            referenceCookie.data());
        osPrintf("HttpMessage::verifyBasicAuthorization msg. cookie: \"%s\"\n",
            givenCookie.data());
#endif
    }

    if(userAllowed)
    {
        //  If the cookies match, allow this message
        userAllowed = (referenceCookie.compareTo(givenCookie.data()) == 0);
    }
#ifdef TEST
    else
    {
        osPrintf("HttpMessage::verifyBasicAuthorization no cookie in authorization field\n");
    }
#endif

    return(userAllowed);
}

/* ============================ INQUIRY =================================== */
UtlBoolean HttpMessage::isWholeMessage(const char* messageBuffer,
                              int bufferLength,
                              int& numberBytesChecked,
                              int& contentLength)
{
    return(FALSE);
}

UtlBoolean HttpMessage::isFirstSend() const
{
    return(!mFirstSent);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
