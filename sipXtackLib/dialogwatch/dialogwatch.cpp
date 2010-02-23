//
//
// Copyright (C) 2007-2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <unistd.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <net/NetMd5Codec.h>
#include <net/SipDialogEvent.h>
#include <net/SipDialogMgr.h>
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/SipMessage.h>
#include <net/SipRefreshManager.h>
#include <net/SipRegEvent.h>
#include <net/SipResourceList.h>
#include <net/SipSubscribeClient.h>
#include <net/SipUserAgent.h>

#define OUTPUT_PREFIX "[start of body]\n"
#define OUTPUT_SUFFIX "\n[end of body]\n"

// Default event type is 'dialog'.
static const char default_event_type[] = DIALOG_EVENT_TYPE;

// The default content types to allow.
// This list is assembled to easily support dialog events, dialog-list
// events, and reg events.
static const char default_content_type[] =
   DIALOG_EVENT_CONTENT_TYPE ","
   CONTENT_TYPE_MULTIPART_RELATED ","
   RESOURCE_LIST_CONTENT_TYPE ","
   REG_EVENT_CONTENT_TYPE ","
   PRESENCE_EVENT_CONTENT_TYPE;

void subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                               const char* earlyDialogHandle,
                               const char* dialogHandle,
                               void* applicationData,
                               int responseCode,
                               const char* responseText,
                               long expiration,
                               const SipMessage* subscribeResponse)
{
   fprintf(stderr,
           "subscriptionStateCallback is called with responseCode = %d (%s)\n",
           responseCode, responseText);
   // If an error reponse, terminate.
   if (!((responseCode >= 100 && responseCode <= 299) || responseCode == -1))
   {
      exit(0);
   }
}


// Callback to handle incoming NOTIFYs.
bool notifyEventCallback(const char* earlyDialogHandle,
                         const char* dialogHandle,
                         void* applicationData,
                         const SipMessage* notifyRequest)
{
   fprintf(stderr,
           "notifyEventCallback called with early handle '%s' handle '%s' message:\n",
           earlyDialogHandle, dialogHandle);
   if (notifyRequest)
   {
      const HttpBody* notifyBody = notifyRequest->getBody();
      fprintf(stdout, OUTPUT_PREFIX);
      if (notifyBody)
      {
         UtlString messageContent;
         ssize_t bodyLength;
         notifyBody->getBytes(&messageContent, &bodyLength);
         fprintf(stdout, "%s", messageContent.data());
      }
      fprintf(stdout, OUTPUT_SUFFIX);
      // Make sure the event notice is written promptly.
      fflush(stdout);
   }
   return true;
}


void usage(const char* szExecutable)
{
    fprintf(stderr,
            "\nUsage: %s [-p localPort] [-e expiration]\n"
            "          [-C realm user password]\n"
            "          target-URI [event-type [content-type]]\n"
            "\n"
            "    localPort defaults to 0; select ephemeral port\n"
            "    expiration defaults to 300 (seconds)\n"
            "    -C provides authentication credentials\n"
            "    event-type defaults to '%s'\n"
            "    content-type defaults to '%s',\n"
            "        which supports dialog, dialog-list, reg, and presence events\n",
            szExecutable, default_event_type, default_content_type);
}

// Parse arguments
bool parseArgs(int argc,
               char*  argv[],
               int*   pPort,
               int*   pExpiration,
               char** ppTargetURI,
               char** ppEventType,
               char** ppContentType,
               char** ppRealm,
               char** ppUser,
               char** ppPassword)
{
    bool bRC = false;

    enum NEXT_ARGUMENT
    {
        NA_TARGET_URI,
        NA_EVENT_TYPE,
        NA_CONTENT_TYPE,
        NA_DONE
    } nextArg = NA_TARGET_URI;

    assert(pPort && pExpiration && ppTargetURI && ppEventType && ppContentType);

    *pPort = 0;
    *pExpiration = 300;
    *ppTargetURI = NULL;
    *ppEventType = NULL;
    *ppContentType = NULL;
    *ppRealm = NULL;
    *ppUser = NULL;
    *ppPassword = NULL;

    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            if ((i+1) < argc)
            {
                *pPort = atoi(argv[++i]);
            }
            else
            {
                fprintf(stderr, "missing port value after -p\n");
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            if ((i+1) < argc)
            {
                *pExpiration = atoi(argv[++i]);
            }
            else
            {
                fprintf(stderr, "missing expiration value after -e\n");
                break ; // Error
            }
        }
        else if (strcmp(argv[i], "-C") == 0)
        {
            if ((i+3) < argc)
            {
               *ppRealm = strdup(argv[++i]);
               *ppUser = strdup(argv[++i]);
               *ppPassword = strdup(argv[++i]);
            }
            else
            {
               fprintf(stderr, "Insufficient number of values after -C\n");
               break ; // Error
            }
        }
        else if (argv[i][0] == '-')
        {
           fprintf(stderr, "Invalid option: %s\n", argv[i]);
           break;
        }
        else
        {
            switch (nextArg)
            {
            case NA_TARGET_URI:
                *ppTargetURI = strdup(argv[i]);
                nextArg = NA_EVENT_TYPE ;
                bRC = true;
                break ;
            case NA_EVENT_TYPE:
                *ppEventType = strdup(argv[i]);
                nextArg = NA_CONTENT_TYPE ;
                break ;
            case NA_CONTENT_TYPE:
                *ppContentType = strdup(argv[i]);
                nextArg = NA_DONE ;
                break ;
            default:
                bRC = false ;
                fprintf(stderr, "Unexpected argument %s\n", argv[i]) ;
                break ;
            }
        }
    }

    if (*ppEventType == NULL)
        *ppEventType = strdup(default_event_type);

    if (*ppContentType == NULL)
        *ppContentType = strdup(default_content_type);

    return bRC ;
}



