// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdio.h>

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipDialogEvent.h>
#include <net/SipUserAgent.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>


#define OUTPUT_PREFIX "[start of body]\n"
#define OUTPUT_SUFFIX "\n[end of body]\n"

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
void notifyEventCallback(const char* earlyDialogHandle,
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
         int bodyLength;
         notifyBody->getBytes(&messageContent, &bodyLength);
         fprintf(stdout, "%s", messageContent.data());
      }
      fprintf(stdout, OUTPUT_SUFFIX);
      // Make sure the event notice is written promptly.
      fflush(stdout);
   }
}


int main(int argc, char* argv[])
{
   // Initialize logging.
   OsSysLog::initialize(0, "test");
   OsSysLog::setOutputFile(0, "log");
   OsSysLog::setLoggingPriority(PRI_DEBUG);

   if (argc == 1 || argc == 3)
   {
      fprintf(stderr, "Usage: %s target-URI [event-type content-type]\n",
              argv[0]);
      exit(1);
   }

   // The URI to subscribe to.
   UtlString resourceId = argv[1];

   // The event type.
   const char* event_type =
      argc >= 4 ? argv[2] : DIALOG_EVENT_TYPE;
   // The content type.
   const char* content_type =
      argc >= 4 ? argv[3] : DIALOG_EVENT_CONTENT_TYPE;

   // Seconds to set for subscription.
   int refreshTimeout = 300;

   // The domain name to call myself.
   UtlString myDomainName = "example.com";

   // Create the SIP Subscribe Client

   SipUserAgent* pSipUserAgent = 
      new SipUserAgent(PORT_DEFAULT, PORT_DEFAULT, PORT_NONE);

   SipDialogMgr dialogManager;

   SipRefreshManager refreshMgr(*pSipUserAgent, dialogManager);
   refreshMgr.start();

   SipSubscribeClient sipSubscribeClient(*pSipUserAgent, dialogManager,
                                         refreshMgr);
   sipSubscribeClient.start();  

   UtlString toUri(resourceId);
   UtlString fromUri = "dialogwatch@" + myDomainName;
   UtlString earlyDialogHandle;
            
   fprintf(stderr,
           "resourceId '%s' fromUri '%s' toUri '%s' event '%s' content-type '%s'\n",
           resourceId.data(), fromUri.data(), toUri.data(), event_type,
           content_type);

   UtlBoolean status =
      sipSubscribeClient.addSubscription(resourceId.data(),
                                         event_type,
                                         content_type,
                                         fromUri.data(),
                                         toUri.data(),
                                         NULL,
                                         refreshTimeout,
                                         (void *) NULL,
                                         subscriptionStateCallback,
                                         notifyEventCallback,
                                         earlyDialogHandle);
               
   if (!status)
   {
      fprintf(stderr, "Subscription attempt failed.\n");
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
