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
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.commons.userdb.User.EmailFormats;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;

public class EmailFormatter {
    public static String EMAIL_FORMATS_BUNDLE = "org.sipfoundry.voicemail.EmailFormats";
    
    IvrConfiguration m_ivrConfig;
    VmMessage m_vmMessage;
    Mailbox m_mailbox;
    ResourceBundle m_bundle;
    Object[] m_args;
    
    /**
     * Factory method to return the right EmailFormatter based on emailFormat
     * @param emailFormat
     * @param ivrConfig
     * @param mailbox
     * @param vmessage
     * @return
     */
    public static EmailFormatter getEmailFormatter(EmailFormats emailFormat,
            IvrConfiguration ivrConfig, Mailbox mailbox, VmMessage vmessage) {
        EmailFormatter emf = null;
        
        switch (emailFormat) {
        case FORMAT_FULL:
            emf = new EmailFormatter();
            break ;
        case FORMAT_MEDIUM:
            emf = new EmailFormatterMedium();
            break;
        case FORMAT_BRIEF:
            emf = new EmailFormatterBrief();
            break;
        case FORMAT_IMAP:
            emf = new EmailFormatterImap();
            break;
        default:
            return null;
        }

        emf.init(ivrConfig, mailbox, vmessage);        
        return emf;
    }
    
    /**
     * A formatter for e-mail messages.
     * 
     * Provides the text parts of an e-mail message using MessageFormat templates.
     * 
     * @param ivrConfig
     * @param mailbox
     * @param vmessage
     */
    
    private void init(IvrConfiguration ivrConfig, Mailbox mailbox, VmMessage vmessage) {
        
        String fromDisplay = null;
        Object[] args = new Object[15];
        String fromUri = "";
        String fromUser = "";
        
        m_ivrConfig = ivrConfig;
        m_mailbox = mailbox;
        
        Locale locale = m_mailbox.getUser().getLocale();
        if (locale == null) {
            locale = Locale.getDefault();
        }
        
        try {
            // Look for "EmailFormats" most likely in an /etc/ path somewhere
            m_bundle = ResourceBundle.getBundle("EmailFormats", locale);
        } catch (MissingResourceException e) {
            // Use the built in one as a last resort
            m_bundle = ResourceBundle.getBundle(EMAIL_FORMATS_BUNDLE, locale);
        }

        m_vmMessage = vmessage;

        if(m_vmMessage != null) {
            fromUri = m_vmMessage.getMessageDescriptor().getFromUri();
            fromUser = ValidUsersXML.getUserPart(fromUri);
            fromDisplay = ValidUsersXML.getDisplayPart(fromUri);
            args[ 0] = new Long(m_vmMessage.getDuration()*1000);    //  0 audio Duration in mS
            args[ 5] = m_vmMessage.getMessageId();                  //  5 Message Id
            args[ 6] = new Date(m_vmMessage.getTimestamp());        //  6 message timestamp         
        }
        
        if (fromDisplay==null) {
            fromDisplay = "";
        }
        
        // Build original set of args
       
        args[ 1] = fromUri;                                     //  1 From URI
        args[ 2] = fromUser;                                    //  2 From User Part (phone number, most likely)
        args[ 3] = fromDisplay;                                 //  3 From Display Name
        args[ 4] = String.format("%s/sipxconfig/mailbox/%s/inbox/", 
                m_ivrConfig.getConfigUrl(),m_mailbox.getUser().getUserName());       
                                                                //  4 Portal Link URL             
        // Using the existing args, add some more, recursively as they are defined with some of the above variables.
        args[ 7] = fmt("SenderName", args);                     //  7 Sender Name
        args[ 8] = fmt("SenderMailto", args);                   //  8 Sender mailto
        args[ 9] = fmt("HtmlTitle", args);                      //  9 html title

        args[10] = fmt("PortalURL", args);                      // 10 Portal URL (if needs to be re-written)
        args[11] = fmt("Sender", args);                         // 11 Sender (as url'ish)
        args[12] = fmt("SubjectFull", args);                    // 12 Subject (for Full)
        args[13] = fmt("SubjectMedium", args);                  // 13 Subject (for Medium)
        args[14] = fmt("SubjectBrief", args);                   // 14 Subject (for Brief)
        m_args = args;
    }


    public String fmt(String text) {
        return fmt(text, m_args);
    }
    
    public String fmt(String text, Object[] args) {
        return MessageFormat.format(m_bundle.getString(text), args);
    }

    
    public String getSender() {
        return fmt("Sender");
    }
    
    /**
     * The Subject part of the e-mail
     * @return
     */
    public String getSubject() {
        return fmt("SubjectFull");
    }
    
    /**
     * The HTML body part of the e-mail.  
     * @return null or "" if there is none
     */
    public String getHtmlBody() {
        return fmt("HtmlBodyFull");
    }

    /**
     * The text body part of the e-mail
     * @return
     */
    public String getTextBody() {
        return fmt("TextBodyFull");
    }
}