int main(int argc, char* argv[])
{
    int port;
    int expiration;
    char* targetURI;
    char* eventType;
    char* contentType;
    char* realm;
    char* user;
    char* password;

   // Initialize logging.
   OsSysLog::initialize(0, "test");
   OsSysLog::setOutputFile(0, "log");
   OsSysLog::setLoggingPriority(PRI_DEBUG);
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

   if (!parseArgs(argc, argv, &port, &expiration,
                  &targetURI, &eventType, &contentType,
                  &realm, &user, &password))
   {
       usage(argv[0]);
       exit(1);
   }

   // The domain name to call myself, obtained from the gethostname()
   // system call.
   char buffer[100];
   memset(buffer, 0, sizeof (buffer));
   // Do not allow gethostname to write over the last NUL in buffer[].
   gethostname(buffer, (sizeof (buffer)) - 1);
   UtlString myDomainName(buffer, strlen(buffer));
   // Use "example.com" if gethostname() failed.
   if (myDomainName.isNull())
   {
      myDomainName = "example.com";
   }

   UtlString fromString = "sip:dialogwatch@" + myDomainName;
   Url fromUri(fromString, TRUE);

   // Create the SIP Subscribe Client

   SipLineMgr lineMgr;

   // Add credentials if they were specified.
   if (realm)
   {
      UtlString id;
      id += "sip:";
      id += user;
      id += "@";
      id += realm;
      Url identity(id, TRUE);

      SipLine line( fromUri  // user entered url
                   ,identity // identity url
                   ,user     // user
                   ,TRUE     // visible
                   ,SipLine::LINE_STATE_PROVISIONED
                   ,TRUE     // auto enable
                   ,FALSE    // use call handling
         );

      lineMgr.addLine(line);

      UtlString cred_input;
      UtlString cred_digest;

      cred_input.append(user);
      cred_input.append(":");
      cred_input.append(realm);
      cred_input.append(":");
      cred_input.append(password);

      NetMd5Codec::encode(cred_input.data(), cred_digest);

      fprintf(stderr,
              "Adding identity '%s': user='%s' realm='%s' password='%s' H(A1)='%s'\n",
              identity.toString().data(), user, realm, password, cred_digest.data()
         );

      assert(lineMgr.addCredentialForLine(identity, realm, user, cred_digest,
                                          HTTP_DIGEST_AUTHENTICATION));
   }

   SipUserAgent* pSipUserAgent =
      new SipUserAgent(port, port, PORT_NONE,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       &lineMgr);
   // Add the 'eventlist' extension, so dialogwatch can subscribe to
   // event lists.
   pSipUserAgent->allowExtension("eventlist");
   if (!pSipUserAgent->isOk())
   {
       fprintf(stderr, "Unable to bind to port %d\n", port);
       exit(1);
   }

   SipDialogMgr dialogManager;

   SipRefreshManager refreshMgr(*pSipUserAgent, dialogManager);
   refreshMgr.start();

   SipSubscribeClient sipSubscribeClient(*pSipUserAgent, dialogManager,
                                         refreshMgr);
   sipSubscribeClient.start();

   // Construct a name-addr from targetURI, in case it contains parameters.
   UtlString toUri;
   toUri = "<";
   toUri += targetURI;
   toUri += ">";
   UtlString earlyDialogHandle;

   fprintf(stderr,
           "resourceId '%s' fromString '%s' toUri '%s' event '%s' content-type '%s' port=%d expiration=%d\n",
           targetURI, fromString.data(), toUri.data(), eventType, contentType,
           port, expiration);

   UtlBoolean status =
      sipSubscribeClient.addSubscription(targetURI,
                                         eventType,
                                         contentType,
                                         fromString.data(),
                                         toUri.data(),
                                         NULL,
                                         expiration,
                                         (void *) NULL,
                                         subscriptionStateCallback,
                                         notifyEventCallback,
                                         earlyDialogHandle);

   if (!status)
   {
      fprintf(stderr, "Subscription attempt failed.\n");
      exit(1);
   }
   else
   {
      fprintf(stderr, "Subscription attempt succeeded.  Handle: '%s'\n",
              earlyDialogHandle.data());
   }

   while (1)
   {
      sleep(1000);
   }
}
