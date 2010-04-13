//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"
#include "net/SipMessage.h"
#include "net/HttpRequestContext.h"
#include "xmlparser/tinyxml.h"
#include "statusserver/MwiPlugin.h"
#include "statusserver/Notifier.h"

// APPLICATION INCLUDES

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MwiPlugin::MwiPlugin (
    const UtlString& eventType,
    const TiXmlElement& pluginElement,
    Notifier* notifier) :
    mNotifier( notifier )
{
    // get from attibutes TODO - @JC hacked to work
    // around not reading from config file
    //mMinExpiresTimeStr.append( "300" );
    //mMinExpiresTimeint = 300;

    // get the attribules associated with the PlugIn type from the XML file

    // get permission associated with this plugin, if any
    TiXmlNode* nextNode = NULL;
    while ((nextNode = (TiXmlNode*)pluginElement.IterateChildren(nextNode)))
    {
        if( nextNode->Type() == TiXmlNode::ELEMENT )
        {
            UtlString tagValue = nextNode->Value();
            if( tagValue.compareTo( XML_TAG_PLUGIN_DATA, UtlString::ignoreCase ) == 0 )
            {
                // Special Plugin-Specific data is stored here
                TiXmlElement* pluginDataElement = nextNode->ToElement();
                TiXmlNode* pluginDataNode = pluginDataElement->FirstChild( XML_TAG_VOICEMAILCGI );
                if (pluginDataNode != NULL)
                {
                    TiXmlNode* pluginValueNode =
                        pluginDataNode->ToElement()->FirstChild();
                    mVoicemailCGIUrl = pluginValueNode->Value();
                }
            }
        }
    }
}

void
MwiPlugin::terminatePlugin()
{}

MwiPlugin::~MwiPlugin()
{
    if( mNotifier )
    {
        mNotifier = NULL;
    }

}

OsStatus
MwiPlugin::handleNotifyResponse (
    const SipMessage& request,
    const SipMessage& response)
{
    // Do nothing here
    return OS_SUCCESS;
}

OsStatus
MwiPlugin::handleSubscribeRequest (
    const SipMessage& message,
    SipMessage& response,
    const char* authenticatedUser,
    const char* authenticatedRealm,
    const char* domain )
{
    OsStatus result = OS_FAILED;
    // send the CGI request to the Voicemail Server which will
    // send a synchronous response to us in the body of the return
    UtlString userOrExtensionAtOptionalDomain = authenticatedUser;

    Url mailboxUrl;

    // See note below
    if ( userOrExtensionAtOptionalDomain.index ("@") != UTL_NOT_FOUND )
    {
        mailboxUrl = authenticatedUser;
    } else
    {
        mailboxUrl.setUserId( userOrExtensionAtOptionalDomain );
        mailboxUrl.setHostAddress( domain );
    }

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "MwiPlugin::handleSubscribeRequest() -"
        " Subscription for %s successfully added", mailboxUrl.toString().data());

    Url voicemailCGIUrl ( mVoicemailCGIUrl );

    UtlString mailboxIdentity;
    mailboxUrl.getIdentity(mailboxIdentity);

    // Note that although this should be the identity it can also
    // be the userid (extension@domain) ideally we should perform a DB
    // lookup here
    voicemailCGIUrl.setHeaderParameter(
        "identity",   mailboxIdentity.data() );
    voicemailCGIUrl.setHeaderParameter(
        "eventtype", "message-summary");

    // Synchronously call the Voicemail CGI to get the
    // message summary in its body contents. Set to use
    // persistent connections.
    HttpMessage httpResponse;
    int httpStatus = httpResponse.get( voicemailCGIUrl, 3*1000 /* 3 sec */, true /* persistent */);
    if ( HTTP_OK_CODE == httpStatus )
      {
        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
          {
            UtlString rspMsg;
            ssize_t rspLength;
            httpResponse.getBytes( &rspMsg, &rspLength );
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "MwiPlugin::handleSubscribeRequest() - voicemailCGI response:\n%s"
                          ,rspMsg.data()
                          );
          }

        UtlString rspContentType;
        httpResponse.getContentType( &rspContentType );
        if ( rspContentType.compareTo( CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY
                                      ,UtlString::ignoreCase
                                      ) == 0
            )
          {
            // get the body from the request containing the MWI summaries
            const HttpBody *pBody = httpResponse.getBody();
            if ( pBody )
              {
                ssize_t charsRead;
                UtlString buffer;

                pBody->getBytes(&buffer, &charsRead);
                httpResponse.getContentLength();

                // Skip over any leading blank lines
                const char* leading=buffer.data();
                int trimOffset;
                for ( trimOffset=0;
                      (   leading[trimOffset]==' '||leading[trimOffset]=='\n'
                       || leading[trimOffset]=='\t'||leading[trimOffset]=='\r'
                       );
                      trimOffset++
                     )
                {
                }
                if ( trimOffset )
                {   // Trim the leading white space
                   buffer = buffer( trimOffset,  buffer.length() -1 -trimOffset );
                }

                // quick sanity check of body content - does it look like it should?
                const char* MessagesWaiting = "Messages-Waiting";
                if ( buffer.index( MessagesWaiting, 0, UtlString::ignoreCase )
                    == 0
                    )
                  {
                    // create a simple message summary body for
                    // this subscription and send it via the user
                    // agent to the device
                    HttpBody* body = new HttpBody ( buffer.data(), buffer.length()
                                                   ,CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY
                                                   );
                    SipMessage notifyRequest;
                    notifyRequest.setBody( body );

                    // Add the content type for the body
                    notifyRequest.setContentType( CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY );
                    notifyRequest.setContentLength( buffer.length() );

                    // Send a MWI state change to just this subscriber
                    mNotifier->sendNotifyForSubscription( mailboxIdentity.data()
                                                         ,SIP_EVENT_MESSAGE_SUMMARY
                                                         ,message
                                                         ,notifyRequest
                                                         );
                    result = OS_SUCCESS;
                  }
                else
                  {
                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                                  "MwiPlugin::handleSubscribeRequest() - voicemailCGI response not valid: %s"
                                  ,buffer.data()
                                  );
                  }
              }
            else
              {
                OsSysLog::add(FAC_SIP, PRI_WARNING,
                              "MwiPlugin::handleSubscribeRequest() - voicemailCGI response has no body"
                              );
              }
          }
        else
          {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "MwiPlugin::handleSubscribeRequest() - voicemailCGI response wrong type"
                          );
          }
      }
    else
      {
        OsSysLog::add(FAC_SIP, PRI_WARNING,
                      "MwiPlugin::handleSubscribeRequest() - voicemailCGI GET failed with %d."
                      ,httpStatus
                      );
      }

    return result;
}

