/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */

package org.sipfoundry.voicemail;

import java.text.MessageFormat;
import java.util.Date;

import org.sipfoundry.sipxivr.Configuration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.ValidUsersXML;

public class EmailFormatter {
    Configuration m_ivrConfig;
    VmMessage m_vmMessage;
    Mailbox m_mailbox;
    Object[] m_args;
    
    /**
     * A formatter for e-mail messages.
     * 
     * Provides the text parts of an e-mail message using MessageFormat templates.
     * TODO Re-work when templates come from configuration somewhere.
     * For now they are fixed.
     * 
     * @param ivrConfig
     * @param mailbox
     * @param vmessage
     */
    public EmailFormatter(Configuration ivrConfig, Mailbox mailbox, VmMessage vmessage) {
        m_ivrConfig = ivrConfig;
        m_mailbox = mailbox;
        m_vmMessage =vmessage;
        
        String fromUri = m_vmMessage.getMessageDescriptor().getFromUri();
        String fromUser = ValidUsersXML.getUserPart(fromUri);
        Object[] args = {
                new Long(m_vmMessage.getDuration()*1000),    //  0 audio Duration in mS
                fromUri,                                     //  1 From URI
                fromUser,                                    //  2 From User Part (phone number, most likely)
                String.format("%s/sipxconfig/mailbox/%s/inbox/", m_ivrConfig.getConfigUrl(), 
                   m_mailbox.getUser().getUserName()),       //  3 Portal Link URL
                m_vmMessage.getMessageId(),                  //  4 Message Id
                new Date(m_vmMessage.getTimestamp()),        //  5 message timestamp
                "Voicemail Notification Service",            //  6 Sender Name
                "postmaster@localhost",                      //  7 Sender mailto
                "Voicemail Notification",                    //  8 html title 
            };
        m_args = args;
    }

    /**
     * The sender.  Ends up in the From header of the e-mail
     * @return
     */
    public String getSender() {
        return MessageFormat.format("{6} <{7}>", m_args);
    }
    
    /**
     * The subject of the e-mail
     * @return
     */
    public String getSubject() {
        return MessageFormat.format("{0,time,m:ss} Voice Message from {2}", m_args);
    }
    
    /**
     * The HTML body part of the e-mail
     * @return
     */
    public String getHtmlBody() {
        
        String t;
        t  = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n";
        t += "   \"http://www.w3.org/TR/html4/strict.dtd\">\n";
        t += "<html>\n";
        t += "   <head>\n";
        t += "      <title>{8}</title>\n";
        t += "   </head>\n";
        t += "   <body>\n";
        t += "      <p>"+ getSubject()+"</p>\n";
        t += "      <p></p>\n";
        t += "      <p><a href=\"{3}{4}\">Listen to message</a></p>\n";
        t += "      <p><a href=\"{3}\">Show voicemail inbox</a></p>\n";
        t += "      <p><a href=\"{3}{4}/delete" + "\">Delete message</a></p>\n";
        t += "   </body>\n";
        t += "</html>\n";
        
        return MessageFormat.format(t, m_args);
    }

    /**
     * The text body part of the e-mail
     * @return
     */
    public String getTextBody() {
        String t;
        t  = "{8}\n";
        t += "\n";
        t += getSubject()+"\n";
        t += "\n";
        t += "Listen to message, click here: {3}{4}\n";
        t += "Show inbox, click here:        {3}\n";
        t += "Delete message, click here:    {3}{4}/delete\n";
        
        return MessageFormat.format(t, m_args);
    }
}
