/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;

public class MailboxPreferences {
    public static final String ACTIVE_GREETING = "voicemail/mailbox/active-greeting";
    public static final String ATTACH_VOICEMAIL = "voicemail/mailbox/attach-voicemail";
    public static final String ATTACH_VOICEMAIL_ALTERNATE = "voicemail/mailbox/attach-voicemail-to-additional-email";
    public static final String IMAP_PASSWORD = "voicemail/imap/password";
    public static final String IMAP_TLS = "voicemail/imap/tls";
    public static final String IMAP_PORT = "voicemail/imap/port";
    public static final String IMAP_HOST = "voicemail/imap/host";

    public enum ActiveGreeting {
        NONE("none"), STANDARD("standard"), OUT_OF_OFFICE("outofoffice"), EXTENDED_ABSENCE("extendedabsence");

        private final String m_id;

        ActiveGreeting(String id) {
            m_id = id;
        }

        public String getId() {
            return m_id;
        }

        public static ActiveGreeting fromId(String id) {
            for (ActiveGreeting greeting : ActiveGreeting.values()) {
                if (greeting.getId().equals(id)) {
                    return greeting;
                }
            }
            throw new IllegalArgumentException("id not recognized " + id);
        }
    }

    public enum AttachType {
        NO("0"), YES("1"), IMAP("2");

        private String m_value;

        AttachType(String value) {
            m_value = value;
        }

        public String getValue() {
            return m_value;
        }

        public static AttachType fromValue(String value) {
            for (AttachType e : values()) {
                if (e.m_value.equals(value)) {
                    return e;
                }
            }
            return NO;
        }
    }

    private ActiveGreeting m_activeGreeting = ActiveGreeting.NONE;

    private String m_emailAddress;
    private AttachType m_attachVoicemailToEmail = AttachType.NO;

    private String m_alternateEmailAddress;
    private boolean m_attachVoicemailToAlternateEmail;

    private boolean m_synchronizeWithImapServer;
    private String m_imapHost;
    private String m_imapPort;
    private boolean m_imapTLS;
    private String m_imapPassword;

    public MailboxPreferences() {
        // empty
    }

    public MailboxPreferences(User user) {
        m_emailAddress = user.getEmailAddress();
        m_alternateEmailAddress = user.getAlternateEmailAddress();
        m_activeGreeting = ActiveGreeting.fromId((user.getSettingValue(ACTIVE_GREETING)));
        m_attachVoicemailToEmail = AttachType.fromValue(user.getSettingValue(ATTACH_VOICEMAIL));
        m_attachVoicemailToAlternateEmail = (Boolean) user.getSettingTypedValue(ATTACH_VOICEMAIL_ALTERNATE);
        m_imapHost = user.getSettingValue(IMAP_HOST);
        m_imapPort = user.getSettingValue(IMAP_PORT);
        m_imapTLS = (Boolean) user.getSettingTypedValue(IMAP_TLS);
        m_imapPassword = user.getSettingValue(IMAP_PASSWORD);
    }

    public void updateUser(User user) {
        user.setEmailAddress(m_emailAddress);
        user.setAlternateEmailAddress(m_alternateEmailAddress);
        user.setSettingValue(ACTIVE_GREETING, m_activeGreeting.getId());
        user.setSettingValue(ATTACH_VOICEMAIL, m_attachVoicemailToEmail.getValue());
        user.setSettingValue(IMAP_HOST, m_imapHost);
        user.setSettingValue(IMAP_PORT, m_imapPort);
        user.setSettingTypedValue(IMAP_TLS, m_imapTLS);
        user.setSettingValue(IMAP_PASSWORD, m_imapPassword);
    }

    public ActiveGreeting getActiveGreeting() {
        return m_activeGreeting;
    }

    public void setActiveGreeting(ActiveGreeting activeGreeting) {
        m_activeGreeting = activeGreeting;
    }

    public AttachType getAttachVoicemailToEmail() {
        return m_attachVoicemailToEmail;
    }

    public void setAttachVoicemailToEmail(AttachType attachVoicemailToEmail) {
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
        return m_synchronizeWithImapServer;
    }

    public void setSynchronizeWithImapServer(boolean synchronizeWithImapServer) {
        m_synchronizeWithImapServer = synchronizeWithImapServer;
    }

    public String getImapHost() {
        return m_imapHost;
    }

    public void setImapHost(String imapHost) {
        m_imapHost = imapHost;
    }

    public String getImapPort() {
        return m_imapPort;
    }

    public void setImapPort(String imapPort) {
        m_imapPort = imapPort;
    }

    public boolean getEmailServerUseTLS() {
        return m_imapTLS;
    }

    public void setImapTLS(boolean imapTls) {
        m_imapTLS = imapTls;
    }

    public String getImapPassword() {
        return m_imapPassword;
    }

    public void setImapPassword(String emailPassword) {
        m_imapPassword = emailPassword;
    }

    public boolean hasImapServer() {
        return StringUtils.isNotEmpty(getImapHost()) && getImapPort() != null;
    }

    public AttachType[] getAttachOptions() {
        if (hasImapServer()) {
            return AttachType.values();
        }
        return new AttachType[] {
            AttachType.YES, AttachType.NO
        };
    }
}
