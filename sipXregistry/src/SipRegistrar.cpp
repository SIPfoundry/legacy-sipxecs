//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "utl/PluginHooks.h"
#include "net/HttpServer.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "net/NameValueTokenizer.h"
#include "net/XmlRpcDispatch.h"
#include "sipdb/RegistrationDB.h"
#include "registry/SipRegistrar.h"
#include "registry/RegisterPlugin.h"
#include "registry/SipRedirectServer.h"
#include "sipXecsService/SipXecsService.h"
#include "RegisterEventServer.h"
#include "RegistrarInitialSync.h"
#include "RegistrarPeer.h"
#include "RegistrarPersist.h"
#include "RegistrarSync.h"
#include "RegistrarTest.h"
#include "SipRegistrarServer.h"
#include "SyncRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define CONFIG_SETTING_LOG_LEVEL      "SIP_REGISTRAR_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"

const int REGISTRAR_DEFAULT_SIP_PORT  = 5070;
const int REGISTRAR_DEFAULT_SIPS_PORT = 5071;

const char* RegisterPlugin::Prefix  = "SIP_REGISTRAR";
const char* RegisterPlugin::Factory = "getRegisterPlugin";

const int REGISTRAR_DEFAULT_REG_EVENT_PORT = 5075;

// STATIC VARIABLE INITIALIZATIONS

const int SipRegistrar::SIP_REGISTRAR_DEFAULT_XMLRPC_PORT = 5077;

SipRegistrar* SipRegistrar::spInstance = NULL;
OsBSem SipRegistrar::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);


// Constructor
SipRegistrar::SipRegistrar(OsConfigDb* configDb) :
   OsServerTask("SipRegistrar", NULL, SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE),
   mConfigDb(configDb),
   mRegistrationDB(RegistrationDB::getInstance()), // implicitly loads database
   mHttpServer(NULL),
   mXmlRpcDispatch(NULL),
   mReplicationConfigured(false),
   mSipUserAgent(NULL),
   mRedirectServer(NULL),
   mRedirectMsgQ(NULL),
   // Create the SipRegistrarServer object so it will be available immediately,
   // but don't start the associated thread until the registrar is operational.
   mRegistrarServer(new SipRegistrarServer(*this)),
   mRegistrarMsgQ(NULL),
   mRegistrarInitialSync(NULL),
   mRegistrarSync(NULL),
   mRegisterEventServer(NULL),
   mRegistrarTest(NULL),
   mRegistrarPersist(NULL)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::SipRegistrar constructed.");

   mHttpPort = mConfigDb->getPort("SIP_REGISTRAR_XMLRPC_PORT");
   if (PORT_NONE == mHttpPort)
   {
      OsSysLog::add(FAC_SIP, PRI_NOTICE,
                    "SipRegistrar::SipRegistrar"
                    " SIP_REGISTRAR_XMLRPC_PORT == PORT_NONE :"
                    " peer synchronization disabled"
                    );
   }
   else // HTTP/RPC port is configured
   {
      if (PORT_DEFAULT == mHttpPort)
      {
         mHttpPort = SIP_REGISTRAR_DEFAULT_XMLRPC_PORT;
      }

      configurePeers();
   }

   // Some phones insist (incorrectly) on putting the proxy port number on urls;
   // we get it from the configuration so that we can ignore it.
   mProxyNormalPort = mConfigDb->getPort("SIP_REGISTRAR_PROXY_PORT");
   if (mProxyNormalPort == PORT_DEFAULT)
   {
      mProxyNormalPort = SIP_PORT;
   }

   // Domain Name
   mConfigDb->get("SIP_REGISTRAR_DOMAIN_NAME", mDefaultDomain);
   if ( mDefaultDomain.isNull() )
   {
      OsSocket::getHostIp(&mDefaultDomain);
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SIP_REGISTRAR_DOMAIN_NAME not configured using IP '%s'",
                    mDefaultDomain.data()
                    );
   }
   // get the url parts for the domain
   Url defaultDomainUrl(mDefaultDomain);
   mDefaultDomainPort = defaultDomainUrl.getHostPort();
   defaultDomainUrl.getHostAddress(mDefaultDomainHost);
   // make sure that the unspecified domain name is also valid
   addValidDomain(mDefaultDomainHost, mDefaultDomainPort);

   // read the domain configuration
   OsConfigDb domainConfig;
   domainConfig.loadFromFile(SipXecsService::domainConfigPath());


   // Domain Aliases
   //   (other domain names that this registrar accepts as valid in the request URI)
   UtlString domainAliases;
   domainConfig.get(SipXecsService::DomainDbKey::SIP_DOMAIN_ALIASES, domainAliases);

   if (!domainAliases.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::SipRegistrar "
                    "SIP_DOMAIN_ALIASES : %s", domainAliases.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipRegistrar::SipRegistrar "
                    "SIP_DOMAIN_ALIASES not found.");
   }

   UtlString aliasString;
   int aliasIndex = 0;
   while(NameValueTokenizer::getSubField(domainAliases.data(), aliasIndex,
                                         ", \t", &aliasString))
   {
      Url aliasUrl(aliasString);
      UtlString hostAlias;
      aliasUrl.getHostAddress(hostAlias);
      int port = aliasUrl.getHostPort();

      addValidDomain(hostAlias,port);
      aliasIndex++;
   }

   mConfigDb->get("SIP_REGISTRAR_BIND_IP", mBindIp);
   if ((mBindIp.isNull()) || !OsSocket::isIp4Address(mBindIp))
   {
	  mBindIp = "0.0.0.0";
   }
}

