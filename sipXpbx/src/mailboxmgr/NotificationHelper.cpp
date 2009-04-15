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
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "utl/XmlContent.h"
#include "net/Url.h"
#include "net/MailMessage.h"
#include "mailboxmgr/NotificationHelper.h"
#include "mailboxmgr/PlayMessagesCGI.h"

// DEFINES
// STATIC INITIALIZERS
NotificationHelper* NotificationHelper::spInstance = NULL;
OsMutex             NotificationHelper::sLockMutex (OsMutex::Q_FIFO);

NotificationHelper*
NotificationHelper::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {   // Create the singleton class for clients to use
        spInstance = new NotificationHelper();
    }
    return spInstance;
}

NotificationHelper::NotificationHelper()
{}

NotificationHelper::~NotificationHelper()
{}

OsStatus
NotificationHelper::send (
    const UtlString& rMailboxIdentity,
    const UtlString& rSMTPServer,
    const Url&      mailboxServiceUrl,
    const UtlString& rContact,
    const UtlString& rFrom,
    const UtlString& rReplyTo,
    const UtlString& rDate,
    const UtlString& rDurationMSecs,
    const UtlString& wavFileName,
    const char*     pAudioData,
    const int&      rAudioDatasize,
    const UtlBoolean& rAttachmentEnabled) const
{
    OsStatus status = OS_SUCCESS;

    // For forwarded messages, duration = aggregate of duration of different
    // messages that make up the forwarded message.
    // Skip duration for forwarded messages in this release (1.1).
    UtlString durationText = "" ;
    if( !wavFileName.contains("-FW") )
    {
        durationText += "Duration " ;
        int iDuration = atoi(rDurationMSecs);
        if( iDuration > 0 )
        {
                        // Convert to seconds
                        iDuration = iDuration / 1000;

                        char temp[10];
                        if( iDuration >= 3600 )
                        {
                                // Retrieve the hour.
                                int hours = iDuration / 3600  ;
                                iDuration = iDuration - (hours * 3600);
                                sprintf( temp, "%02d", hours );
                                durationText = UtlString( temp ) + ":" ;
                        }

                        if( iDuration >= 60 )
                        {
                                // Retrieve the hour.
                                int mins = iDuration / 60  ;
                                iDuration = iDuration - (mins * 60);
                                sprintf( temp, "%02d", mins );

                                durationText += UtlString( temp ) + ":" ;
                        }
                        else
                        {
                                durationText += "00:" ;
                        }

                        // append the seconds
                        sprintf( temp, "%02d", iDuration );
                        durationText += temp;
        }
        else
        {
            durationText = UtlString("00:00") ;
        }
    }

    UtlString strFrom = "Unknown" ;
    if( !rFrom.isNull() && rFrom.length() > 0)
        strFrom = rFrom ;
    UtlString subject = "New Voicemail from " + strFrom ;

    UtlString rawMessageId = wavFileName(0, wavFileName.first('-'));
    UtlString userId = rMailboxIdentity(0, rMailboxIdentity.first('@'));

    UtlString plainBodyText, htmlBodyText;

    MailMessage message ( "Voicemail Notification Service", rReplyTo, rSMTPServer );

    UtlString baseMailboxLink = mailboxServiceUrl.toString();
    baseMailboxLink.append("/").append(userId).append("/inbox");

    UtlString playMessageLink = baseMailboxLink;
    playMessageLink.append("/").append(rawMessageId);
    UtlString deleteMessageLink = baseMailboxLink;
    deleteMessageLink.append("/").append(rawMessageId).append("/delete");
    UtlString showMailboxLink = baseMailboxLink;

    plainBodyText += "On " + rDate + ", " + strFrom + " left new voicemail. " +
        durationText + "\n";
    plainBodyText += "Listen to message " + playMessageLink + "\n";
    plainBodyText += "Show Voicemail Inbox "   + showMailboxLink + "\n";
    plainBodyText += "Delete message " + deleteMessageLink + "\n";

    UtlString playMessageLinkXml ;
    UtlString deleteMessageLinkXml;
    UtlString showMailboxLinkXml;
    XmlEscape(playMessageLinkXml, playMessageLink) ;
    XmlEscape(deleteMessageLinkXml, deleteMessageLink) ;
    XmlEscape(showMailboxLinkXml, showMailboxLink) ;
    // Format the html text if supported by the browser
    htmlBodyText =
        (UtlString)"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n" +
                  "   \"http://www.w3.org/TR/html4/strict.dtd\">\n" +
                  "<HTML>\n" +
                  "<HEAD>\n" +
                  "<TITLE>Voicemail Notification</TITLE>\n" +
                  "</HEAD>\n<BODY>\n";
    htmlBodyText +=
        "<p>On " + rDate + ", " + strFrom + " left new voicemail. " +
        durationText + "</p>\n";
    htmlBodyText +=
        "<p><a href=\"" + playMessageLinkXml + "\">Listen to message</a></p>\n";
    htmlBodyText +=
        "<p><a href=\"" + showMailboxLinkXml + "\">Show Voicemail Inbox</a></p>\n";
    htmlBodyText +=
        "<p><a href=\"" + deleteMessageLinkXml + "\">Delete message</a></p>\n";
    htmlBodyText +=
        (UtlString)"</BODY>\n" +
                                  "</HTML>\n";

    if ( rAttachmentEnabled == TRUE )
    {
        unsigned char* unsignedAudioData = (unsigned char*) pAudioData;
        message.Attach( unsignedAudioData, rAudioDatasize, wavFileName);
    }

    message.Body( plainBodyText , htmlBodyText );
    message.Subject( subject );
    message.To( rContact, rContact );
    UtlString response = message.Send();
    if ( !response.isNull() )
    {
        if( response.length() > 0 )
        {
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR, 
                          "NotificationHelper: "
                          "Error sending e-mail to '%s' via SMTP server '%s'\n    %s",
                          rContact.data(), rSMTPServer.data(), response.data());
            OsSysLog::flush();
        }
    }
    return status;
}
