//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <string.h>
#include <ctype.h>
#include <stdint.h>
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
const UtlContainableType HttpMessage::TYPE = "HttpMessage";
const unsigned int HTTP_READ_TIMEOUT_MSECS = 30000;
const unsigned int HTTP_MIN_DELAY_MSECS = 20;
const int MAX_UDP_MESSAGE = 65536;

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
HttpMessage::HttpMessage(const char* messageBytes, ssize_t byteCount)
   : mHeaderCacheClean(FALSE)
   , body(NULL)
   , mUseChunkedEncoding(false)
   , transportTimeStamp(0)
   , lastResendInterval(0)
   , transportProtocol(OsSocket::UNKNOWN)
   , timesSent(0)
   , mFirstSent(FALSE)
   , mSendPort(PORT_NONE)
   , mpResponseListenerQueue(NULL)
   , mResponseListenerData(NULL)
{
   smHttpMessageCount++;

#ifdef HTTP_TIMELOG
   mTimeLog.addEvent("CREATED");
#endif

   parseMessage(messageBytes, byteCount);
}

HttpMessage::HttpMessage(OsSocket* inSocket, ssize_t bufferSize)
   : mHeaderCacheClean(FALSE)
   , body(NULL)
   , mUseChunkedEncoding(false)
   , transportTimeStamp(0)
   , lastResendInterval(0)
   , transportProtocol(OsSocket::UNKNOWN)
   , timesSent(0)
   , mFirstSent(FALSE)
   , mSendPort(PORT_NONE)
   , mpResponseListenerQueue(NULL)
   , mResponseListenerData(NULL)
{
   smHttpMessageCount++;

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
    mUseChunkedEncoding = rHttpMessage.mUseChunkedEncoding;
    body = NULL;
    if(rHttpMessage.body)
    {
        body = rHttpMessage.body->copy();
    }
    //nameValues = new UtlHashBag(100);
    transportTimeStamp = rHttpMessage.transportTimeStamp;
    lastResendInterval = rHttpMessage.lastResendInterval;
    transportProtocol = rHttpMessage.transportProtocol;
    timesSent = rHttpMessage.timesSent;
    mFirstSent = rHttpMessage.mFirstSent;
    mSendPort = rHttpMessage.mSendPort;
    mpResponseListenerQueue = rHttpMessage.mpResponseListenerQueue;
    mResponseListenerData = rHttpMessage.mResponseListenerData;

    rHttpMessage.mNameValues.copyTo<NameValuePair>( mNameValues );

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

UtlContainableType HttpMessage::getContainableType(void) const
{
   return HttpMessage::TYPE;
};

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
       lastResendInterval = rHttpMessage.lastResendInterval;
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

    ssize_t pathSeparatorIndex;
    while((pathSeparatorIndex = uriString.first('/')) >= 0)
    {
        uriString.replace(pathSeparatorIndex, 1, "\\");
    }

#endif
    platformFilePath = uriString;
}*/