int SipRegistrar::run(void* pArg)
{
   UtlBoolean bFatalError = false;
   int taskResult = 0;

   startRpcServer(bFatalError);
   if (!bFatalError)
   {
      /*
       * If replication is configured,
       *   the following blocks until the state of each peer is known
       */
      startupPhase();

      if (!isShuttingDown())
      {
         // Exit if the operational phase fails (e.g. SipUserAgent reports
         // problems)
         if (operationalPhase())
         {
            // from here on, everything happens in handleMessage
            taskResult = OsServerTask::run(pArg);
         }
      }
   }
   else
   {
       OsSysLog::add(FAC_SIP, PRI_EMERG, "Unable to startup Rpc server (port in use?)\n");
   }

   return taskResult;
}

/// Launch all Startup Phase threads.
void SipRegistrar::startupPhase()
{
   OsSysLog::add(FAC_SIP, PRI_INFO, "SipRegistrar entering startup phase");

   // Create and start the persist thread, before making any changes
   // to the registration DB.
   createAndStartPersist();

   if (mReplicationConfigured)
   {
      // Create replication-related thread objects, because they own some
      // the data that the RegistrarInitialSync thread needs,
      // but don't start them yet.
      createReplicationThreads();

      // Begin the RegistrarInitialSync thread and then wait for it.
      // The RegistrarInitialSync thread performs steps (1) to (4) of
      // section 5.7.1 of sipXregistry/doc/SyncDesign.*.
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRegistrar::startupPhase starting initialSyncThread"
                    );
      mRegistrarInitialSync->start();
      yield();
      mRegistrarInitialSync->waitForCompletion();

      // The initial sync thread has no further value, to the ash heap of history it goes
      delete mRegistrarInitialSync;
      mRegistrarInitialSync = NULL;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "SipRegistrar::startupPhase no replication configured"
                    );
   }

   // Reset the DbUpdateNumber so that the upper half is the epoch time.
   // Step (5) in section 5.7.1 of sipXregistry/doc/SyncDesign.*.
   // We perform this even if there are no peer registrars to ensure that
   // update numbers are monotonic even if HA is enabled and then disabled.
   getRegistrarServer().resetDbUpdateNumberEpoch();

   // Step (6) is not performed explicitly.  Instead, we allow the normal
   // operation of the RegistrarTest thread to do that processing.
}

/// Create and start the RegistrarPersist thread.
void SipRegistrar::createAndStartPersist()
{
   mRegistrarPersist = new RegistrarPersist(*this);
   mRegistrarPersist->start();
}

