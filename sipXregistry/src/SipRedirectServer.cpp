//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "net/SipUserAgent.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/PermissionDB.h"
#include "registry/SipRedirectServer.h"
#include "RedirectResumeMsg.h"
#include "registry/RedirectSuspend.h"
#include "utl/PluginHooks.h"
#include "registry/RedirectPlugin.h"

// DEFINES
#define LOWEST_AUTHORITY_LEVEL   (0)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SipRedirectServer::AuthorityLevelPrefix  = "_AUTHORITY_LEVEL";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// STATIC VARIABLE INITIALIZATIONS
SipRedirectServer* SipRedirectServer::spInstance = NULL;

// Constructor
SipRedirectServer::SipRedirectServer(OsConfigDb*   pOsConfigDb,  ///< Configuration parameters
                                     SipUserAgent* pSipUserAgent ///< User Agent to use when sending responses
                                     ) :
   OsServerTask("SipRedirectServer-%d", NULL, SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE),
   mRedirectorMutex(OsMutex::Q_FIFO),
   mIsStarted(FALSE),
   mpSipUserAgent(pSipUserAgent),
   mNextSeqNo(0),
   mRedirectPlugins(RedirectPlugin::Factory, RedirectPlugin::Prefix),
   mpConfiguredRedirectors(NULL)
{
   spInstance = this;
   initialize(*pOsConfigDb);
}

void SipRedirectServer::requestShutdown(void)
{
   /*
    * This is called from the SipRegistrar task destructor.
    * Use the shutdown message to wake up the SipRedirectServer task
    * so that we can clean up all the redirectors before actually
    * starting the shutdown of the task
    */
   OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

   postMessage(msg);
   yield(); // make the caller wait so that the task can run.
}

// Destructor
SipRedirectServer::~SipRedirectServer()
{
   waitUntilShutDown();
}

// Get the unique instance, if it exists.  Will not create it, though.
SipRedirectServer*
SipRedirectServer::getInstance()
{
    return spInstance;
}

UtlBoolean
SipRedirectServer::initialize(OsConfigDb& configDb
                              ///< Configuration parameters
                              )
{
   configDb.get("SIP_REGISTRAR_DOMAIN_NAME", mDefaultDomain);

   mProxyNormalPort = configDb.getPort("SIP_REGISTRAR_PROXY_PORT");
   if (mProxyNormalPort == PORT_DEFAULT)
   {
      mProxyNormalPort = SIP_PORT;
   }
   mAckRouteToProxy.insert(0, "<");
   mAckRouteToProxy.append(mDefaultDomain);
   mAckRouteToProxy.append(";lr>");

   // Load the list of redirect processors.
   mRedirectPlugins.readConfig(configDb);
   mRedirectorCount = mRedirectPlugins.entries();

   // Call their ::initialize() methods.
   mpConfiguredRedirectors = new RedirectorDescriptor[ mRedirectorCount ];
   PluginIterator iterator(mRedirectPlugins);
   RedirectPlugin* redirector;
   UtlString redirectorName;
   bool bAuthorityLevelDbAvailable;
   UtlString authorityLevelDbPrefix = RedirectPlugin::Prefix;
   authorityLevelDbPrefix.append( AuthorityLevelPrefix );
   authorityLevelDbPrefix.append( '.' );
   OsConfigDb authorityLevelDb;

   bAuthorityLevelDbAvailable = ( configDb.getSubHash( authorityLevelDbPrefix, authorityLevelDb ) == OS_SUCCESS );
   int i;       // Iterator sequence number.

   for (i = 0; (redirector = static_cast <RedirectPlugin*> (iterator.next( &redirectorName )));
        i++)
   {
      mpConfiguredRedirectors[i].name = redirectorName;
      if( ( mpConfiguredRedirectors[i].bActive =
             ( redirector->initialize(configDb, i, mDefaultDomain) == OS_SUCCESS ) ) )
      {
         int authorityLevel;
         if( bAuthorityLevelDbAvailable         &&
             authorityLevelDb.get( redirectorName, authorityLevel ) == OS_SUCCESS )
         {
            mpConfiguredRedirectors[i].authorityLevel = authorityLevel;
         }
         else
         {
            mpConfiguredRedirectors[i].authorityLevel = LOWEST_AUTHORITY_LEVEL;
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::initialize "
                       "Initialized redirector %s (authority level = %zd)", redirectorName.data(), mpConfiguredRedirectors[i].authorityLevel );
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::initialize "
                       "Redirector %s is inactive ", redirectorName.data() );
      }
   }
   return true;
}