ssize_t HttpMessage::parseFirstLine(const char* messageBytesPtr, ssize_t byteCount)
{
   mHeaderCacheClean = FALSE;
   mFirstHeaderLine = OsUtil::NULL_OS_STRING;
   ssize_t bytesConsumed = 0;

    // Read the first header line
   ssize_t nextLineOffset;
        ssize_t headerLineLength =
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

// Parse message out of messageBytes
void HttpMessage::parseMessage(const char* messageBytes, ssize_t byteCount)
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
      ssize_t bytesConsumed = 0;
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

void HttpMessage::parseBody(const char* messageBytesPtr, ssize_t bodyLength)
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

ssize_t HttpMessage::findHeaderEnd(const char* headerBytes, ssize_t messageLength)
{
    ssize_t lineLength = 0;
    ssize_t nextLineIndex = 0;
    ssize_t bytesConsumed = 0;
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
        bytesConsumed = HTTP_NOT_FOUND;

    return(bytesConsumed);
}

ssize_t HttpMessage::parseHeaders(const char* headerBytes, ssize_t messageLength,
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

int HttpMessage::get/*[3]*/(Url& httpUrl,
                            int  maxWaitMilliSeconds,
                            bool bPersistent)
{
    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get[3] httpUrl = '%s'",
                  httpUrl.toString().data());

    HttpMessage request;
    UtlString uriString;

    httpUrl.getPath(uriString, TRUE);

    request.setRequestFirstHeaderLine(HTTP_GET_METHOD,
                                      uriString,
                                      HTTP_PROTOCOL_VERSION);
    return(get(httpUrl, request, maxWaitMilliSeconds, bPersistent));
}

OsStatus HttpMessage::get/*[5]*/(Url& httpUrl,
                                 int iMaxWaitMilliSeconds,
                                 GetDataCallbackProc pCallbackProc,
                                 void* pOptionalData,
                                 OsConnectionSocket** socket)
{
   OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get[5] httpUrl = '%s'",
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

   if (!portIsValid(httpPort))
   {
      httpPort = (httpUrl.getScheme() == Url::HttpsUrlScheme) ? 443 : 80;

      hostPort.append(":");
      char tmpportbuf[10];
      sprintf(tmpportbuf,"%d",httpPort);
      hostPort += tmpportbuf;
   }

   request.addHeaderField("Host", hostPort.data());
   request.addHeaderField("Accept", "*/*");
   OsConnectionSocket *httpSocket = NULL;
   int connected = 0;

   int tries = 0;
   int exp = 1;
   while (tries++ < HttpMessageRetries)
   {
      if (httpUrl.getScheme() == Url::HttpsUrlScheme)
      {
#ifdef HAVE_SSL
         httpSocket = (OsConnectionSocket *)new OsSSLConnectionSocket(httpPort, httpHost, iMaxWaitMilliSeconds/1000);
#else /* ! HAVE_SSL */
         // SSL is not configured in, so we cannot do https: gets.
         OsSysLog::add(FAC_HTTP, PRI_CRIT,
                       "HttpMessage::get[5] SSL not configured; "
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
            OsSysLog::add(FAC_HTTP, PRI_ERR,
                          "HttpMessage::get[5] socket connection to %s:%d failed, try again %d ...",
                          httpHost.data(), httpPort, tries);
            delete httpSocket;
            httpSocket = 0;
            OsTask::delay(HTTP_MIN_DELAY_MSECS*exp);
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
      OsSysLog::add(FAC_HTTP, PRI_ERR,
                    "HttpMessage::get[5] socket connection to %s:%d failed, give up...",
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
         ssize_t iHeaderLength = parseFirstLine(buffer.data(), iRead) ;
         parseHeaders(&buffer.data()[iHeaderLength], iRead-iHeaderLength, mNameValues) ;

         ssize_t iContentLength = getContentLength() ;
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

int HttpMessage::get/*[4]*/(Url& httpUrl,
                            HttpMessage& request,
                            int maxWaitMilliSeconds,
                            bool bPersistent)
{
    int httpStatus = -1; // indicates a connection error if unchanged.
    if (OsSysLog::willLog(FAC_HTTP, PRI_DEBUG))
    {
       UtlString url;
       httpUrl.toString(url);
       OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                     "HttpMessage::get[4](httpUrl = '%s' maxwait = %d %s)",
                     url.data(),
                     maxWaitMilliSeconds,
                     bPersistent ? "PERSISTENT" : "NOT PERSISTENT");
    }

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

    // preserve these fields if they are already set
    if (request.getHeaderValue(0, HTTP_HOST_FIELD) == NULL)
    {
        UtlString hostPort(httpHost);
        httpPort = httpUrl.getHostPort();
        if (httpPort == PORT_NONE)
        {
           httpPort = (httpUrl.getScheme() == Url::HttpsUrlScheme) ? 443 : 80;

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
    bool connected = false;

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
    bool responseReceived = false;
    while (sendTries < HttpMessageRetries && ! responseReceived && 0 == bytesRead)
    {
        if (httpSocket == NULL)
        {
            // httpSocket is seen to be NULL, so we have no socket.
            connected = false;
            int tries = 0;
            int exp = 1;
            while (!connected && (tries++ < HttpMessageRetries))
            {
               if (Url::HttpsUrlScheme == httpUrl.getScheme())
               {
#                 ifdef HAVE_SSL
                  httpSocket = dynamic_cast<OsConnectionSocket*>
                     (new OsSSLConnectionSocket(httpPort, httpHost,
                                                maxWaitMilliSeconds/OsTime::MSECS_PER_SEC));
#                 else /* ! HAVE_SSL */
                  // SSL is not configured in, so we cannot do https: gets.
                  OsSysLog::add(FAC_HTTP, PRI_CRIT,
                                "HttpMessage::get[4] SSL not configured; "
                                "cannot get URL '%s'", httpUrl.toString().data());
                  httpSocket = NULL;
#                 endif /* HAVE_SSL */
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
                     int retryTime = HTTP_MIN_DELAY_MSECS*exp;
                     OsSysLog::add(FAC_HTTP, PRI_ERR,
                                   "HttpMessage::get[4]"
                                   " socket to %s:%d not connected, retry %d after %dms",
                                   httpHost.data(), httpPort, tries, retryTime);
                     delete httpSocket;
                     httpSocket = 0;
                     OsTask::delay(retryTime);
                     exp = exp*2;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_HTTP, PRI_ERR,
                                "HttpMessage::get[4]"
                                " socket creation failed to %s:%d",
                                httpHost.data(), httpPort
                                );
               }
            } // while (!connected && (tries++ < HttpMessageRetries))

            // If we created a new connection and are persistent
            // then remember the socket in the map and mark connection as being used
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
           OsSysLog::add(FAC_HTTP, PRI_ERR,
                         "HttpMessage::get[4] socket connection to %s:%d failed, give up...\n",
                         httpHost.data(), httpPort);
           if (pConnectionMap)
           {
               // Release lock on persistent connection
               pConnectionMapEntry->mLock.release();
           }
           return httpStatus;
        }

        // Send the request - most of the time returns 1 for some reason, 0 indicates problem
        if (httpSocket && httpSocket->isReadyToWrite(maxWaitMilliSeconds))
        {
            bytesSent = request.write(httpSocket);
            OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get[4] sent request");
        }

        if (bytesSent <= 0)
        {
           OsSysLog::add(FAC_HTTP, PRI_WARNING,
                         "HttpMessage::get[4] "
                         "failed sending on try %d",
                         sendTries);

            if (pConnectionMap)
            {
               // No bytes were sent .. if this is a persistent connection and it failed on retry
               // mark it unused in the connection map. Set socket to NULL
               if (sendTries == HttpMessageRetries-1)
               {
                  pConnectionMapEntry->mbInUse = false;
               }
               if (httpSocket)
               {
                  // Close socket to avoid timeouts in subsequent calls
                  OsSysLog::add(FAC_HTTP, PRI_ERR,
                                "HttpMessage::get[4] "
                                "closing failed socket after %d tries",
                                sendTries);
                  httpSocket->close();
                  delete httpSocket;
                  pConnectionMapEntry->mpSocket = NULL;
                  httpSocket = NULL;
                  connected = false;
               }
            }
        }
        else // request was sent
        {
           if (httpSocket && httpSocket->isReadyToRead(maxWaitMilliSeconds))
           {
              bytesRead = read(httpSocket); // consumes bytes until full message is read
              OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                            "HttpMessage::get[4] read returned %d bytes",
                            bytesRead);
              if (!bytesRead)
              {
                 // Close a non-persistent connection
                 if (!pConnectionMap)
                 {
                    OsSysLog::add(FAC_HTTP, PRI_ERR, "HttpMessage::get[4] read failed - closing");
                    httpSocket->close();
                 }
                 else
                 {
                    // persistent connection
                    // No bytes were read .. if this is a persistent connection
                    // and it failed on retry mark it unused
                    // in the connection map. Set socket to NULL
                    OsSysLog::add(FAC_HTTP, PRI_ERR,
                                  "HttpMessage::get[4] "
                                  "Receiving failed on persistent connection on try %d",
                                  sendTries);
                    if (sendTries == HttpMessageRetries-1)
                    {
                       pConnectionMapEntry->mbInUse = false;
                    }
                    if (httpSocket)
                    {
                       httpSocket->close();
                       delete httpSocket;
                       pConnectionMapEntry->mpSocket = NULL;
                       httpSocket = NULL;
                       connected = false;
                    }
                 }
              }
           }
           else
           {
              // Either the socket closed or the read timed out
              if (httpSocket)
              {
                 /*
                  * If it timed out, close it.  This is required because otherwise the
                  * response may eventually show up and be associated with some other request.
                  * There is no way to recover the HTTP request/response framing.
                  */
                 OsSysLog::add(FAC_HTTP, PRI_ERR, "HttpMessage::get[4] read timed out - closing");
                 httpSocket->close();
                 delete httpSocket;
                 if (pConnectionMapEntry)
                 {
                    pConnectionMapEntry->mpSocket = NULL;
                 }
                 httpSocket = NULL;
                 connected = false;
              }

              responseReceived = true; // to get out of the loop and bail
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
        HttpEndpointEnum authEntity = SERVER;

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

                int tries = 0;
                int connected = 0;
                int exp = 1;
                while (!connected && (tries++ < 6))
                {
                   if (Url::HttpsUrlScheme == httpUrl.getScheme())
                   {
#                     ifdef HAVE_SSL
                      httpAuthSocket = (OsConnectionSocket *)
                         new OsSSLConnectionSocket(httpPort, httpHost, maxWaitMilliSeconds/1000);
#                     else /* ! HAVE_SSL */
                      // SSL is not configured in, so we cannot do https: gets.
                      OsSysLog::add(FAC_HTTP, PRI_CRIT,
                                    "HttpMessage::get[4] basic auth SSL not configured; "
                                    "cannot get URL '%s'", httpUrl.toString().data());
                      httpAuthSocket = NULL;
#                     endif /* HAVE_SSL */
                   }
                   else
                   {
                      httpAuthSocket = new OsConnectionSocket(httpPort, httpHost);
                   }

                   if (httpAuthSocket)
                   {
                      connected = httpAuthSocket->isConnected();
                      if (!connected)
                      {
                         OsSysLog::add(FAC_HTTP, PRI_ERR,
                                       "HttpMessage::get[4] basic auth "
                                       "socket connection to %s:%d failed, try again %d ...\n",
                                       httpHost.data(), httpPort, tries);
                         delete httpAuthSocket;
                         httpAuthSocket = 0;
                         OsTask::delay(HTTP_MIN_DELAY_MSECS*exp);
                         exp = exp*2;
                      }
                   }
                } // while (!connected && (tries++ < 6))

                if (!connected)
                {
                   OsSysLog::add(FAC_HTTP, PRI_ERR,
                                 "HttpMessage::get[4] basic auth "
                                 "socket connection to %s:%d failed, give up...\n",
                                 httpHost.data(), httpPort);
                   return httpStatus;
                }

                // Sent the request again
                if (httpAuthSocket->isReadyToWrite(maxWaitMilliSeconds))
                {
                   bytesSent = request.write(httpAuthSocket);
                }
                bytesRead = 0;

                // Clear out the data in the previous response
                mHeaderCacheClean = FALSE;
                mNameValues.destroyAll();
                if(body)
                {
                   delete body;
                   body = NULL;
                }

                // Wait for the response
                if(   bytesSent > 0
                   && httpAuthSocket->isReadyToRead(maxWaitMilliSeconds))
                {
                   bytesRead = read(httpAuthSocket); // consumes full msg

                    httpAuthSocket->close();
                }

                // Get the response
                if(bytesRead > 0)
                {
                    httpStatus = getResponseStatusCode();
                }

                if (httpAuthSocket)
                {
                   delete httpAuthSocket;
                }

            } // end if auth. retry

        } // End if Basic Auth.

    }  // End if first response was returned

    if (httpSocket && !bPersistent)
    {
       OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get[4] closing non-persistent connection");
       delete httpSocket;
       httpSocket = 0;
    }
    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::get[4] returning %d response", httpStatus);
    return(httpStatus);
}

int HttpMessage::readHeader(OsSocket* inSocket, UtlString& buffer)
{
   char      ch ;
   int       iBytesRead = 0 ;
   ssize_t   iRead;
   OsSocket::IpProtocolSocketType socketType = inSocket->getIpProtocol();
   UtlString  remoteHost;
   int        remotePort;
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
   ssize_t      iRead;
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
         ssize_t iMaxRead = MIN(sizeof(buffer), (size_t) (iLength - iBytesRead)) ;
         iRead = inSocket->read(buffer, iMaxRead, &remoteHost, &remotePort);
         if (iRead > 0)
         {
            iBytesRead += iRead ;

            UtlBoolean bRC = (*pCallbackProc)(buffer, iRead, pOptionalData, this) ;
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




int HttpMessage::read(OsSocket* inSocket, ssize_t bufferSize,
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
   UtlString* allBytes = externalBuffer ? externalBuffer : &localBuffer;
#  ifdef MSG_DEBUG
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "HttpMessage::read %zu initial residual bytes: '%s'",
                 allBytes->length(), allBytes->data());
#  endif

   ssize_t residualBytes = allBytes->length(); // passed into this read already in the buffer
   int returnMessageLength = 0;

   // Attempt to minimize the number of times that the string gets
   // re-allocated and copied by setting the initial capacity to the
   // requested approximate message size, bufferSize.
   ssize_t byteCapacity = allBytes->capacity(bufferSize);
   if (byteCapacity >= bufferSize)
   {
      // Reallocating allBytes was successful.

      // Get information about the socket.
      UtlString remoteHost;
      int remotePort;

      OsSocket::IpProtocolSocketType socketType = inSocket->getIpProtocol();
      setSendProtocol(socketType);

#     ifdef TEST
      OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpMessage::read socket: %p "
                    "getIpProtocol: %d remoteHost: %s remotePort: %d\n",
                    inSocket, socketType,
                    remoteHost.data(), remotePort);
#     endif

      // Initialize control variables.
      char buffer[bufferSize];  // fixed buffer to read into from the socket
      ssize_t bytesTotal = 0;       // total bytes accumulated

      // The byte offset of the end of the header.  -1 means the end
      // has not yet been seen.
      ssize_t headerEnd = HTTP_NOT_FOUND;

      // The length of the content.  -1 means the end is not yet known.
      int contentLength = -1;

      // Assume content-type is set until we read all of the headers.
      UtlBoolean contentTypeSet = TRUE;
      UtlString contentType;

      //
      // Read the HTTP message.
      //
      bool finished = false;
      ssize_t bytesRead = 0;
      while (   ! finished
             && (   residualBytes > 0 // there are bytes already in the buffer - no need to read
                 || (   inSocket->isOk()
                     && (   OsSocket::isFramed(socketType)
                         || inSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS)
                         )
                     && (bytesRead = inSocket->read(buffer, bufferSize,
                                                    &remoteHost, &remotePort)
                         ) > 0
                     )
                 )
             )
      {
         if (residualBytes)
         {
#           ifdef MSG_DEBUG
            OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                          "HttpMessage::read %zu residual bytes: '%.*s'",
                          residualBytes, (int)residualBytes, buffer);
#           endif

            // set the variables as though this had been read from the socket.
            bytesRead = residualBytes;
            residualBytes = 0;

            if (mSendAddress.isNull())
            {
               // There can only be residual buffer data if this is TCP, so we know
               // that the socket has the remote party data cached.
               inSocket->getRemoteHostIp(&remoteHost, &remotePort);
               setSendAddress(remoteHost.data(), remotePort);
            }
         }
         else // we must have read bytes from the socket
         {
#           ifdef MSG_DEBUG
            OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                          "HttpMessage::read %zu bytes read: '%.*s'",
                          bytesRead, (int)bytesRead, buffer);
#           endif


            allBytes->append(buffer, bytesRead); // move from temporary buffer into UtlString
            if ( bytesRead == bufferSize )
            {
               // we filled the buffer in the one read. We need to empty the socket buffer completely so read
               // again until we get nothing back or the buffer is not completely filled.
               int morebytesRead = bytesRead;
               while ( morebytesRead == bufferSize )
               {
                 morebytesRead = inSocket->read(buffer, bufferSize,
                                           &remoteHost, &remotePort);
                 if ( morebytesRead > 0 )
                 {
                    allBytes->append(buffer, morebytesRead); // move from temporary buffer into UtlString
                    bytesRead += morebytesRead;
                 }
               }
            }


            if (mSendAddress.isNull())
            {
               setSendAddress(remoteHost.data(), remotePort);
            }
         }

         bytesTotal += bytesRead;
         bytesRead = 0;

         if (bytesTotal > 0)
         {
            // If we have not yet found the end of the headers
            if (headerEnd < 0)
            {
               headerEnd = findHeaderEnd(allBytes->data(), bytesTotal);

               // UDP and Multicast UDP you can only do one read
               // The fragmentation is handled at the socket layer
               // If we did not get it all we are not going to get any more
               if (headerEnd <= 0 && OsSocket::isFramed(socketType))
               {
                  headerEnd = bytesTotal;
               }

               // We found the end of all the headers.  Parse them.
               if (headerEnd > 0)
               {
                  // Parse the first line
                  ssize_t endOfFirstLine = parseFirstLine(allBytes->data(),
                                                      headerEnd);
                  // Parse all of the headers
                  parseHeaders(&(allBytes->data()[endOfFirstLine]),
                               headerEnd - endOfFirstLine,
                               mNameValues);

                  // Get the content length
                  {
                     const char* value;
                     if ((value = getHeaderValue(0, HTTP_CONTENT_LENGTH_FIELD)))
                     {
                        contentLength = atoi(value);
                     }
                     else if ((value = getHeaderValue(0, SIP_SHORT_CONTENT_LENGTH_FIELD)))
                     {
                        contentLength = atoi(value);
                     }
                     else
                     {
                        // We've seen the end of the headers and no Content-Length
                        // header was found.  Although sloppy, it is valid condition
                        // in most cases.  Produce a log entry and assume a content
                        // length of 0.
                        OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                                      "HttpMessage::read "
                                      "no content-length set on %sframed %s socket",
                                      OsSocket::isFramed(socketType) ? "" : "un",
                                      OsSocket::ipProtocolString(socketType));

                        contentLength = 0;
                     }
                  }

                  // Get the content type
                  contentTypeSet = getContentType(&contentType);

#                 ifdef TEST
                  OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                                "HttpMessage::read  "
                                "contentLength %d, contentTypeSet %d, "
                                "contentType '%s'",
                                contentLength, contentTypeSet,
                                contentType.data());
#                 endif

                  // If this is UDP we know that the message has
                  // a maximum size of 64K
                  if (   socketType == OsSocket::UDP
                      && contentLength > MAX_UDP_MESSAGE)
                  {
                     OsSysLog::add(FAC_HTTP, PRI_ERR,
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
                     OsSysLog::add(FAC_HTTP, PRI_ERR,
                                   "HttpMessage::read "
                                   "Content-Length(%d) > maxContentLength(%d): "
                                   "closing socket type: %d to %s:%d",
                                   contentLength, maxContentLength,
                                   socketType, remoteHost.data(), remotePort);
                     // Shut it all down, because it may be an abusive sender.
                     inSocket->close();
                     allBytes->remove(0);
                     bytesTotal = 0;
                     finished = true;
                  }

                  // If a content length is set adjust the capacity
                  // to minimize the number of resizing and memory
                  // shuffling operations
                  else if (contentLength > 0)
                  {
                     byteCapacity = headerEnd + contentLength + 100;
                     allBytes->capacity(byteCapacity);
                  }
               }
            }

            // analyze whether or not we've got the whole message.
            if ( !finished )
            {
               if( headerEnd > 0 ) // we have seen the end of the headers
               {
                  if (OsSocket::isFramed(socketType))
                  {
                     if (contentLength + headerEnd <= (int)allBytes->length())
                     {
                        finished = true;
                     }
                     else
                     {
                        // we got a content length on a framed socket,
                        // but there is not that much data in the message
                        OsSysLog::add(FAC_HTTP, PRI_WARNING,
                                      "HttpMessage::read "
                                      "truncated body on framed %s socket",
                                      OsSocket::ipProtocolString(socketType));
                        // @TODO - I think we want to pass this up so that it can return
                        // a bad request error, but need to check up the stack.
                        finished = true;
                     }
                  }
                  else
                  {
					// seen the end of the headers, on an unframed socket
                     if (contentLength + headerEnd <= (int)allBytes->length())
                     {
                        // we got all of the body, so we're done
                        finished = true;
                     }
                     else
                     {
                        // there is more to read on the stream, so let the loop iterate
                     }
                  }
               }
               else
               {
                  // We have not yet seen the end of the headers
                  if (OsSocket::isFramed(socketType))
                  {
                     // framed transport, but no header end seen yet - probably not good
                     // there was suppose to be a body, but we never saw it
                     if (contentLength > 0)
                     {
                        // The headers we got said there was a body, but we didn't find
                        // the end of the headers, so this is truncated.
                        OsSysLog::add(FAC_HTTP, PRI_ERR,
                                      "HttpMessage::read "
                                      "truncated message on framed %s socket:\n%s",
                                      OsSocket::ipProtocolString(socketType),
                                      allBytes->data()
                                      );
                        // message is not good, but pass it up so we can return Bad Request
                        finished = true;
                     }
                     else
                     {
                        // unterminated headers on a framed socket, with a content length of 0
                        // probably just forgot the blank line to terminate the headers so allow it.
                        OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                                      "HttpMessage::read "
                                      "unterminated headers with content-length 0 on %s socket",
                                      OsSocket::ipProtocolString(socketType));
                        finished = true;
                     }
                  }
                  else
                  {
                     // We have not yet seen the end of the headers on an unframed socket,
                     // so keep reading...
                  }
               }
            }
         }

         // Read more of the message and continue processing it.
      }
#     ifdef MSG_DEBUG
      OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                    "HttpMessage::read Reached end of reading");
#     endif /* MSG_DEBUG */

      //
      // We have reached one of the conditions that indicates to stop
      // reading the HTTP message.  Decide whether we have finished
      // reading successfully.
      //

      // Calculate the end of the message & body length
      size_t bodyLength = 0;
      size_t messageLength = 0;
      if ( headerEnd > 0 )
      {
         if( OsSocket::isFramed(socketType) )
         {
            messageLength = allBytes->length();
            bodyLength = messageLength - headerEnd;
         }
         else // !OsSocket::isFramed(socketType)
         {
            //only if the total bytes read is as expected
            //ie equal or greater than (contentLength + headerEnd)
            if (bytesTotal - headerEnd >=  contentLength)
            {
               // We have the entire expected length of the message.
               bodyLength = contentLength;
               messageLength = headerEnd + contentLength;

               OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                             "HttpMessage::read full msg rcvd bytes %zu: header: %zu content: %d",
                             bytesTotal, headerEnd, contentLength);

               // There are residual bytes for the next message
               if (messageLength < allBytes->length())
               {
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
               }
            }
            else
            {
               // We do not have the entire expected length of the message.
               bodyLength = bytesTotal - headerEnd;
#              ifdef MSG_DEBUG
               // At this point, the entire message should have been read
               // (in multiple reads if necessary).
               OsSysLog::add(FAC_HTTP, PRI_WARNING,
                             "HttpMessage::read Not all content data "
                             "successfully read: received %zu body bytes but "
                             "Content-Length was %d",
                             bodyLength, contentLength);
#              endif /* MSG_DEBUG */
            }
         }
      }
      else if (allBytes->length() > 0)
      {
         // We have not found the end of headers

#        ifdef MSG_DEBUG
         // This should not happen because the message will have been
         // fetched with multiple reads if necessary.
         OsSysLog::add(FAC_HTTP, PRI_ERR,
                       "HttpMessage::read End of headers not found.  "
                       "%zu bytes read.  Content:\n>>>%.*s<<<\n",
                       allBytes->length(), (int)allBytes->length(),
                       allBytes->data());
#        endif /* MSG_DEBUG */
      }

      // If there is a body.
      if (headerEnd > 0 && bodyLength)
      {
         // All that is left is the body to deal with
         parseBody(&(allBytes->data()[headerEnd]), bodyLength);
      }

      returnMessageLength = messageLength;
   }
   else
   {
      // Attempt to resize the input buffer to the requested
      // approximate size (allBytes->capacity(bufferSize)) failed, so
      // return an error.
      OsSysLog::add(FAC_HTTP, PRI_ERR,
                    "HttpMessage::read allBytes->capacity(%zu) failed, "
                    "returning %zu",
                    bufferSize, byteCapacity);
      returnMessageLength = 0;
   }