/// Launch all Operational Phase threads.
UtlBoolean SipRegistrar::operationalPhase()
{
   OsSysLog::add(FAC_SIP, PRI_INFO, "SipRegistrar entering operational phase");

   // Start the SIP stack.
   int tcpPort = PORT_DEFAULT;
   int udpPort = PORT_DEFAULT;
   int tlsPort = PORT_DEFAULT;

   udpPort = mConfigDb->getPort("SIP_REGISTRAR_UDP_PORT");
   if (udpPort == PORT_DEFAULT)
   {
      udpPort = REGISTRAR_DEFAULT_SIP_PORT;
   }

   tcpPort = mConfigDb->getPort("SIP_REGISTRAR_TCP_PORT");
   if (tcpPort == PORT_DEFAULT)
   {
      tcpPort = REGISTRAR_DEFAULT_SIP_PORT;
   }

   tlsPort = mConfigDb->getPort("SIP_REGISTRAR_TLS_PORT");
   if (tlsPort == PORT_DEFAULT)
   {
      tlsPort = REGISTRAR_DEFAULT_SIPS_PORT;
   }

   mSipUserAgent = new SipUserAgent(tcpPort,
                                    udpPort,
                                    tlsPort,
                                    NULL,   // public IP address (not used)
                                    NULL,   // default user (not used)
                                    mBindIp,
                                    NULL,   // outbound proxy
                                    NULL,   // directory server
                                    NULL,   // registry server
                                    NULL,   // auth realm
                                    NULL,   // auth DB
                                    NULL,   // auth user IDs
                                    NULL,   // auth passwords
                                    NULL,   // line mgr
                                    SIP_DEFAULT_RTT, // first resend timeout
                                    TRUE,   // default to UA transaction
                                    SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
                                    SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE, // OsServerTask message queue size
                                    FALSE,  // do not use next available port
                                    FALSE,  // do not do UA message checks for METHOD, requires, etc...
                                    TRUE,   // forceSymmetricSignaling
                                    SipUserAgent::PASS_OPTIONS_TO_CONSUMER
                                    );

   if ( mSipUserAgent )
   {
      mSipUserAgent->addMessageObserver( *this->getMessageQueue(), NULL /* all methods */ );

      // the above causes us to receive all methods
      // the following sets what we send in Allow headers
      mSipUserAgent->allowMethod(SIP_REGISTER_METHOD);
      mSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
      mSipUserAgent->allowMethod(SIP_OPTIONS_METHOD);
      mSipUserAgent->allowMethod(SIP_CANCEL_METHOD);

      mSipUserAgent->allowExtension("gruu"); // should be moved to gruu processor?
      mSipUserAgent->allowExtension("path");

      mSipUserAgent->setUserAgentHeaderProperty("sipXecs/registry");
   }

   if (mReplicationConfigured)
   {
      // Start the test and sync threads.  (We ran the init sync thread earlier.)
      mRegistrarTest->start();
      mRegistrarSync->start();

      // Register the pushUpdates and reset methods.
      // (We registered the pullUpdates method earlier because it only needs DB access.)
      SyncRpcPushUpdates::registerSelf(*this);
      SyncRpcReset::registerSelf(*this);
   }

   mSipUserAgent->start();
   startRegistrarServer();
   startRedirectServer();
   startEventServer();

   if (!mSipUserAgent->isOk())
   {
      OsSysLog::add(FAC_SIP, PRI_EMERG,
            "SipUserAgent reported a problem while starting up (port in use?)");
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "tcpPort = %d, udpPort = %d, tlsPort = %d, mBindIp = '%s'",
                    tcpPort, udpPort, tlsPort, mBindIp.data());
   }

   return mSipUserAgent->isOk();
}

/// Get the XML-RPC dispatcher
XmlRpcDispatch* SipRegistrar::getXmlRpcDispatch()
{
   return mXmlRpcDispatch;
}

/// Get the RegistrarPersist thread object
RegistrarPersist* SipRegistrar::getRegistrarPersist()
{
   return mRegistrarPersist;
}

/// Get the RegistrarTest thread object
RegistrarTest* SipRegistrar::getRegistrarTest()
{
   return mRegistrarTest;
}

/// Get the RegistrarSync thread object
RegistrarSync* SipRegistrar::getRegistrarSync()
{
   return mRegistrarSync;
}

/// Get the RegisterEventServer thread object
RegisterEventServer* SipRegistrar::getRegisterEventServer()
{
   return mRegisterEventServer;
}

/// Return true if replication is configured, false otherwise
bool SipRegistrar::isReplicationConfigured()
{
   return mReplicationConfigured;
}

/// Get the RegistrationDB thread object
RegistrationDB* SipRegistrar::getRegistrationDB()
{
   return mRegistrationDB;
}

/// Get the config DB
OsConfigDb* SipRegistrar::getConfigDB()
{
   return mConfigDb;
}

