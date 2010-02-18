//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsUtil.h"
#include "statusserver/WebServer.h"
#include "os/OsSysLog.h"



// APPLICATION INCLUDES
#include "net/HttpServer.h"
#include "net/HttpRequestContext.h"
#include "net/HttpMessage.h"
#include "statusserver/PluginXmlParser.h"
#include "statusserver/SubscribeServerPluginBase.h"
#include "statusserver/StatusPluginReference.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
#define EVENTTYPE "eventType"

// STATIC VARIABLE INITIALIZATIONS
WebServer* WebServer::spInstance = 0;
OsBSem  WebServer::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
PluginXmlParser* WebServer::spPluginTable = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the WebServer task, creating it if necessary
WebServer* WebServer::getWebServerTask(PluginXmlParser* pluginTable)
{
    // If the task does not yet exist or hasn't been started, then acquire
    // the lock to ensure that only one instance of the task is started
    sLock.acquire();

    // If the task object already exists, and the corresponding low-level task
    // has been started, then use it
    if ( spInstance == NULL )
        spInstance = new WebServer();

    if( pluginTable )
        spPluginTable = pluginTable;

    sLock.release();

    return spInstance;
}

// Destructor
WebServer::~WebServer()
{
    spInstance = NULL;
    spPluginTable = NULL;
}

/* ============================ ACCESSORS ================================= */
void
WebServer::ProcessEvent(
    const HttpRequestContext& requestContext,
    const HttpMessage& request,
    HttpMessage*& response )
{
    // get the action type (used to be the event)
    UtlString event;
    response = new HttpMessage();

    ssize_t len;
    UtlString httpString;
    SubscribeServerPluginBase* plugin = NULL;

    request.getBytes(&httpString , &len);
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "WebServer::ProcessEvent HttpEvent \n%s",
                  httpString.data());

    // get the ACTION CGI variable
    requestContext.getCgiVariable( EVENTTYPE, event );

    if( !event.isNull())
    {
        //according to event type , get the correct plugin from spPluginTable
        StatusPluginReference* pluginContainer = spPluginTable->getPlugin(event);
        if(pluginContainer)
        {
            plugin = pluginContainer->getPlugin();
            if(plugin)
            {
                // send 200 ok reply.
                response->setResponseFirstHeaderLine (
                    HTTP_PROTOCOL_VERSION,
                    HTTP_OK_CODE,
                    HTTP_OK_TEXT );

                //call the event handler for the plugin
                plugin->handleEvent(requestContext, request, *response);
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "WebServer::ProcessEvent no plugin in container for event type '%s'",
                             event.data()
                             );
            }
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_WARNING,
                         "WebServer::ProcessEvent no plugin found for event type '%s'",
                         event.data()
                         );
        }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_WARNING,
                     "WebServer::ProcessEvent no '" EVENTTYPE "' variable found"
                     );
    }


    // We did not find a plugin so nobody handled this request.
    if(plugin == NULL)
    {
        response->setResponseFirstHeaderLine (
            HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE,
            HTTP_FILE_NOT_FOUND_TEXT );
    }
    OsSysLog::flush();

}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via Status Server())
WebServer::WebServer()
{}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void WebServer::initWebServer( HttpServer* pHttpServer )
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "WebServer::initWebServer") ;
    if( pHttpServer )
    {
        // New user interface
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "WebServer::add requests to web server") ;

        pHttpServer->addRequestProcessor("/cgi/StatusEvent.cgi", ProcessEvent);
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_CRIT,
                      "WebServer::initWebServer no http server passed - requests not added"
                      ) ;
    }
}