#  ifdef MSG_DEBUG
   UtlString b;
   ssize_t l;
   getBytes(&b, &l);
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "HttpMessage::read returning %d '%s'",
                 returnMessageLength, b.data());
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "HttpMessage::read %zu final residual bytes: '%s'",
                 allBytes->length(), allBytes->data());
#  endif

   return(returnMessageLength);
}

UtlBoolean HttpMessage::write(OsSocket* outSocket) const
{
        UtlString buffer;
        ssize_t bufferLen;
        ssize_t bytesWritten;

        getBytes(&buffer, &bufferLen);
        bytesWritten = outSocket->write(buffer.data(), bufferLen);
        return(bytesWritten == bufferLen);
}

/// Write just the headers and the terminating CRLF to outSocket
UtlBoolean HttpMessage::writeHeaders(OsSocket* outSocket) const
{
   /*
    * This method must only be used if the message has been marked as using
    * chunked encoding (by calling useChunkedBody).
    *
    * The chunks for the body should then be written by calling
    * writeChunk, followed by a call to writeEndChunks when all
    * chunks have been written.
    */
   UtlBoolean writtenOk = FALSE;

   if (!mUseChunkedEncoding)
   {
      OsSysLog::add(FAC_HTTP, PRI_CRIT, "HttpMessage::writeHeaders used on a non-chunked message");
      assert(mUseChunkedEncoding);
   }

   UtlString buffer;
   ssize_t bufferLen;
   ssize_t bytesWritten;

   getBytes(&buffer, &bufferLen, false /* do not include the body */);
   OsSysLog::add(FAC_HTTP, PRI_INFO,
                 "HttpMessage::writeHeaders writing:\n%s",
                 buffer.data());

   bytesWritten = outSocket->write(buffer.data(), bufferLen);

   writtenOk = (bytesWritten == bufferLen);

   return writtenOk;
}