void SipRegistrar::requestShutdown(void)
{
   // This is called from the SipRegistrar task destructor below in the main routine thread.
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::requestShutdown");

   // Set the low level task flag and wake up the SipRegistrar task

   OsMsg msg(OsMsg::OS_SHUTDOWN, 0);
   /*
    * Let the shutdown message wake up the SipRegistrar task
    * so that it can shut down all the other threads in an orderly way.
    */
   postMessage(msg);
   yield(); // make the caller wait so that SipRegistrar can run.
}

// Destructor
SipRegistrar::~SipRegistrar()
{
   // this is called from the main routine
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::~ waiting for task exit");

   waitUntilShutDown(); // wait for the thread to exit

   // all other threads have been shut down
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::~ task shut down - complete destructor");

   mPeers.destroyAll();

   mValidDomains.destroyAll();

   // release the registration database instance
   if (mRegistrationDB)
   {
      mRegistrationDB->releaseInstance();
      mRegistrationDB = NULL;
   }
}

/// Get the default domain name for this registrar
const char* SipRegistrar::defaultDomain() const
{
   return mDefaultDomain.data();
}


/// Get the proxy port for the domain
int SipRegistrar::domainProxyPort() const
{
   return mProxyNormalPort;
}


UtlBoolean SipRegistrar::handleMessage( OsMsg& eventMessage )
{
    UtlBoolean handled = FALSE;

    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    if (   (msgType == OsMsg::PHONE_APP)
        && (msgSubType == SipMessage::NET_SIP_MESSAGE)
        )
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::handleMessage()"
                      " Start processing SIP message") ;

        const SipMessage* message =
           ((SipMessageEvent&)eventMessage).getMessage();
        UtlString callId;
        if ( message )
        {
            message->getCallIdField(&callId);
            UtlString method;
            message->getRequestMethod(&method);

            if ( !message->isResponse() ) // is a request ?
            {
                if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
                {
                    //send to Register Thread
                    sendToRegistrarServer(eventMessage);
                }
                else if ( method.compareTo(SIP_OPTIONS_METHOD) == 0 )
                {
                    UtlString requestUri;
                    message->getRequestUri(&requestUri);

                    // Check if the OPTIONS request URI is addressed to a user or
                    // to the domain.
                    if (!requestUri.contains("@"))
                    {
                        UtlString contentEncoding;
                        message->getContentEncodingField(&contentEncoding);

                        UtlString disallowedExtensions;

                        int extensionIndex = 0;
                        UtlString extension;

                        disallowedExtensions.remove(0);
                        while(message->getRequireExtension(extensionIndex, &extension))
                        {
                            if(!(mSipUserAgent->isExtensionAllowed(extension.data())) )
                            {
                                if(!disallowedExtensions.isNull())
                                {
                                    disallowedExtensions.append(SIP_MULTIFIELD_SEPARATOR);
                                    disallowedExtensions.append(SIP_SINGLE_SPACE);
                                }
                                disallowedExtensions.append(extension.data());
                            }
                            extensionIndex++;
                        }

                        //delete leading and trailing white spaces
                        disallowedExtensions = disallowedExtensions.strip(UtlString::both);

                        SipMessage response;

                        // Check if the extensions are supported
                        if(!disallowedExtensions.isNull() )
                        {
                           // error response - bad extension
                           response.setRequestBadExtension(message,
                                                           disallowedExtensions);
                        }
                        // Check if the encoding is supported
                        // i.e. no encoding
                        else if(!contentEncoding.isNull())
                        {
                           // error response - content encoding
                           response.setRequestBadContentEncoding(message,"");
                        }
                        else
                        {
                            // Send an OK, the allowed field will get added to all final responses.
                            // Options 200 response
                            response.setResponseData(message,
                                                     SIP_OK_CODE,
                                                     SIP_OK_TEXT);
                        }

                        mSipUserAgent->send(response);
                    }
                    else
                    {
                        //OPTIONS is addressed to a user, send to redirect thread
                        sendToRedirectServer(eventMessage);
                    }
                }
                else
                {
                    //send to redirect thread
                    sendToRedirectServer(eventMessage);
                }
            }
            else
            {
               // responses are ignored.
            }
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipRegistrar::handleMessage no message."
                         ) ;
        }

        handled = TRUE;
    }
    else if ( OsMsg::OS_SHUTDOWN == msgType )
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipRegistrar::handleMessage shutting down all tasks");

       // Do an orderly shutdown of all the various threads.

       // Deleting an OsServerTask is the only way of
       // waiting for it to complete cleanly
       if ( mRegistrarInitialSync )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarInitialSync");
          mRegistrarInitialSync->requestShutdown();
          delete mRegistrarInitialSync;
          mRegistrarInitialSync = NULL;
       }

       if ( mSipUserAgent )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down SipUserAgent");
          mSipUserAgent->shutdown();
          delete mSipUserAgent ;
          mSipUserAgent = NULL ;
       }

       if ( mHttpServer )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down HttpServer");
          mHttpServer->requestShutdown();
          delete mHttpServer;
          mHttpServer = NULL;
       }

       if ( mRegistrarTest )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarTest");
          delete mRegistrarTest;
          mRegistrarTest = NULL;
       }

       if ( mRegistrarSync )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarSync");
          mRegistrarSync->requestShutdown();
          delete mRegistrarSync;
          mRegistrarSync = NULL;
       }

       if ( mRegistrarPersist )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarPersist");
          mRegistrarPersist->requestShutdown();
          delete mRegistrarPersist;
          mRegistrarPersist = NULL;
       }

       if ( mRedirectServer )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RedirectServer");
          mRedirectServer->requestShutdown();
          delete mRedirectServer;
          mRedirectServer = NULL;
          mRedirectMsgQ = NULL;
       }

       if ( mRegistrarServer )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarServer");
          mRegistrarServer->requestShutdown();
          delete mRegistrarServer;
          mRegistrarServer = NULL;
          mRegistrarMsgQ = NULL;
       }

       if ( mRegisterEventServer )
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegisterEventServer");
          delete mRegisterEventServer;
          mRegisterEventServer = NULL;
       }

       OsTask::requestShutdown(); // tell OsServerTask::run to exit
       handled = TRUE;
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipRegistrar::handleMessage unhandled type %d/%d",
                     msgType, msgSubType
                     ) ;
    }

    return handled;
}

