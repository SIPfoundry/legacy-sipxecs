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



#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

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

#ifdef TEST
static UtlMemCheck* spMemCheck = NULL;
#endif

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
{  // get Event Type
    UtlString event;
    response = new HttpMessage();

    int len;
    UtlString httpString;
    SubscribeServerPluginBase* plugin = NULL;
    request.getBytes(&httpString , &len);
    osPrintf("WebServer::ProcessEvent HttpEvent \n%s\n", httpString.data());

    requestContext.getCgiVariable(EVENTTYPE, event);
    if( !event.isNull())
    {
        //according to event type , get the correct plugin from spPluginTable
        StatusPluginReference* pluginContainer = spPluginTable->getPlugin(event);
        if(pluginContainer)
        {
            plugin = pluginContainer->getPlugin();
            if(plugin)
            {
                //create copy of request , response and request Context
                //HttpRequestContext *reqContext = new HttpRequestContext(requestContext);
                //HttpMessage *req = new HttpMessage(request);
                //HttpMessage *res = new HttpMessage(*response);

                // send 200 ok reply.
                response->setResponseFirstHeaderLine (
                    HTTP_PROTOCOL_VERSION,
                    HTTP_OK_CODE,
                    HTTP_OK_TEXT );

                //call the event handler for the plugin
                plugin->handleEvent(requestContext, request, *response);
            }
        }
    }

    // We did not find a plugin so nobody handled this request.
    if(plugin == NULL)
    {
        response->setResponseFirstHeaderLine (
            HTTP_PROTOCOL_VERSION,
            HTTP_FILE_NOT_FOUND_CODE,
            HTTP_FILE_NOT_FOUND_TEXT );
    }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via Status Server())
WebServer::WebServer()
{}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void WebServer::initWebServer( HttpServer* pHttpServer )
{
   osPrintf("WebServer::initWebServer reached\n") ;
   if( pHttpServer )
   {
      osPrintf("WebServer::access HttpServer pointer\n") ;
      // New user interface
      osPrintf("WebServer::add requests to web server\n") ;

      pHttpServer->addRequestProcessor("/cgi/ProcessMwiEvent.cgi", ProcessEvent);
   }
   else
   {
      osPrintf("WebServer::couldn't add requests\n") ;
   }
}