/// Mark this message as using chunked encoding to delimit the body @see writeHeaders
void HttpMessage::useChunkedBody(bool useChunked)
{
   mUseChunkedEncoding = useChunked;
   if (mUseChunkedEncoding)
   {
      if (body)
      {
         OsSysLog::add(FAC_HTTP, PRI_CRIT,
                       "HttpMessage::useChunkedBody "
                       "used on a message that has a body - existing body deleted");
         assert(body);
         delete body;
      }
      removeHeader(HTTP_CONTENT_LENGTH_FIELD, 0);
      setHeaderValue(HTTP_TRANSFER_ENCODING_FIELD, "chunked");
   }
   else
   {
      removeHeader(HTTP_TRANSFER_ENCODING_FIELD, 0);
   }
}

static const UtlString EndOfChunk(END_OF_LINE_DELIMITER);

/// Write a partial body using chunked encoding.
UtlBoolean HttpMessage::writeChunk(OsSocket* outSocket, const UtlString& chunk)
{
   UtlString chunkHeader;
   chunkHeader.appendNumber(chunk.length(), "%x");
   chunkHeader.append(END_OF_LINE_DELIMITER);

   ssize_t bytesWritten;
   bytesWritten  = outSocket->write(chunkHeader.data(), chunkHeader.length());
   bytesWritten += outSocket->write(chunk.data(),       chunk.length());
   bytesWritten += outSocket->write(EndOfChunk.data(),  EndOfChunk.length());

   ssize_t bytesExpected = chunkHeader.length() + chunk.length() + EndOfChunk.length();

   return (bytesWritten == bytesExpected);
}