SipRegistrar*
SipRegistrar::getInstance(OsConfigDb* configDb)
{
    OsLock singletonLock(sLock);

    if ( spInstance == NULL )
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRegistrar::getInstance(%p)",
                     configDb);

       spInstance = new SipRegistrar(configDb);
    }

    return spInstance;
}

SipRegistrarServer&
SipRegistrar::getRegistrarServer()
{
   // The SipRegistrarServer is created in the SipRegistrar constructor, so
   // mRegistrarServer should never be null
   assert(mRegistrarServer);

   return *mRegistrarServer;
}

/// Read peer configuration and initialize peer state
void SipRegistrar::configurePeers()
{
   // in case we can ever do this on the fly, clear out any old peer configuration
   mReplicationConfigured = false;
   mPeers.destroyAll();
   mPrimaryName.remove(0);

   UtlString peersMsg;

   mConfigDb->get("SIP_REGISTRAR_NAME", mPrimaryName);

   if (! mPrimaryName.isNull())
   {
      mPrimaryName.toLower();
      OsSysLog::add(FAC_SIP, PRI_INFO, "SipRegistrar::configurePeers "
                    "SIP_REGISTRAR_NAME : '%s'", mPrimaryName.data()
                    );

      UtlString peerNames;
      mConfigDb->get("SIP_REGISTRAR_SYNC_WITH", peerNames);

      if (!peerNames.isNull())
      {
         UtlString peerName;

         for (int peerIndex = 0;
              NameValueTokenizer::getSubField(peerNames.data(), peerIndex, ", \t", &peerName);
              peerIndex++
              )
         {
            if (peerName.compareTo(mPrimaryName, UtlString::ignoreCase)) // not myself
            {
               RegistrarPeer* thisPeer = new RegistrarPeer(this, peerName, mHttpPort);
               assert(thisPeer);

               mPeers.append(thisPeer);
               if (!peersMsg.isNull())
               {
                  peersMsg.append(", ");
               }
               peersMsg.append(thisPeer->data());
            }
         }

         if (mPeers.isEmpty())
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "SipRegistrar::configurePeers - no peers configured"
                          );
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_INFO,
                          "SipRegistrar::configurePeers: %s", peersMsg.data()
                          );
            mReplicationConfigured = true;
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_NOTICE, "SipRegistrar::configurePeers "
                       "SIP_REGISTRAR_SYNC_WITH not set - replication disabled"
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_NOTICE, "SipRegistrar::configurePeers "
                    "SIP_REGISTRAR_NAME not set - replication disabled"
                    );
   }
}