/**
 * Cancel a suspended redirection.
 *
 * Caller must hold mRedirectorMutex.
 *
 * containableSeqNo - UtlInt containing the sequence number.
 *
 * suspendObject - pointer to the suspense object.
 */
void SipRedirectServer::cancelRedirect(UtlInt& containableSeqNo,
                                       RedirectSuspend* suspendObject)
{
   RedirectPlugin::RequestSeqNo seqNo = containableSeqNo.getValue();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectServer::cancelRedirect "
                 "Canceling suspense of request %d", seqNo);
   // Call cancel for redirectors that need it.
   PluginIterator iterator(mRedirectPlugins);
   RedirectPlugin* redirector;
   int i;                       // Iterator sequence number.
   for (i = 0; (redirector = static_cast <RedirectPlugin*> (iterator.next()));
        i++)
   {
      if (mpConfiguredRedirectors[i].bActive &&
          suspendObject->mRedirectors[i].needsCancel)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectServer::cancelRedirect "
                       "Calling cancel(%d) for redirector %d", seqNo, i);
         redirector->cancel(seqNo);
      }
   }
   // Remove the entry from mSuspendList.
   // Also deletes the suspend object.
   // Deleting the suspend object frees the array of information about
   // the redirectors, and the private storage for each redirector.
   // (See RedirectSuspend::~RedirectSuspend().)
   mSuspendList.destroy(&containableSeqNo);
}

/**
 * Process a redirection.  The caller sets up our processing, but we
 * carry it through to the generation of the response, queuing of the
 * suspend object, etc.  We send the response if we finish processing.
 *
 * message is the message to be processed.  Its memory is owned by our
 * caller, or is attached to the suspend object.
 *
 * method is the request's SIP method.  Its memory is owned by our caller.
 *
 * seqNo is the sequence number to be used for this request.  (If this
 * request is to be suspended and seqNo == mNextSeqNo, we must increment
 * mNextSeqNo.)
 *
 * suspendObject is the suspend object for this request (if it already
 * exists) or NULL.  It is passed as an argument to avoid attempting to
 * look it up if the caller knows that it does not exist (because this
 * is a first processing attempt).
 */
