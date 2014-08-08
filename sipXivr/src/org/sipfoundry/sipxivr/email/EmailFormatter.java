/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */

package org.sipfoundry.sipxivr.email;

import java.util.Date;
import java.util.Locale;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.voicemail.mailbox.VmMessage;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class EmailFormatter implements ApplicationContextAware {

    private String m_emailAddressUrl;
    private Object[] m_args;
    private User m_user;
    private ApplicationContext m_context;
    
    /**
     * A formatter for e-mail messages.
     * 
     * Provides the text parts of an e-mail message using MessageFormat templates.
     * 
     * @param ivrConfig
     * @param mailbox
     * @param vmessage
     */
    
    public void init(User user, VmMessage vmessage) {
        m_user = user;
        String fromDisplay = null;
        Object[] args = new Object[15];
        String fromUri = "";
        String fromUser = "";
        
        if(vmessage != null) {
            fromUri = vmessage.getDescriptor().getFromUri();
            fromUser = ValidUsers.getUserPart(fromUri);
            fromDisplay = ValidUsers.getDisplayPart(fromUri);
            args[ 0] = new Long(vmessage.getDescriptor().getDurationSecsLong()*1000);    //  0 audio Duration in mS
            args[ 5] = vmessage.getMessageId();                  //  5 Message Id
            args[ 6] = new Date(vmessage.getDescriptor().getTimeStampDate().getTime());        //  6 message timestamp         
        }
        
        if (fromDisplay==null) {
            fromDisplay = "";
        }
        
        // Build original set of args
       
        args[ 1] = fromUri;                                     //  1 From URI
        args[ 2] = fromUser;                                    //  2 From User Part (phone number, most likely)
        args[ 3] = fromDisplay;                                 //  3 From Display Name
        args[ 4] = fmt("PortalURL", args);
        if (args[ 4] == null || StringUtils.equals("null", (String) args[ 4])) {
            args[ 4] = m_emailAddressUrl;
        }
                                                                //  4 Portal Link URL             
        // Using the existing args, add some more, recursively as they are defined with some of the above variables.
        args[ 7] = fmt("SenderName", args);                     //  7 Sender Name
        args[ 8] = fmt("SenderMailto", args);                   //  8 Sender mailto
        args[ 9] = fmt("HtmlTitle", args);                      //  9 html title

        args[10] = String.format("%s/sipxconfig/mailbox/%s/inbox/", 
                args[ 4], m_user.getUserName());                      // 10 Portal URL (if needs to be re-written)
        args[11] = fmt("Sender", args);                         // 11 Sender (as url'ish)
        args[12] = fmt("SubjectFull", args);                    // 12 Subject (for Full)
        args[13] = fmt("SubjectMedium", args);                  // 13 Subject (for Medium)
        args[14] = fmt("SubjectBrief", args);                   // 14 Subject (for Brief)
        m_args = args;
    }


    public String fmt(String text) {
        return fmt(text, m_args);
    }

    private String fmt(String text, Object[] args) {
        String value = "";
        if (text == null) {
            return value;
        }
        Locale locale = m_user.getLocale();
        if (locale == null) {
            locale = Locale.getDefault();
        }
        return m_context.getMessage(text, args, "Not Found", locale);
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

    public void setEmailAddressUrl(String emailAddressUrl) {
        m_emailAddressUrl = emailAddressUrl;
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_context = context;
    }
}