/// If replication is configured, then name of this registrar as primary
const UtlString& SipRegistrar::primaryName() const
{
   return mPrimaryName;
}


/// Server for XML-RPC requests
void SipRegistrar::startRpcServer(UtlBoolean& bFatalError)
{
    bFatalError = false; // disable is not a fatal error

   // Begins operation of the HTTP/RPC service
   // sets mHttpServer and mXmlRpcDispatcher
   if (mReplicationConfigured)
   {
      // Initialize mHttpServer and mXmlRpcDispatch
      mXmlRpcDispatch = new XmlRpcDispatch(mHttpPort, true /* use https */,
                                           XmlRpcDispatch::DEFAULT_URL_PATH,
                                           mBindIp);
      mHttpServer = mXmlRpcDispatch->getHttpServer();
      if (!mHttpServer->isSocketOk())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "XmlRpc HttpServer failed to initialize listening socket");
         bFatalError = true ;
      }
   }
}

/// Get an iterator over all peers.
UtlSListIterator* SipRegistrar::getPeers()
{
   return (  ( ! mReplicationConfigured || mPeers.isEmpty() )
           ? NULL : new UtlSListIterator(mPeers));
}

/// Get peer state object by name.
RegistrarPeer* SipRegistrar::getPeer(const UtlString& peerName)
{
   return (  mReplicationConfigured
           ? dynamic_cast<RegistrarPeer*>(mPeers.find(&peerName))
           : NULL
           );
}

void
SipRegistrar::startRedirectServer()
{
   mRedirectServer = new SipRedirectServer(mConfigDb, mSipUserAgent);
   mRedirectMsgQ = mRedirectServer->getMessageQueue();
   mRedirectServer->start();
}

void
SipRegistrar::startRegistrarServer()
{
    mRegistrarMsgQ = mRegistrarServer->getMessageQueue();
    mRegistrarServer->initialize(mConfigDb, mSipUserAgent);
    mRegistrarServer->start();
}

void
SipRegistrar::startEventServer()
{
   // Start the registration event server.
   int port = mConfigDb->getPort("SIP_REGISTRAR_REG_EVENT_PORT");
   if (port == PORT_DEFAULT)
   {
      port = REGISTRAR_DEFAULT_REG_EVENT_PORT;
   }

   mRegisterEventServer = new RegisterEventServer(defaultDomain(),
                                                  port,
                                                  port,
                                                  PORT_NONE,
                                                  mBindIp);
}

void
SipRegistrar::sendToRedirectServer(OsMsg& eventMessage)
{
    if ( mRedirectMsgQ )
    {
        mRedirectMsgQ->send(eventMessage);
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT, "sendToRedirectServer - queue not initialized.");
    }
}

void
SipRegistrar::sendToRegistrarServer(OsMsg& eventMessage)
{
    if ( mRegistrarMsgQ )
    {
        mRegistrarMsgQ->send(eventMessage);
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT, "sendToRegistrarServer - queue not initialized.");
    }
}

/// Create replication-related thread objects, but don't start them yet
void SipRegistrar::createReplicationThreads()
{
   mRegistrarInitialSync = new RegistrarInitialSync(*this);
   mRegistrarSync = new RegistrarSync(*this);
   mRegistrarTest = new RegistrarTest(*this);
}

bool
SipRegistrar::isValidDomain(const Url& uri) const
{
   bool isValid = false;

   UtlString domain;
   uri.getHostAddress(domain);
   domain.toLower();

   int port = uri.getHostPort();
   if (port == PORT_NONE)
   {
      port = SIP_PORT;
   }
   char portNum[15];
   sprintf(portNum,"%d",port);

   domain.append(":");
   domain.append(portNum);

   if ( mValidDomains.contains(&domain) )
   {
      isValid = true;
      OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                    "SipRegistrar::isValidDomain(%s) VALID",
                    domain.data()) ;
   }
   return isValid;
}

void
SipRegistrar::addValidDomain(const UtlString& host, int port)
{
   UtlString* valid = new UtlString(host);
   valid->toLower();

   char explicitPort[20];
   sprintf(explicitPort,":%d", PORT_NONE==port ? SIP_PORT : port );
   valid->append(explicitPort);

   OsSysLog::add(FAC_AUTH, PRI_DEBUG, "SipRegistrar::addValidDomain(%s)",valid->data()) ;

   mValidDomains.insert(valid);
}