void SipRedirectServer::processRedirect(const SipMessage* pMessage,
                                        UtlString& method,
                                        RedirectPlugin::RequestSeqNo seqNo,
                                        RedirectSuspend* suspendObject)
{
   ErrorDescriptor errorDescriptor;

   // Extract the request URI.
   UtlString stringUri;
   pMessage->getRequestUri(&stringUri);
   // The requestUri is an addr-spec, not a name-addr.
   Url requestUri(stringUri, TRUE);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectServer::processRedirect "
                 "Starting to process request URI '%s'",
                 stringUri.data());

   /*
    * Normalize the port in the Request URI
    *   This is not strictly kosher, but it solves interoperability problems.
    *   Technically, user@foo:5060 != user@foo , but many implementations
    *   insist on including the explicit port even when they should not, and
    *   it causes registration mismatches, so we normalize the URI when inserting
    *   and looking up in the database so that if explicit port is the same as
    *   the proxy listening port, then we remove it.
    *   (Since our proxy has mProxyNormalPort open, no other SIP entity
    *   can use sip:user@domain:mProxyNormalPort, so this normalization
    *   cannot interfere with valid addresses.)
    *
    * For the strict rules, set the configuration parameter
    *   SIP_REGISTRAR_PROXY_PORT : PORT_NONE
    */
   if (   mProxyNormalPort != PORT_NONE
       && requestUri.getHostPort() == mProxyNormalPort
       )
   {
      requestUri.setHostPort(PORT_NONE);
   }

   // Seize the lock that protects the list of suspend objects.
   OsLock lock(mRedirectorMutex);

   // Process with the redirectors.
   // Set to TRUE if any of the redirectors requests suspension.
   UtlBoolean willSuspend = FALSE;
   // Set to TRUE if any of the redirectors requests an error response.
   UtlBoolean willError = FALSE;
   PluginIterator iterator(mRedirectPlugins);
   RedirectPlugin* redirector;
   // Authority level of the last redirector to have modified the contact list.
   ssize_t contactListAuthorityLevel = LOWEST_AUTHORITY_LEVEL;
   ContactList contactList( stringUri );

   int i;                       // Iterator sequence number.
   for (i = 0; (redirector = static_cast <RedirectPlugin*> (iterator.next())) && !willError;
        i++)
   {
      if (mpConfiguredRedirectors[i].bActive)
      {
         // verify if the redirector has a suitable authority level to perform a look-up
         if( mpConfiguredRedirectors[i].authorityLevel >= contactListAuthorityLevel )
         {
            // Place to store the private storage pointer.
            SipRedirectorPrivateStorage* privateStorageP;
            // Initialize it.
            privateStorageP = (suspendObject ?
                               suspendObject->mRedirectors[i].privateStorage :
                               NULL);

            // Call the redirector to process the request.
            contactList.resetWasModifiedFlag();
            RedirectPlugin::LookUpStatus status =
               redirector->lookUp(*pMessage, stringUri, requestUri, method,
                                  contactList, seqNo, i, privateStorageP, errorDescriptor);

            // Create the suspend object if it does not already exist and we need it.
            if (!suspendObject &&
                (status == RedirectPlugin::SEARCH_PENDING || privateStorageP))
            {
               suspendObject = new RedirectSuspend(mRedirectorCount);
               // Insert it into mSuspendList, keyed by seqNo.
               UtlInt* containableSeqNo = new UtlInt(seqNo);
               mSuspendList.insertKeyAndValue(containableSeqNo, suspendObject);
               // Save in it a copy of the message.  (*pMessage is
               // dependent on the OsMsg bringing the message to us, and
               // will be freed when we are done with that OsMsg.)
               suspendObject->mMessage = *pMessage;
               // Use the next sequence number for the next request.
               if (seqNo == mNextSeqNo)
               {
                  // Increment to the next value (and roll over if necessary).
                  mNextSeqNo++;
               }
            }
            // Store the private storage pointer.
            if (suspendObject)
            {
               suspendObject->mRedirectors[i].privateStorage = privateStorageP;
            }

            int statusCode;
            UtlString reasonPhrase;

            // Dispatch on status.
            switch (status)
            {
            case RedirectPlugin::SUCCESS:
               // Processing was successful. If the plug-in modified the Contact list then
               // raise the authority level required to modify the contact list to the
               // authority level of this plug-in.
               if( contactList.wasListModified() )
               {
                  contactListAuthorityLevel = mpConfiguredRedirectors[i].authorityLevel;
               }
               break;

            case RedirectPlugin::ERROR:
               // Processing detected an error.  Log it and set the 'error' flag.
               errorDescriptor.getStatusLineData( statusCode, reasonPhrase );

               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipRedirectServer::processRedirect "
                             "ERROR returned by redirector "
                             "'%s' while processing method '%s' URI '%s': "
                             "Status code = %d (%s)",
                             redirector->name().data(), method.data(), stringUri.data(), statusCode, reasonPhrase.data() );
               willError = TRUE;
               break;

            case RedirectPlugin::SEARCH_PENDING:
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRedirectServer::processRedirect "
                             "SEARCH_PENDING returned by redirector "
                             "'%s' while processing method '%s' URI '%s'",
                             redirector->name().data(), method.data(), stringUri.data());
               willSuspend = TRUE;
               // Mark that this redirector has requested suspension.
               suspendObject->mRedirectors[i].suspended = TRUE;
               suspendObject->mRedirectors[i].needsCancel = TRUE;
               suspendObject->mSuspendCount++;
               break;

            default:
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipRedirectServer::processRedirect "
                             "Invalid status value %d returned by redirector "
                             "'%s' while processing method '%s' URI '%s'",
                             status, redirector->name().data(), method.data(), stringUri.data());
               break;
            }  // end status switch
         } // end authorityLevel >= contactListAuthorityLevel
         else
         {
            // redirector plug-in does not have a suitable authority level to look up the
            // request - it is only allowed to observe it.
            redirector->observe(*pMessage, stringUri, requestUri, method,
                               contactList, seqNo, i);
         }
      }  // end mpConfiguredRedirectors[i]
   }  // end Redirector plug-in iterator

   if (willError || !willSuspend)   // redirector has done all it can
   {
       int numContacts = 0;
       if (method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) == 0)
       {
          // See if location server has returned an address to use
          // to forward or just drop the ACK here.
           if (!willError && ((numContacts = contactList.entries()) == 1))
           {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRedirectServer::processRedirect "
                             "Forwarding ACK for URI '%s': ",
                             stringUri.data());

               UtlString contactUri;
               UtlString routeEntries;
               int maxForwards;

               contactList.get(0, contactUri);

               // create the message to forward -
               // change reqUri to located value, add route for debugging info, decrement max-forwards
               // leave last via since we are circling back, not responding
               SipMessage ackCopy(*pMessage);    // "clone" original ACK
               ackCopy.setRequestFirstHeaderLine(SIP_ACK_METHOD, contactUri, SIP_PROTOCOL_VERSION);
               routeEntries.insert(0, mAckRouteToProxy);        // put route proxy in first position for informational purposes
               ackCopy.setRouteField(routeEntries.data());      // into forwarded ack
               if(!ackCopy.getMaxForwards(maxForwards))
               {
                   maxForwards = SIP_DEFAULT_MAX_FORWARDS;
               }
               maxForwards--;
               ackCopy.setMaxForwards(maxForwards);

               // get send-to address/port/protocol from top via
               UtlString lastViaAddress;
               int lastViaPort;
               UtlString lastViaProtocol;
               OsSocket::IpProtocolSocketType viaProtocol = OsSocket::UNKNOWN;

               ackCopy.getTopVia(&lastViaAddress, &lastViaPort, &lastViaProtocol);
               SipMessage::convertProtocolStringToEnum(lastViaProtocol.data(), viaProtocol);


               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRedirectServer::processRedirect "
                             "sending ACK to '%s':%d using '%s'"
                             "location service returned '%s'",
                             lastViaAddress.data(),
                             lastViaPort,
                             lastViaProtocol.data(),
                             contactUri.data());

               // This ACK has no matching INVITE, special case
               mpSipUserAgent->sendStatelessAck(ackCopy,            // this will add via
                                                lastViaAddress,
                                                lastViaPort,
                                                viaProtocol);
           }
           else  // locater must return EXACTLY one value to forward
           {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipRedirectServer::processRedirect "
                             "Cannot redirect ACK for URI '%s', dropping: "
                             "number of contacts=%d  ",
                             stringUri.data(), numContacts);
           }
       }   // end handle ACK redirection
       else     // build and send the proper (error) response
       {
          // The response we will compose and, hopefully, send.
          SipMessage response;

          // Send a response and terminate processing now if an error has
          // been found or if no redirector has requested suspension.
          if (willError)
          {
             buildResponseFromRequestAndErrorDescriptor( response, *pMessage, errorDescriptor );
          }
          else
          {
             // If request processing is finished, construct a response,
             // either 302 or 404.
             if (contactList.entries() > 0)
             {
                // There are contacts, so send a 302 Moved Temporarily.
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipRedirectServer::processRedirect "
                              "Contacts added, sending 302 response");
                response.setResponseData(pMessage,
                                         SIP_TEMPORARY_MOVE_CODE,
                                         SIP_TEMPORARY_MOVE_TEXT);

                // add contacts collected in the contactList to the response
                size_t index;
                size_t numEntries = contactList.entries();

                for( index = 0; index < numEntries; index++ )
                {
                   UtlString contactString;
                   if( contactList.get( index, contactString ) )
                   {
                      response.setContactField(contactString, index );
                   }
                   else
                   {
                      OsSysLog::add(FAC_SIP, PRI_CRIT,
                                    "SipRedirectServer::processRedirect "
                                    "Failed to retrieve contact index %zu", index );
                   }
                }
             }
             else
             {
                // There are no contacts, send back a 404 Not Found.
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipRedirectServer::processRedirect "
                              "No contacts added, sending 404 response");
                response.setResponseData(pMessage,
                                         SIP_NOT_FOUND_CODE,
                                         SIP_NOT_FOUND_TEXT);
             }
          }

          // Identify ourselves in the response
          mpSipUserAgent->setUserAgentHeader(response);

          // Now that we've set the right code into the response, send the
          // response.
          mpSipUserAgent->send(response);
       }    // end sending response to Invite

       // If the suspend object exists, remove it from mSuspendList
       // and delete it.
       if (suspendObject)
       {
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipRedirectServer::processRedirect "
                        "Cleaning up suspense of request %d", seqNo);
          UtlInt containableSeqNo(seqNo);
          cancelRedirect(containableSeqNo, suspendObject);
       }
   }    // end redirector did all it could
   else
   {
      // Request is suspended.
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectServer::processRedirect "
                    "Suspending request %d", seqNo);
   }
}