static const UtlString EndOfChunks("0" END_OF_LINE_DELIMITER END_OF_LINE_DELIMITER);

/// Write the end-of-chunks marker to terminate a message sent with chunked encoding.
UtlBoolean HttpMessage::writeEndOfChunks(OsSocket* outSocket)
{
   ssize_t bytesWritten = outSocket->write(EndOfChunks.data(), EndOfChunks.length());
   ssize_t bytesExpected = EndOfChunks.length();
   return (bytesWritten == bytesExpected);
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
#ifdef TEST_PRINT_OS
                    osPrintf("HttpMessage::unescape char: %d\n", escapedChar);
#endif
                    //unescapedText.append((char) escapedChar);
                    resultPtr[numUnescapedChars] = (char) escapedChar;
                    numUnescapedChars++;
                }
                else
                {
#ifdef TEST_PRINT
                    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "Bogus dude: escaped char wo/ 2 hex digits: %s\n",
                        escapedText.data());
#endif
                    break;
                }
            }
            else
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_HTTP, PRI_DEBUG, "Bogus dude: escaped char wo/ 2 hex digits: %s\n",
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
#ifdef TEST_PRINT_OS
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
#ifdef TEST_PRINT_OS
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
    ssize_t len = token.length();
    UtlBoolean capNextChar = TRUE;
    const char* tokenPtr = token.data();
    char thisChar;
    for(ssize_t capCharIndex = 0; capCharIndex < len; capCharIndex++)
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

void HttpMessage::setResendInterval(int resendMSec)
{
        lastResendInterval = resendMSec;
}

