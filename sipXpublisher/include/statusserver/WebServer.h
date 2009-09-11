//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _WebServer_h_
#define _WebServer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsConfigDb.h"
#include "os/OsServerTask.h"
#include "net/HttpServer.h"

class PluginXmlParser;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
UtlString httpCreateRow(const char* row, const char* value);
void processGetConfig(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response);

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
//:WebServer class
// This is the main WebServer task. It is a singleton task and is responsible
// for initializing the WebServer device and starting up any other tasks that
// are needed.
class Url;

class WebServer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   static WebServer* getWebServerTask(PluginXmlParser* pluginTable);
     //:Return a pointer to the WebServer task, creating it if necessary

   virtual
   ~WebServer();
     //:Destructor

   void initWebServer(HttpServer* pHttpServer);

/* ============================ MANIPULATORS ============================== */

   static void ProcessEvent(const HttpRequestContext& requestContext,
                      const HttpMessage& request,
                      HttpMessage*& response) ;


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
	static PluginXmlParser* spPluginTable;

   WebServer();
     //:Constructor (called only indirectly via getWebServerTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


   //HttpServer*       mpHttpServer;       // Http Server

   // Static data members used to enforce Singleton behavior
   static WebServer*    spInstance;    // pointer to the single instance of
                                    //  the WebServer class
   static OsBSem     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

   // device profile (pinger-config) parameters
   static const char* deviceConfig[] ;

   // all parameters managed by the Web UI
   static const char* allConfig[] ;

private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _WebServer_h_