UtlBoolean
SipRedirectServer::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean handled = FALSE;
   int msgType = eventMessage.getMsgType();

   switch (msgType)
   {
   case OsMsg::PHONE_APP:
   {
      // An incoming request to be redirected.

      // Get a pointer to the SIP message.
      const SipMessage* message =
         ((SipMessageEvent&) eventMessage).getMessage();

      // Extract the request method.
      UtlString method;
      message->getRequestMethod(&method);

      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         UtlString stringUri;
         message->getRequestUri(&stringUri);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                       "Start processing redirect message %d: '%s' '%s'",
                       mNextSeqNo, method.data(), stringUri.data());
      }

      if (method.compareTo(SIP_CANCEL_METHOD, UtlString::ignoreCase) == 0)
      {
         // For CANCEL.
         // If we have a suspended request with this Call-Id, cancel it.
         // Send a 200 response.

         // Cancel the suspended request.
         UtlString cancelCallId;
         message->getCallIdField(&cancelCallId);

         {
            // Seize the lock that protects the list of suspend objects.
            OsLock lock(mRedirectorMutex);

            // Look for a suspended request that had this Call-Id.
            UtlHashMapIterator itor(mSuspendList);

            // Fetch a pointer to each suspend object.
            while (itor())
            {
               RedirectSuspend* suspend_object =
                  dynamic_cast<RedirectSuspend*> (itor.value());

               // Is this a request to which the CANCEL applies?
               if (suspend_object->mMessage.isInviteFor(message))
               {
                  // Send a 487 response to the original request.
                  SipMessage response;
                  response.setResponseData(&suspend_object->mMessage,
                                           SIP_REQUEST_TERMINATED_CODE,
                                           SIP_REQUEST_TERMINATED_TEXT);
                  mpSipUserAgent->send(response);

                  // Cancel the redirection.
                  // (After we are done using suspend_object->mMessage
                  // to generate the response.)
                  UtlInt requestNo = *dynamic_cast<UtlInt*> (itor.key());
                  cancelRedirect(requestNo, suspend_object);
               }
            }
         }
         // We do not need to send a 200 for the CANCEL, as the stack does that
         // for us.  (And will eat a 200 that we generate, it seems!)
      }
      else
      {
         // For all methods other than CANCEL:
         // Note: any ACK that gets passed up to the redirector
         // needs to be processed and forwarded if possible.
         // ACKs for the error responses sent from the redirector
         // are recognized by the stack and not passed to this application
         //
         // Call processRedirect to call the redirectors, and handle
         // their results, send a response or suspend processing of
         // the request.
         // For ACKs, no response is sent(or allowed).  Instead, the ACK is routed
         // back to the proxy with information that allows it to be sent
         // to the correct next hop (ReqUri is replaced).

         // Assign mNextSeqNo as the sequence number for this request.
         // If this request needs to be suspended, processRedirect will
         // increment mNextSeqNo so that value will not be reused (soon).
         // Initially, the suspendObject is NULL.
         processRedirect(message, method, mNextSeqNo, (RedirectSuspend*) 0);
      }
      handled = TRUE;
   }
   break;

   case RedirectResumeMsg::REDIRECT_RESTART:
   {
      // A message saying that a redirector is now willing to resume
      // processing of a request.
      // Get the redirector and sequence number.
      const RedirectResumeMsg* msg =
         dynamic_cast<RedirectResumeMsg*> (&eventMessage);
      RedirectPlugin::RequestSeqNo seqNo = msg->getRequestSeqNo();
      int redirectorNo = msg->getRedirectorNo();
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                    "Resume for redirector %d request %d",
                    redirectorNo, seqNo);

      // Look up the suspend object.
      UtlInt containableSeqNo(seqNo);
      RedirectSuspend* suspendObject =
         dynamic_cast<RedirectSuspend*>
         (mSuspendList.findValue(&containableSeqNo));

      // If there is no request with this sequence number, ignore the message.
      if (!suspendObject)
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRedirectServer::handleMessage No suspended request "
                       "with seqNo %d",
                       seqNo);
         break;
      }

      // Check that this redirector is suspended.
      if (redirectorNo < 0 || redirectorNo >= mRedirectorCount)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectServer::handleMessage "
                       "Invalid redirector %d",
                       redirectorNo);
         break;
      }
      if (!suspendObject->mRedirectors[redirectorNo].suspended)
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRedirectServer::handleMessage Redirector %d is "
                       "not suspended for seqNo %d",
                       redirectorNo, seqNo);
         break;
      }

      // Mark this redirector as no longer wanting suspension.
      suspendObject->mRedirectors[redirectorNo].suspended = FALSE;
      suspendObject->mSuspendCount--;
      // If no more redirectors want suspension, reprocess the request.
      if (suspendObject->mSuspendCount == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::handleMessage "
                       "Start reprocessing request %d", seqNo);

         // Get a pointer to the message.
         const SipMessage* message = &suspendObject->mMessage;

         // Extract the request method.
         UtlString method;
         message->getRequestMethod(&method);

         processRedirect(message, method, seqNo, suspendObject);
      }
      handled = TRUE;
   }
   break;

   case OsMsg::OS_SHUTDOWN:
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectServer::handleMessage received shutdown request"
                    );

      // Seize the lock that protects the list of suspend objects.
      OsLock lock(mRedirectorMutex);

      // Cancel all suspended requests.
      UtlHashMapIterator itor(mSuspendList);
      UtlInt* key;
      while ((key = dynamic_cast<UtlInt*> (itor())))
      {
         cancelRedirect(*key, dynamic_cast<RedirectSuspend*>(itor.value()));
      }

      // Finalize and delete all the redirectors.
      PluginIterator redirectors(mRedirectPlugins);
      RedirectPlugin* redirector;
      while ((redirector = dynamic_cast<RedirectPlugin*>(redirectors.next())))
      {
         redirector->finalize();
         delete redirector;
      }

      spInstance = NULL;
      OsTask::requestShutdown(); // tell OsServerTask::run to exit
      handled = TRUE;
   }
   break;

   default:
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipRedirectServer::handleMessage unhandled msg type %d",
                    msgType
                    );
   }
   }

   return handled;
}