int HttpMessage::getResendInterval() const
{
        return(lastResendInterval);
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
    if(address)
    {
       mSendAddress.append(address);
    }
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
    lastResendInterval = 0;
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
        NameValuePair* headerField = NULL;
        int fieldIndex = 0;

        UtlString headerFieldName(name ? name : "");
        if(name)
        {
           headerFieldName.toUpper();
        }

        // For each name value:
        while(fieldIndex <= index)
        {
                // Go to the next header field
                if(name)
                {

                   headerField = (NameValuePair*) iterator.findNext(&headerFieldName);
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
        useChunkedBody(false);
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
        *epochDate = (long)OsDateTime::convertHttpDateToEpoch(dateField);
        // returns zero if the format is not understood
        if(! *epochDate)
        {
#ifdef TEST_PRINT_OS
            osPrintf("WARNING: unsupported date format\n");
            osPrintf("Date field: \"%s\"\n", dateField);
            osPrintf("epoch date: %ld\n", *epochDate);
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

void HttpMessage::getBytes(UtlString* bufferString, ssize_t* length, bool includeBody) const
{
    *length = 0;
    UtlString name;
    const char* value;

    *bufferString = mFirstHeaderLine;

    bufferString->append(END_OF_LINE_DELIMITER);

    UtlDListIterator iterator((UtlDList&)mNameValues);
    NameValuePair* headerField;
    UtlBoolean foundContentLengthHeader = FALSE;
    ssize_t bodyLen = 0;
    UtlString bodyBytes;
    if(includeBody && body)
    {
       body->getBytes(&bodyBytes, &bodyLen);
    }

    ssize_t oldLen = 0 ;
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
            ssize_t fieldBodyLengthValue = atoi(value ? value : "");
            if(fieldBodyLengthValue != bodyLen)
            {
                char bodyLengthString[40];
                sprintf(bodyLengthString, "%zu", bodyLen);
                OsSysLog::add(FAC_HTTP, PRI_WARNING,
                              "HttpMessage::getBytes content-length: %s wrong setting to: %s",
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
                bufferString->append(END_OF_LINE_DELIMITER);
        }

        // Make sure the content length is set
        if(!foundContentLengthHeader && !mUseChunkedEncoding)
        {
                UtlString ContentLen(HTTP_CONTENT_LENGTH_FIELD);
                cannonizeToken(ContentLen);
                bufferString->append(ContentLen);
                bufferString->append(HTTP_NAME_VALUE_DELIMITER);
                char bodyLengthString[40];
                sprintf(bodyLengthString, " %zu", bodyLen);
                bufferString->append(bodyLengthString);
                bufferString->append(END_OF_LINE_DELIMITER);
        }

        bufferString->append(END_OF_LINE_DELIMITER);

        if(body)
        {
                bufferString->append(bodyBytes.data(), bodyLen);
        }

        *length = bufferString->length();
}

// Get a malloc'ed string containing the text of the message.
char* HttpMessage::getBytes() const
{
   UtlString buffer;
   ssize_t length;

   getBytes(&buffer, &length);
   char* ret = (char*) malloc(length + 1);
   assert(ret);
   memcpy(ret, buffer.data(), length + 1);

   return ret;
}

// Print the message to stdout.  (To be called from GDB.)
void HttpMessage::debugPrint() const
{
   UtlString buffer;
   ssize_t length;

   getBytes(&buffer, &length);
   printf("%s", buffer.data());
}

void HttpMessage::getFirstHeaderLinePart(ssize_t partIndex, UtlString* part, char separator) const
{
        const char* partStart = mFirstHeaderLine.data();
    // Tolerate separators in the begining
    while(*partStart == separator) partStart++;

        const char* partEnd;
        ssize_t index = 0;
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

                        ssize_t len = partEnd - partStart;
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
      OsSysLog::add(FAC_HTTP,
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
                                                HttpMessage::HttpEndpointEnum authorizationEntity
                                                ) const
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

void HttpMessage::setAuthenticateData(const char* scheme,
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
    authField.append(" "
                     HTTP_AUTHENTICATION_REALM_TOKEN
                     "=");
    if(realm)
    {
        authField.append('\"');
        authField.append(realm);
        authField.append('\"');
    }
    else
    {
        OsSysLog::add( FAC_HTTP, PRI_ERR,
                      "HttpMessage::setAuthenticateData: no realm specified"
                      );
    }

    if( 0 == schemeString.compareTo(HTTP_DIGEST_AUTHENTICATION,
                                    UtlString::ignoreCase
                                    )
       )
    {
        if(domain && *domain)
        {
            authField.append(", "
                             HTTP_AUTHENTICATION_DOMAIN_TOKEN
                             "=\"");
            authField.append(domain);
            authField.append("\"");
        }
        if(nonce && *nonce)
        {
            authField.append(", "
                             HTTP_AUTHENTICATION_NONCE_TOKEN
                             "=\"");
            authField.append(nonce);
            authField.append("\"");
        }
        if(opaque && *opaque)
        {
            authField.append(", "
                             HTTP_AUTHENTICATION_OPAQUE_TOKEN
                             "=\"");
            authField.append(opaque);
            authField.append("\"");
        }

        // Always ask for 'qop="auth"' in www or proxy authenticate header
        authField.append(", "
                         HTTP_AUTHENTICATION_QOP_TOKEN
                         "=\""
                         HTTP_QOP_AUTH
                         "\"");
    }

    addAuthenticateField(authField, authEntity);
}

UtlBoolean HttpMessage::getAuthenticateData(UtlString* scheme,
                                            UtlString* realm,
                                            UtlString* nonce,
                                            UtlString* opaque,
                                            UtlString* algorithm, // MD5 or MD5-sess
                                            UtlString* qop, // may be multiple values
                                            HttpMessage::HttpEndpointEnum authorizationEntity,
                                            unsigned   index
                                            ) const
{
    const char* fieldValue = NULL;
    UtlBoolean foundData = FALSE;

    if(authorizationEntity == SERVER)
    {
        fieldValue = getHeaderValue(index, HTTP_WWW_AUTHENTICATE_FIELD);
    }
    else if(authorizationEntity == PROXY)
    {
        fieldValue = getHeaderValue(index, HTTP_PROXY_AUTHENTICATE_FIELD);
    }

    if(fieldValue)
    {
       foundData = parseAuthenticateData(fieldValue,
                                           scheme, realm, nonce, opaque, algorithm, qop,
                                           NULL /* domain */);
    }

    return foundData;
}

bool HttpMessage::parseAuthenticateData(const UtlString& authenticateField,
                                        UtlString* scheme,
                                        UtlString* realm,
                                        UtlString* nonce,
                                        UtlString* opaque,
                                        UtlString* algorithm, // MD5 or MD5-sess
                                        UtlString* qop,
                                        UtlString* domain
                                        )
{
   NetAttributeTokenizer tokenizer(authenticateField.data());
   UtlString name;
   UtlString value;

   if(scheme) scheme->remove(0);
   if(realm) realm->remove(0);
   if(nonce) nonce->remove(0);
   if(opaque) opaque->remove(0);
   if(algorithm) algorithm->remove(0);
   if(qop) qop->remove(0);
   if(domain) domain->remove(0);

   if (scheme)
   {
      tokenizer.getNextAttribute(*scheme, value);
      cannonizeToken(*scheme);
   }
   else
   {
      UtlString ignoredScheme;
      tokenizer.getNextAttribute(ignoredScheme, value);
   }

   bool foundRealm = false;
   bool foundNonce = false;
   // Search for tokens independent of order
   while(tokenizer.getNextAttribute(name, value))
   {
      if(name.compareTo(HTTP_AUTHENTICATION_REALM_TOKEN, UtlString::ignoreCase) == 0)
      {
         if (realm && !foundRealm)
         {
            realm->append(value.data());
         }
         foundRealm = true;
      }
      else if(name.compareTo(HTTP_AUTHENTICATION_NONCE_TOKEN, UtlString::ignoreCase) == 0)
      {
         if (nonce && !foundNonce)
         {
            nonce->append(value.data());
         }
         foundNonce = true;
      }
      else if(opaque
              && name.compareTo(HTTP_AUTHENTICATION_OPAQUE_TOKEN, UtlString::ignoreCase) == 0)
      {
         opaque->append(value.data());
      }
      else if(algorithm
              && name.compareTo(HTTP_AUTHENTICATION_ALGORITHM_TOKEN, UtlString::ignoreCase) == 0)
      {
         algorithm->append(value.data());
      }
      else if(qop
              && name.compareTo(HTTP_AUTHENTICATION_QOP_TOKEN, UtlString::ignoreCase) == 0)
      {
         qop->append(value.data());
      }
      else if(domain
              && name.compareTo(HTTP_AUTHENTICATION_DOMAIN_TOKEN, UtlString::ignoreCase) == 0)
      {
         domain->append(value.data());
      }
   }

   return foundRealm && foundNonce;
}


UtlBoolean HttpMessage::getAuthorizationUser(UtlString* user,
                                             UtlString* userBase) const
{
   UtlBoolean foundUserId = FALSE;
   UtlString scheme;
   UtlString dummy;

   getAuthorizationScheme(&scheme);
#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "HttpMessage::getAuthorizationUser authorization scheme: \"%s\"",
                 scheme.data());
#endif

   // Basic
   if (scheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
   {
      foundUserId = getBasicAuthorizationData(user, &dummy);
      // Set *userBase to *user, as we do not process instrument indicators
      // in basic authentication.
      if (userBase)
      {
         *userBase = *user;
      }
#ifdef TEST_PRINT
      if (foundUserId)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "HttpMessage::getAuthorizationUser userId: \"%s\" from message",
                       user->data());
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "HttpMessage::getAuthorizationUser failed to get userId from message");
      }
#endif
   }

   // Digest
   else if (scheme.compareTo(HTTP_DIGEST_AUTHENTICATION, UtlString::ignoreCase) == 0)
   {
      getDigestAuthorizationData(user,
                                 NULL, // default most arguments
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,  // cnonce
                                 NULL,  // nonceCount
                                 NULL,  // qop
                                 HttpMessage::PROXY,
                                 0,
                                 userBase, // provide user base argument
                                 NULL);
   }

   // scheme not supported
   else
   {
      user->remove(0);
      userBase->remove(0);
   }

   return foundUserId;
}

UtlBoolean HttpMessage::getAuthorizationField(UtlString* authorizationField,
                                              HttpMessage::HttpEndpointEnum authorizationEntity
                                              ) const
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

    authorizationField->remove(0);

    if(fieldValue)
    {
        authorizationField->append(fieldValue);
    }

    return(fieldValue != NULL);
}

UtlBoolean HttpMessage::getDigestAuthorizationData(UtlString* user,
                                                   UtlString* realm,
                                                   UtlString* nonce,
                                                   UtlString* opaque,
                                                   UtlString* response,
                                                   UtlString* uri,
                                                   UtlString* cnonce,
                                                   UtlString* nonceCount,
                                                   UtlString* qop,
                                                   HttpMessage::HttpEndpointEnum authorizationEntity,
                                                   int index,
                                                   UtlString* user_base,
                                                   UtlString* instrument) const
{
   // Empty the output arguments.
   if (realm) realm->remove(0);
   if (nonce) nonce->remove(0);
   if (opaque) opaque->remove(0);
   if (user) user->remove(0);
   if (uri) uri->remove(0);
   if (response) response->remove(0);
   if (cnonce) cnonce->remove(0);
   if (nonceCount) nonceCount->remove(0);
   if (qop) qop->remove(0);
   if (user_base) user_base->remove(0);
   if (instrument) instrument->remove(0);

   const char* value = NULL;
   if (authorizationEntity == SERVER)
   {
      value = getHeaderValue(index, HTTP_AUTHORIZATION_FIELD);
   }
   else if (authorizationEntity == PROXY)
   {
      value = getHeaderValue(index, HTTP_PROXY_AUTHORIZATION_FIELD);
   }

   if (value)
   {
      NetAttributeTokenizer tokenizer(value);
      UtlString name;
      UtlString value;
      UtlString scheme;

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
               realm->append(value);
            }
            else if(nonce && name.compareTo(HTTP_AUTHENTICATION_NONCE_TOKEN, UtlString::ignoreCase) == 0)
            {
               nonce->append(value);
            }
            else if(cnonce && name.compareTo(HTTP_AUTHENTICATION_CNONCE_TOKEN, UtlString::ignoreCase) == 0)
            {
               cnonce->append(value);
            }
            else if(nonceCount && name.compareTo(HTTP_AUTHENTICATION_NONCE_COUNT_TOKEN, UtlString::ignoreCase) == 0)
            {
               nonceCount->append(value);
            }
            else if(qop && name.compareTo(HTTP_AUTHENTICATION_QOP_TOKEN, UtlString::ignoreCase) == 0)
            {
               qop->append(value);   // NOTE: value returned has been stripped of quotes
            }
            else if(opaque && name.compareTo(HTTP_AUTHENTICATION_OPAQUE_TOKEN, UtlString::ignoreCase) == 0)
            {
               opaque->append(value);
            }
            else if (user &&
                     name.compareTo(HTTP_AUTHENTICATION_USERNAME_TOKEN,
                                    UtlString::ignoreCase) == 0)
            {
               user->append(value);
               if (user_base || instrument)
               {
                  /** Separate the 'user' presented in an authorization
                   *  header into the real user part (user_base) and the
                   *  phone instrument identification part (instrument).
                   */

                  // Check to see if user contains '/'.  If so, find
                  // the last occurrence, because the instrument part,
                  // being a token, cannot contain '/'.
                  ssize_t index = value.last('/');
                  if (index != UTL_NOT_FOUND)
                  {
                     // Assign the first part to user_base and the
                     // last part to instrument.
                     if (user_base)
                     {
                        user_base->append(value, 0, index);
                     }
                     if (instrument)
                     {
                        instrument->append(value, index+1, value.length()-index-1);
                     }
                  }
                  else
                  {
                     // Assign the entire value to user_base.
                     if (user_base)
                     {
                        user_base->append(value);
                     }
                  }
               }
            }
            else if(response && name.compareTo(HTTP_AUTHENTICATION_RESPONSE_TOKEN, UtlString::ignoreCase) == 0)
            {
               response->append(value);
            }
            else if(uri && name.compareTo(HTTP_AUTHENTICATION_URI_TOKEN, UtlString::ignoreCase) == 0)
            {
               uri->append(value);
            }
         }
      }
   }

   return value != NULL;
}


