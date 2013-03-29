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
#include <mongo/client/dbclient.h>
#include <mongo/client/connpool.h>

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsLogger.h"
#include "utl/PluginHooks.h"
#include "net/HttpServer.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"
#include "net/NameValueTokenizer.h"
#include "net/XmlRpcDispatch.h"
#include "registry/SipRegistrar.h"
#include "registry/RegisterPlugin.h"
#include "registry/SipRedirectServer.h"
#include "sipXecsService/SipXecsService.h"
#include "RegisterEventServer.h"
#include "SipRegistrarServer.h"
#include <assert.h>


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
   mSipUserAgent(NULL),
   mRedirectServer(NULL),
   mRedirectMsgQ(NULL),
   // Create the SipRegistrarServer object so it will be available immediately,
   // but don't start the associated thread until the registrar is operational.
   mRegistrarServer(new SipRegistrarServer(*this)),
   mRegistrarMsgQ(NULL),
   mRegisterEventServer(NULL),
   mpRegDb(NULL),
   mpSubscribeDb(NULL),
   mpEntityDb(NULL)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::SipRegistrar constructed.");


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
      Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
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
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::SipRegistrar "
                    "SIP_DOMAIN_ALIASES : %s", domainAliases.data());
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_ERR, "SipRegistrar::SipRegistrar "
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

   mongo::ConnectionString mongoConn = MongoDB::ConnectionInfo::connectionStringFromFile();
   mpRegDb = new RegDB(MongoDB::ConnectionInfo(mongoConn, RegDB::NS));
   mpSubscribeDb = new SubscribeDB(MongoDB::ConnectionInfo(mongoConn, SubscribeDB::NS));
   mpEntityDb = new EntityDB(MongoDB::ConnectionInfo(mongoConn, EntityDB::NS));

   mConfigDb->get("SIP_REGISTRAR_BIND_IP", mBindIp);
   if ((mBindIp.isNull()) || !OsSocket::isIp4Address(mBindIp))
   {
	  mBindIp = "0.0.0.0";
   }

   SipTransaction::SendTryingForNist = mConfigDb->getBoolean("SIPX_SEND_TRYING_FOR_NIST", TRUE);
}

int SipRegistrar::run(void* pArg)
{
   UtlBoolean bFatalError = false;
   int taskResult = 0;


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
       Os::Logger::instance().log(FAC_SIP, PRI_EMERG, "Unable to startup Rpc server (port in use?)\n");
   }

   return taskResult;
}

/// Launch all Startup Phase threads.
void SipRegistrar::startupPhase()
{
   Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SipRegistrar entering startup phase");

   // Create and start the persist thread, before making any changes
   // to the registration DB.

   // Step (6) is not performed explicitly.  Instead, we allow the normal
   // operation of the RegistrarTest thread to do that processing.
}


/// Launch all Operational Phase threads.
UtlBoolean SipRegistrar::operationalPhase()
{
   Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SipRegistrar entering operational phase");

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

   mSipUserAgent->start();
   startRegistrarServer();
   startRedirectServer();
   startEventServer();

   if (!mSipUserAgent->isOk())
   {
      Os::Logger::instance().log(FAC_SIP, PRI_EMERG,
            "SipUserAgent reported a problem while starting up (port in use?)");
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "tcpPort = %d, udpPort = %d, tlsPort = %d, mBindIp = '%s'",
                    tcpPort, udpPort, tlsPort, mBindIp.data());
   }

   return mSipUserAgent->isOk();
}




/// Get the RegisterEventServer thread object
RegisterEventServer* SipRegistrar::getRegisterEventServer()
{
   return mRegisterEventServer;
}


/// Get the config DB
OsConfigDb* SipRegistrar::getConfigDB()
{
   return mConfigDb;
}

void SipRegistrar::requestShutdown(void)
{
   // This is called from the SipRegistrar task destructor below in the main routine thread.
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::requestShutdown");

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
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::~ waiting for task exit");

   waitUntilShutDown(); // wait for the thread to exit

   // all other threads have been shut down
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::~ task shut down - complete destructor");

   mValidDomains.destroyAll();

   if (mpRegDb != NULL)
   {
       delete mpRegDb;
       mpRegDb = NULL;
   }

   if (mpSubscribeDb != NULL)
   {
       delete mpSubscribeDb;
       mpSubscribeDb = NULL;
   }

   if (mpEntityDb != NULL)
   {
       delete mpEntityDb;
       mpEntityDb = NULL;
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
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::handleMessage()"
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
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipRegistrar::handleMessage no message."
                         ) ;
        }

        handled = TRUE;
    }
    else if ( OsMsg::OS_SHUTDOWN == msgType )
    {
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                     "SipRegistrar::handleMessage shutting down all tasks");

       // Do an orderly shutdown of all the various threads.


       if ( mSipUserAgent )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down SipUserAgent");
          mSipUserAgent->shutdown();
          delete mSipUserAgent ;
          mSipUserAgent = NULL ;
       }


       if ( mRedirectServer )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RedirectServer");
          mRedirectServer->requestShutdown();
          delete mRedirectServer;
          mRedirectServer = NULL;
          mRedirectMsgQ = NULL;
       }

       if ( mRegistrarServer )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegistrarServer");
          mRegistrarServer->requestShutdown();
          delete mRegistrarServer;
          mRegistrarServer = NULL;
          mRegistrarMsgQ = NULL;
       }

       if ( mRegisterEventServer )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                        "SipRegistrar::handleMessage shutting down RegisterEventServer");
          delete mRegisterEventServer;
          mRegisterEventServer = NULL;
       }

       OsTask::requestShutdown(); // tell OsServerTask::run to exit
       handled = TRUE;
    }
    else
    {
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
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
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipRegistrar::getInstance(%p)",
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
       Os::Logger::instance().log(FAC_SIP, PRI_CRIT, "sendToRedirectServer - queue not initialized.");
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
       Os::Logger::instance().log(FAC_SIP, PRI_CRIT, "sendToRegistrarServer - queue not initialized.");
    }
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
      Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
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

   Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "SipRegistrar::addValidDomain(%s)",valid->data()) ;

   mValidDomains.insert(valid);
}
