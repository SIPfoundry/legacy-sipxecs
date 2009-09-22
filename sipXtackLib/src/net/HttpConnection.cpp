//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


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

// APPLICATION INCLUDES
#include <net/HttpMessage.h>
#include <net/HttpConnection.h>
#include <net/NameValuePair.h>
// Needed for SIP_SHORT_CONTENT_LENGTH_FIELD.
#include <net/SipMessage.h>

#include <os/OsConnectionSocket.h>
#ifdef HAVE_SSL
#include <os/OsSSLConnectionSocket.h>
#endif /* HAVE_SSL */
#include <os/OsSysLog.h>
#include <os/OsTask.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HTTP_READ_TIMEOUT_MSECS  30000

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpConnection::HttpConnection(OsConnectionSocket* requestSocket,
                               HttpServer* httpServer) :
   OsTask("HttpConnection-%d"),
   UtlString(getName()), // That's OsTask::getName()
   mpRequestSocket(requestSocket),
   mpHttpServer(httpServer),
   mbToBeDeleted(false)
{
}

// Destructor
HttpConnection::~HttpConnection()
{
    // Tell the task we want to shutdown (sets isShuttingDown())
    requestShutdown() ;

    // Wake it up by "closing" the socket (actually calls shutdown(2))
    if (mpRequestSocket)
    {
        mpRequestSocket->close();
    }

    // Wait until run exits before clobbering members
    waitUntilShutDown();

    OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                  "Shutting down HttpConnection for socket %p",
                  mpRequestSocket);

    if (mpRequestSocket)
    {
        delete mpRequestSocket;
        mpRequestSocket = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */
int HttpConnection::run(void* runArg)
{
    HttpMessage request;
    bool bConnected = true;

    if (!mpRequestSocket || !mpRequestSocket->isOk())
    {
        OsSysLog::add(FAC_HTTP, PRI_ERR, "HttpConnection: port not ok" );
    }

    while(!isShuttingDown() && mpRequestSocket && mpRequestSocket->isOk() && bConnected)
    {
        // Read a http request from the socket
        if (mpRequestSocket->isReadyToRead(HTTP_READ_TIMEOUT_MSECS))
        {
            int bytesRead = request.read(mpRequestSocket);

            if (bytesRead > 0)
            {
                UtlString remoteIp;
                mpRequestSocket->getRemoteHostIp(&remoteIp);

                HttpMessage* response = NULL;

                // If request from Valid IP Address
                if (mpHttpServer->processRequestIpAddr(remoteIp, request, response))
                {
                    // If the request is authorized
                    mpHttpServer->processRequest(request, response, mpRequestSocket);
                }

                if(response)
                {
                    response->setHeaderValue("Connection", "Keep-Alive");
                    response->write(mpRequestSocket);
                    delete response;
                    response = NULL;
                }
            }
            else
            {
                // isReadyToRead indicated readability but the read returned 0 bytes - the peer
                // must have shut down
                bConnected = FALSE;
                OsSysLog::add(FAC_HTTP, PRI_WARNING, "HttpConnection::run"
                              " read 0 bytes, indicating peer shut down");
            }
        }
    }
    // If we're here either we're shutting down or the socket went bad. Mark for deletion
    mbToBeDeleted = true;
    OsSysLog::add(FAC_HTTP, PRI_DEBUG, "HttpConnection::run shutting down");

    return(TRUE);
}
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