OsStatus
MwiPlugin::handleEvent (
    const HttpRequestContext& requestContext,
    const HttpMessage& request,
    HttpMessage& response)
{
    OsStatus result = OS_SUCCESS;

    // mailbox identity and action CGI variables
    UtlString identityUrlStr;
    UtlString action;
    requestContext.getCgiVariable( "action",   action );
    requestContext.getCgiVariable( "identity", identityUrlStr );

    if( !identityUrlStr.isNull())
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "MwiPlugin::handleEvent notice for '%s'", identityUrlStr.data());

        // Extract just the identity part from the IDENTITY
        // a user may specify all sorts of displayname and field
        // modifier in the request, we are interested in the user@address part
        Url mailboxUrl ( identityUrlStr );
        UtlString mailboxIdentity;
        mailboxUrl.getIdentity( mailboxIdentity );

        // get the body from the request containing the MWI summaries
        ssize_t charsRead;
        UtlString buffer;
        HttpBody *pBody = (HttpBody *)request.getBody();
        if ( pBody )
        {
           UtlString bodyType = pBody->getContentType();
           if ( bodyType.compareTo( CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY, UtlString::ignoreCase )
               == 0
               )
           {
              pBody->getBytes(&buffer, &charsRead);
              ssize_t start = buffer.index("messages-waiting:", 0, UtlString::ignoreCase);

              if ( start != UTL_NOT_FOUND )
              {
                 // Remove anything before the start
                 buffer.remove(0, start);
                 int contentLength = buffer.length();
                

                 // create a simple message summary body for
                 // each subscription and send it via the user
                 // agent to the device
                 HttpBody* body = new HttpBody (buffer,
                                                contentLength,
                                                CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY );

                 SipMessage notifyRequest;
                 notifyRequest.setBody( body );

                 // Add the content type for the body
                 notifyRequest.setContentType( CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY );
                 notifyRequest.setContentLength( contentLength );

                 // Send a MWI state change to all subscribed to this
                 // mailbox
                 mNotifier->sendNotifyForeachSubscription(mailboxIdentity.data(),
                                                          SIP_EVENT_MESSAGE_SUMMARY,
                                                          notifyRequest);
              }
              else
              {
                 OsSysLog::add(FAC_SIP, PRI_ERR,
                               "MwiPlugin::handleEvent request body is not a valid notice\n"
                               "======\n%s\n======",
                               buffer.data()
                               );
              }
           }
           else
           {
              OsSysLog::add(FAC_SIP, PRI_ERR,
                            "MwiPlugin::handleEvent request body is not type '"
                            CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY "'\n"
                            "Content-Type: '%s'",
                            bodyType.data()
                            );
           }
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_ERR,
                         "MwiPlugin::handleEvent request has no body");
        }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "MwiPlugin::handleEvent no identity found in http request");
    }
    return result;
}

/******************************PRIVATE**************************************/


SubscribeServerPluginBase*
MwiPluginFactory(const TiXmlNode& pluginConfigDataNode,
                Notifier* notifier )
{
    SubscribeServerPluginBase* plugin = NULL;
    // get attibutes for the Plugin and create new instance of a plugin
    // according to the event type.
    TiXmlElement* pluginElement = (TiXmlElement*)pluginConfigDataNode.ToElement();
    if ( pluginElement != NULL )
    {
        TiXmlNode* attibuteNode = NULL;
        // get event type associated with this plug in
        // create a new plugin for this event type
        attibuteNode = pluginElement->FirstChild(XML_TAG_EVENT_TYPE);
        if(attibuteNode)
        {
            TiXmlNode* eventTypeValue = attibuteNode->FirstChild();
            if(eventTypeValue && eventTypeValue->Type() == TiXmlNode::TEXT)
            {
                TiXmlText* eventTypeText = eventTypeValue->ToText();
                if (eventTypeText)
                {
                    UtlString eventType = eventTypeText->Value();
                    // The following 2 event types map to the MwiPlugin
                    if( eventType.compareTo( SIP_EVENT_MESSAGE_SUMMARY,
                                             UtlString::ignoreCase ) ==0 ||
                        eventType.compareTo( SIP_EVENT_SIMPLE_MESSAGE_SUMMARY,
                                             UtlString::ignoreCase ) == 0 )
                    {
                        // create new plug for MWI ( use the lastest event that
                        // the pHOne supports to create the plugin )
                        plugin = new MwiPlugin(SIP_EVENT_MESSAGE_SUMMARY, *pluginElement, (Notifier*)notifier );
                        // after the plug in is created set the other configuration
                        // data inside the Plugin
                    }
                }
            }
        }
    }
    return plugin;
}