void HttpMessage::addAuthenticateField(const UtlString& authenticateField,
                                       enum HttpEndpointEnum authEntity)
{
    const char* fieldName = NULL;
    switch(authEntity)
    {
    case PROXY:
        fieldName = HTTP_PROXY_AUTHENTICATE_FIELD;
        break;
    case SERVER:
        fieldName = HTTP_WWW_AUTHENTICATE_FIELD;
        break;
    default:
       OsSysLog::add(FAC_SIP, PRI_CRIT,
                     "HttpMessage::addAuthenticateField invalid authEntity - no field added"
                     );
       assert(false);
    }
    if (fieldName)
    {
       addHeaderField(fieldName, authenticateField);
    }
}

bool HttpMessage::getAuthenticateField(int index,
                                       enum HttpEndpointEnum authEntity,
                                       UtlString& authenticateField) const
{
   authenticateField.remove(0);

   const char* fieldName;
   switch( authEntity )
   {
   case SERVER:
      fieldName = HTTP_WWW_AUTHENTICATE_FIELD;
      break;
   case PROXY:
      fieldName = HTTP_PROXY_AUTHENTICATE_FIELD;
      break;
   default:
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "HttpMessage::getAuthenticateField invalid authEntity"
                    );
      assert(false); // invalid authEntity value
   }
   if (fieldName)
   {
      const char* fieldValue = getHeaderValue(index, fieldName);
      if (fieldValue)
      {
         authenticateField.append(fieldValue);
      }
   }

   return(!authenticateField.isNull());
}


// Must check for QOP consistency before setting header data
void HttpMessage::setDigestAuthorizationData(const char* user,
                                             const char* realm,
                                             const char* nonce,
                                             const char* uri,
                                             const char* response,
                                             const char* algorithm,
                                             const char* cnonce,
                                             const char* opaque,
                                             const char* qop,
                                             const char* nonceCount,
                                             HttpMessage::HttpEndpointEnum authorizationEntity)
{
    UtlString schemeString;
    UtlString authField;

    authField.append(HTTP_DIGEST_AUTHENTICATION);
    if(user && strlen(user))
    {
        authField.append(" "
                         HTTP_AUTHENTICATION_USERNAME_TOKEN
                         "=\"");
        authField.append(user);
        authField.append("\"");
    }
    if(realm)
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_REALM_TOKEN
                         "=\"");
        authField.append(realm);
        authField.append("\"");
    }
    if(nonce && strlen(nonce))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_NONCE_TOKEN
                         "=\"");
        authField.append(nonce);
        authField.append("\"");
    }
    if(uri && strlen(uri))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_URI_TOKEN
                         "=\"");
        authField.append(uri);
        authField.append("\"");
    }
    if(response && strlen(response))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_RESPONSE_TOKEN
                         "=\"");
        authField.append(response);
        authField.append("\"");
    }
    if(algorithm && strlen(algorithm))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_ALGORITHM_TOKEN
                         "=");
        authField.append(algorithm);
    }

    if((cnonce && strlen(cnonce)) && (qop && strlen(qop)))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_CNONCE_TOKEN
                         "=\"");
        authField.append(cnonce);
        authField.append("\"");
    }
    if(opaque && strlen(opaque))
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_OPAQUE_TOKEN
                         "=\"");
        authField.append(opaque);
        authField.append("\"");
    }

    if(qop && strlen(qop)) // qop value passed in is exactly what's wanted
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_QOP_TOKEN
                         "=");
        authField.append(qop); 
    }

    // nonceCount and qop consistency should be validated before calling this function
    if(nonceCount && strlen(nonceCount))     
    {
        authField.append(", "
                         HTTP_AUTHENTICATION_NONCE_COUNT_TOKEN
                         "=");
        authField.append(nonceCount);
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

void HttpMessage::buildMd5Digest(const char* userPasswordDigest,
                                 const char* algorithm,
                                 const char* nonce,
                                 const char* cnonce,
                                 const char* nonceCount,
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

#if 0   // TBD - auth-int is not currently supported.
    // If needed, get the digest of the body
    // This code has not been tested
    ssize_t qopIndex;
    UtlString qopString(qop ? qop : "");
    UtlBoolean qopInt = FALSE;
    qopIndex = qopString.index(HTTP_QOP_AUTH_INTEGRITY, 0, UtlString::ignoreCase);
    if(qopIndex >= 0)
    {
        qopInt = TRUE;
        a2Buffer.append(':');
        if(bodyDigest) a2Buffer.append(bodyDigest);
    }
#endif

    // Encode A2
    NetMd5Codec::encode(a2Buffer.data(), encodedA2);

    // Construct buffer
    UtlString buffer(encodedA1);
    buffer.append(':');
    if(nonce) buffer.append(nonce);
    if(qop )
    {
        buffer.append(':');
        buffer.append(nonceCount);
        buffer.append(':');
        if(cnonce) buffer.append(cnonce);
        buffer.append(':');
        buffer.append(qop);

#if 0   // TBD - auth-int is not currently supported.
        // code not tested
       if(qopInt)
        {
            buffer.append(HTTP_QOP_AUTH_INTEGRITY);
        }
        else
        {
            buffer.append(HTTP_QOP_AUTH);
        }
#endif       
    }
    buffer.append(':');
    buffer.append(encodedA2);

    // Encode buffer
    NetMd5Codec::encode(buffer.data(), *responseToken);

#ifdef TEST_PRINT
    OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                  "HttpMessage::buildMd5Digest "
                  "expecting authorization: "
                  "userPasswordDigest:'%s', nonce:'%s', method:'%s', uri:'%s', "
                  "cnonce='%s', nonceCount='%s', qop='%s', response:'%s'",
                  userPasswordDigest, nonce, method, uri, 
                  cnonce, nonceCount, qop, responseToken->data());
#endif
}

