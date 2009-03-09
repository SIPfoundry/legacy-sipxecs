/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.voicemail;


// Copy from sipXconfig-neoconf-vm  Perhaps we can share one day?

/**
 * Final output format
 * <pre>
 * &lt;prefs&gt;
 *   &lt;activegreeting&gt;outofoffice&lt;/activegreeting&gt;
 *   &lt;notification&gt;
 *       &lt;contact type="email" attachments="no"&gt;dhubler@pingtel.com&lt;/contact&gt;
 *   &lt;/notification&gt;
 * &lt;/prefs&gt;
 * </pre>
 */
public class MailboxPreferences {
    public static final String EMAIL_PROP = "emailAddress";
    private ActiveGreeting m_activeGreeting = ActiveGreeting.NONE;
    private String m_emailAddress;
    private boolean m_attachVoicemailToEmail;
    private String m_alternateEmailAddress;
    private boolean m_attachVoicemailToAlternateEmail;
         
    public enum ActiveGreeting {
        NONE("none"), 
        STANDARD("standard"), 
        OUT_OF_OFFICE("outofoffice"), 
        EXTENDED_ABSENCE("extendedabsence");
        
        private String m_id;
        
        ActiveGreeting(String id) {
            m_id = id;
        }
        
        public String getId() {
            return m_id;
        }
        
        public static ActiveGreeting valueOfById(String id) {
            for (ActiveGreeting greeting : ActiveGreeting.values()) {
                if (greeting.getId().equals(id)) {
                    return greeting;
                }
            }
            throw new IllegalArgumentException("id not recognized " + id);
        }
    }
    
    public ActiveGreeting getActiveGreeting() {
        return m_activeGreeting;
    }
    
    public void setActiveGreeting(ActiveGreeting activeGreeting) {
        m_activeGreeting = activeGreeting;
    }
    
    public boolean isAttachVoicemailToEmail() {
        return m_attachVoicemailToEmail;
    }
    
    public void setAttachVoicemailToEmail(boolean attachVoicemailToEmail) {
        m_attachVoicemailToEmail = attachVoicemailToEmail;
    }
    
    public String getEmailAddress() {
        return m_emailAddress;
    }
    
    public void setEmailAddress(String emailAddress) {
        m_emailAddress = emailAddress;
    }

    public String getAlternateEmailAddress() {
        return m_alternateEmailAddress;
    }

    public void setAlternateEmailAddress(String alternateEmailAddress) {
        m_alternateEmailAddress = alternateEmailAddress;
    }

    public boolean isAttachVoicemailToAlternateEmail() {
        return m_attachVoicemailToAlternateEmail;
    }

    public void setAttachVoicemailToAlternateEmail(boolean attachVoicemailToAlternateEmail) {
        m_attachVoicemailToAlternateEmail = attachVoicemailToAlternateEmail;
    }
}