void
SipRedirectServer::resumeRequest(RedirectPlugin::RequestSeqNo requestSeqNo,
                                 int redirectorNo)
{
   // Create the appropriate message.
   RedirectResumeMsg message = RedirectResumeMsg(requestSeqNo, redirectorNo);

   // Send the message to the redirect server.
   // Note that send() copies its argument.
   getMessageQueue()->send(message);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRedirectServer::resumeRequest "
                 "Redirector %d sent message to resume request %d",
                 redirectorNo, requestSeqNo);
}

SipRedirectorPrivateStorage*
SipRedirectServer::getPrivateStorage(
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo)
{
   // Turn the request number into a UtlInt.
   UtlInt containableSeqNo(requestSeqNo);
   // Look up the suspend object.
   RedirectSuspend* suspendObject =
      dynamic_cast<RedirectSuspend*>
      (mSuspendList.findValue(&containableSeqNo));
   // Get the private storage pointer.
   return suspendObject->mRedirectors[redirectorNo].privateStorage;
}

void SipRedirectServer::buildResponseFromRequestAndErrorDescriptor( SipMessage& response,
                                                                    const SipMessage& request,
                                                                    const ErrorDescriptor& errorDescriptor )
{
    int code;
    UtlString textValue;
    errorDescriptor.getStatusLineData( code, textValue );

    // set status line and basic fields from request
    response.setResponseData( &request,
                              code,
                              textValue );

    // apply warning header if warning data was specified
    if( errorDescriptor.getWarningData( code, textValue ) )
    {
       response.setWarningField( code, mDefaultDomain, textValue );
    }

    // add copy of offending request as sipfrag body is requested
    if( errorDescriptor.shouldRequestBeAppendedToResponse() )
    {
       response.setRequestDiagBody( request );
    }

    if( errorDescriptor.getAcceptFieldValue( textValue ) )
    {
       response.setAcceptField( textValue );
    }

    if( errorDescriptor.getAcceptEncodingFieldValue( textValue ) )
    {
       response.setAcceptEncodingField( textValue );
    }

    if( errorDescriptor.getAcceptLanguageFieldValue( textValue ) )
    {
       response.setAcceptLanguageField( textValue );
    }

    if( errorDescriptor.getAllowFieldValue( textValue ) )
    {
       response.setAllowField( textValue );
    }

    if( errorDescriptor.getRequireFieldValue( textValue ) )
    {
       response.setRequireField( textValue );
    }

    if( errorDescriptor.getRetryAfterFieldValue( textValue ) )
    {
       response.setRetryAfterField( textValue );
    }

    if( errorDescriptor.getUnsupportedFieldValue( textValue ) )
    {
       response.setUnsupportedField( textValue );
    }
}