UtlBoolean HttpMessage::verifyMd5Authorization(const char* userId,
                                               const char* password,
                                               const char* nonce,
                                               const char* realm,
                                               const char* cnonce,
                                               const char* nonceCount,
                                               const char* qop,
                                               const char* thisMessageMethod,
                                               const char* thisMessageUri,
                                               enum HttpEndpointEnum authEntity) const
{
    UtlBoolean allowed = FALSE;
    UtlString uri;
    UtlString method;
    UtlString referenceHash;
    UtlString msgDigestHash;

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

    // Construct A1
    UtlString a1Buffer;
    UtlString encodedA1;
    a1Buffer.append(userId);
    a1Buffer.append(':');
    a1Buffer.append(realm);
    a1Buffer.append(':');
    a1Buffer.append(password);
    NetMd5Codec::encode(a1Buffer.data(), encodedA1);

    // Build a digest hash for the reference
    buildMd5Digest(encodedA1,
                   NULL, // algorithm
                   nonce,
                   cnonce, // cnonce
                   nonceCount, // nonceCount
                   qop, // qop
                   method.data(),
                   uri.data(),
                   NULL, // body digest
                   &referenceHash);
    OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                  "HttpMessage::verifyMd5Authorization "
                  "password = '%s', nonce = '%s', method = '%s', uri = '%s', "
                  "cnonce='%s', nonceCount='%s', qop='%s', referenceHash = '%s'",
                  encodedA1.data(), nonce, method.data(), uri.data(), 
                  cnonce, nonceCount, qop, referenceHash.data());

    // Get  the digest hash contained in the message for comparison
    int authIndex = 0;
    while(getDigestAuthorizationData(NULL,  // user - not used here
                                     NULL,  // realm - not used here
                                     NULL,  // Nonce - not used here
                                     NULL,  // Opaque - not used here
                                     &msgDigestHash,
                                     NULL,  // uri - not used here
                                     NULL,  // cnonce - not used here
                                     NULL,  // nonceCount - not used here
                                     NULL,  // qop - not used here
                                     authEntity,
                                     authIndex))
    {
        OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                      "HttpMessage::verifyMd5Authorization msgDigestHash = '%s'",
                      msgDigestHash.data());
        if((referenceHash.compareTo(msgDigestHash) == 0))
        {
            allowed = TRUE;
            break;
        }
        authIndex++;
    }

#ifdef TEST
    OsSysLog::add(FAC_HTTP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization got digest response: \"%s\"\n", msgDigestHash.data());
    OsSysLog::add(FAC_HTTP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization wanted     response: \"%s\"\n", referenceHash.data());
#endif
    return(allowed);
}

UtlBoolean HttpMessage::verifyMd5Authorization(const char* userPasswordDigest,
                                               const char* nonce,
                                               const char* cnonce,
                                               const char* nonceCount,
                                               const char* qop,
                                               const char* thisMessageMethod,
                                               const char* thisMessageUri) const
{
    UtlBoolean allowed;
    UtlString uri;
    UtlString method;
    UtlString referenceHash;
    UtlString msgDigestHash;

    if (thisMessageUri && *thisMessageUri)
    {
        uri.append(thisMessageUri);
    }
    else
    {
        getRequestUri(&uri);
    }
    if (thisMessageMethod && *thisMessageMethod)
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
                   cnonce, // cnonce
                   nonceCount, // nonceCount
                   qop, // qop
                   method.data(),
                   uri.data(),
                   NULL, // body digest
                   &referenceHash);

    // Get the digest hash contained in the message for comparison
    allowed = getDigestAuthorizationData(NULL,  // user - not used here
                                         NULL,  // realm - not used here
                                         NULL,  // Nonce - not used here
                                         NULL,  // Opaque - not used here
                                         &msgDigestHash);
    if (allowed)
    {
        allowed = (referenceHash.compareTo(msgDigestHash) == 0);
    }

#ifdef TEST
    OsSysLog::add(FAC_HTTP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization got digest response: \"%s\"\n", msgDigestHash.data());
    OsSysLog::add(FAC_HTTP,PRI_DEBUG,"HttpMessage::verifyMd5Authorization wanted     response: \"%s\"\n", referenceHash.data());
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
    setAuthenticateData(authenticationScheme, authenticationRealm,
                        authenticationNonce, authenticationOpaque,
                        authenticationDomain);
}

void HttpMessage::setBasicAuthorization(const char* user,
                                        const char* password,
                                        HttpMessage::HttpEndpointEnum authorizationEntity
                                        )
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
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::getBasicAuthorizationData "
                      "cookie: \"%s\"\n",
                      cookie.data());
#endif
        int decodedLength = NetBase64Codec::decodedSize(cookie.length(), cookie.data());
        char* decodedCookie = new char[decodedLength + 1];
        NetBase64Codec::decode(cookie.length(), cookie.data(),
                                decodedLength, decodedCookie);

#ifdef TEST_PRINT
        decodedCookie[decodedLength] = 0;
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::getBasicAuthorizationData "
                      "decoded cookie: \"%s\"\n",
            decodedCookie);
#endif
        // Parse out the userId and password
        char* userPasswordSeparatorIndex = strchr(decodedCookie, ':');
        if(userPasswordSeparatorIndex)
        {
            size_t userPasswordSeparatorOffset = userPasswordSeparatorIndex - decodedCookie;

            userId->append(decodedCookie, userPasswordSeparatorOffset);
            password->append(&decodedCookie[userPasswordSeparatorOffset + 1],
                            decodedLength - (userPasswordSeparatorOffset + 1));
#ifdef TEST
            osPrintf("HttpMessage::getBasicAuthorizationData user/password separator index: %p\n",
                userPasswordSeparatorIndex);
            osPrintf("HttpMessage::getBasicAuthorizationData user: \"%s\" password: \"%s\"\n",
                userId->data(), password->data());
#endif
        }
        // No separator, assume the whole thing is a user Id
        else
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                          "HttpMessage::getBasicAuthorizationData "
                          "no user/password separator found\n");
#endif
            userId->append(decodedCookie, decodedLength);
        }

        delete[] decodedCookie;
        decodedCookie = NULL;
    }
#ifdef TEST
    else
    {
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::getBasicAuthorizationData "
                      "cookie not found in message\n");
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
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::verifyBasicAuthorization "
                      "no db user id given\n");
#endif
        userAllowed = FALSE;
    }
#ifdef TEST
    else
    {
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::verifyBasicAuthorization "
                      "user: \"%s\" password: \"%s\"\n",
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
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::verifyBasicAuthorization "
                      "user: \"%s\" password: \"%s\"\n",
            user, password);
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::verifyBasicAuthorization "
                      " ref. cookie: \"%s\"\n",
            referenceCookie.data());
        OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                      "HttpMessage::verifyBasicAuthorization "
                      "msg. cookie: \"%s\"\n",
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
            OsSysLog::add(FAC_HTTP,PRI_DEBUG,
                          "HttpMessage::verifyBasicAuthorization "
                          "no cookie in authorization field\n");
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
