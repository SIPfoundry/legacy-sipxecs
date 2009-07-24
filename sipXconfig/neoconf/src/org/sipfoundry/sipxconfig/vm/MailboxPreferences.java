/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Final output format
 *
 * <pre>
 * &lt;prefs&gt;
 *   &lt;activegreeting&gt;outofoffice&lt;/activegreeting&gt;
 *   &lt;notification&gt;
 *       &lt;contact type=&quot;email&quot; attachments=&quot;no&quot;&gt;dhubler@pingtel.com&lt;/contact&gt;
 *   &lt;/notification&gt;
 * &lt;/prefs&gt;
 * </pre>
 */
public class MailboxPreferences {
    public static final String ATTACH_VOICEMAIL = "attachVoicemail";
    public static final String DO_NOT_ATTACH_VOICEMAIL = "doNotAttachVoicemail";
    public static final String SYNCHRONIZE_WITH_EMAIL_SERVER = "synchronizeWithEmailServer";
    public static final String EMAIL_PROP = "emailAddress";
    private User m_user;
    private ActiveGreeting m_activeGreeting = ActiveGreeting.NONE;
    private String m_emailAddress;
    private boolean m_attachVoicemailToEmail;
    private String m_alternateEmailAddress;
    private boolean m_attachVoicemailToAlternateEmail;
    private boolean m_synchronizeWithEmailServer;
    private String m_voicemailProperties;
    private String m_emailServerHost;
    private String m_emailServerPort;
    private boolean m_emailServerUseTLS;
    private String m_emailPassword;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public enum ActiveGreeting {
        NONE("none"), STANDARD("standard"), OUT_OF_OFFICE("outofoffice"), EXTENDED_ABSENCE("extendedabsence");

        private final String m_id;

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

    public boolean isSynchronizeWithEmailServer() {
        return m_synchronizeWithEmailServer;
    }

    public void setSynchronizeWithEmailServer(boolean synchronizeWithEmailServer) {
        m_synchronizeWithEmailServer = synchronizeWithEmailServer;
    }

    public String getVoicemailProperties() {
        return m_voicemailProperties;
    }

    public void setVoicemailProperties(String voicemailProperties) {
        m_voicemailProperties = voicemailProperties;
        if (m_voicemailProperties != null && m_voicemailProperties.equals(SYNCHRONIZE_WITH_EMAIL_SERVER)) {
            m_synchronizeWithEmailServer = true;
            m_attachVoicemailToEmail = true;
        } else if (m_voicemailProperties != null && m_voicemailProperties.equals(ATTACH_VOICEMAIL)) {
            m_synchronizeWithEmailServer = false;
            m_attachVoicemailToEmail = true;
        } else if (m_voicemailProperties != null && m_voicemailProperties.equals(DO_NOT_ATTACH_VOICEMAIL)) {
            m_synchronizeWithEmailServer = false;
            m_attachVoicemailToEmail = false;
        }
    }

    public String getEmailServerHost() {
        return m_emailServerHost;
    }

    public void setEmailServerHost(String emailServerHost) {
        m_emailServerHost = emailServerHost;
    }

    public String getEmailServerPort() {
        return m_emailServerPort;
    }

    public void setEmailServerPort(String emailServerPort) {
        m_emailServerPort = emailServerPort;
    }

    public boolean getEmailServerUseTLS() {
        return m_emailServerUseTLS;
    }

    public void setEmailServerUseTLS(boolean emailServerUseTLS) {
        m_emailServerUseTLS = emailServerUseTLS;
    }

    public String getEmailPassword() {
        return m_emailPassword;
    }

    public void setEmailPassword(String emailPassword) {
        m_emailPassword = emailPassword;
    }

    public boolean ifEmailServer() {
        if (StringUtils.isNotEmpty(getEmailServerHost()) && getEmailServerPort() != null) {
            return true;
        }
        return false;
    }
}