SipRedirectServerPrivateStorageIterator::
SipRedirectServerPrivateStorageIterator(int redirectorNo) :
   UtlHashMapIterator(SipRedirectServer::getInstance()->mSuspendList),
   mRedirectorNo(redirectorNo)
{
}

SipRedirectorPrivateStorage*
SipRedirectServerPrivateStorageIterator::operator()()
{
   SipRedirectorPrivateStorage* pStorage = NULL;
   // Step the iterator until we find a member which has a non-NULL pointer
   // to private storage for redirector mRedirectorNo.
// :BUG: Known to be incorrect
//   while ((*((UtlIterator*) this))() &&
//          (pStorage =
//           (dynamic_cast<RedirectSuspend*> (this->value()))->
//           mRedirectors[mRedirectorNo].privateStorage))
//   {
//   }
   while (1)
   {
      // Step the iterator using UtlHashMapIterator's step function.
      if (this->UtlHashMapIterator::operator()() ==
          NULL)
      {
         break;
      }
      pStorage =
         (dynamic_cast<RedirectSuspend*> (this->value()))->
         mRedirectors[mRedirectorNo].privateStorage;
      if ( pStorage != NULL )
      {
         break;
      }
   }
   // Return the pointer to the storage, or NULL if none was found.
   return pStorage;
}

RedirectPlugin::RequestSeqNo SipRedirectServerPrivateStorageIterator::requestSeqNo() const
{
   // The key is a UtlInt which is the request sequence number.
   return (dynamic_cast<UtlInt*> (this->key()))->getValue();
}
