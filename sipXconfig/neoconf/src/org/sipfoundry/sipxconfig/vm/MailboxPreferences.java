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

    public static final String PRIMARY_EMAIL_NOTIFICATION = "voicemail/mailbox/primary-email-voicemail-notification";
    public static final String PRIMARY_EMAIL_FORMAT = "voicemail/mailbox/primary-email-format";
    public static final String PRIMARY_EMAIL_ATTACH_AUDIO = "voicemail/mailbox/primary-email-attach-audio";

    public static final String ALT_EMAIL_NOTIFICATION = "voicemail/mailbox/alternate-email-voicemail-notification";
    public static final String ALT_EMAIL_FORMAT = "voicemail/mailbox/alternate-email-format";
    public static final String ALT_EMAIL_ATTACH_AUDIO = "voicemail/mailbox/alternate-email-attach-audio";

    public static final String IMAP_ACCOUNT = "voicemail/imap/account";
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

    public enum MailFormat {
        FULL, MEDIUM, BRIEF;
    }

    private ActiveGreeting m_activeGreeting = ActiveGreeting.NONE;

    private String m_emailAddress;
    private MailFormat m_emailFormat = MailFormat.FULL;
    private AttachType m_attachVoicemailToEmail = AttachType.NO;
    private boolean m_includeAudioAttachment;

    private String m_alternateEmailAddress;
    private MailFormat m_alternateEmailFormat = MailFormat.FULL;
    private AttachType m_voicemailToAlternateEmailNotification = AttachType.NO;
    private boolean m_includeAudioAttachmentAlternateEmail;

    private String m_imapHost;
    private String m_imapPort;
    private boolean m_imapTLS;
    private String m_imapAccount;
    private String m_imapPassword;

    public MailboxPreferences() {
        // empty
    }

    public MailboxPreferences(User user) {
        m_emailAddress = user.getEmailAddress();
        m_alternateEmailAddress = user.getAlternateEmailAddress();
        m_activeGreeting = ActiveGreeting.fromId((user.getSettingValue(ACTIVE_GREETING)));
        m_attachVoicemailToEmail = AttachType.fromValue(user.getSettingValue(PRIMARY_EMAIL_NOTIFICATION));
        m_emailFormat = MailFormat.valueOf(user.getSettingValue(PRIMARY_EMAIL_FORMAT));
        m_alternateEmailFormat = MailFormat.valueOf(user.getSettingValue(ALT_EMAIL_FORMAT));
        m_voicemailToAlternateEmailNotification = AttachType.fromValue(user.getSettingValue(ALT_EMAIL_NOTIFICATION));
        m_imapHost = user.getSettingValue(IMAP_HOST);
        m_imapPort = user.getSettingValue(IMAP_PORT);
        m_imapTLS = (Boolean) user.getSettingTypedValue(IMAP_TLS);
        m_imapPassword = user.getSettingValue(IMAP_PASSWORD);
        m_imapAccount = user.getSettingValue(IMAP_ACCOUNT);
        m_includeAudioAttachment = (Boolean) user.getSettingTypedValue(PRIMARY_EMAIL_ATTACH_AUDIO);
        m_includeAudioAttachmentAlternateEmail = (Boolean) user.getSettingTypedValue(ALT_EMAIL_ATTACH_AUDIO);
    }

    public void updateUser(User user) {
        user.setEmailAddress(m_emailAddress);
        user.setAlternateEmailAddress(m_alternateEmailAddress);
        user.setSettingValue(ACTIVE_GREETING, m_activeGreeting.getId());
        user.setSettingValue(PRIMARY_EMAIL_NOTIFICATION, m_attachVoicemailToEmail.getValue());
        user.setSettingValue(PRIMARY_EMAIL_FORMAT, m_emailFormat.name());
        user.setSettingValue(ALT_EMAIL_FORMAT, m_alternateEmailFormat.name());
        user.setSettingValue(ALT_EMAIL_NOTIFICATION, m_voicemailToAlternateEmailNotification.getValue());
        user.setSettingValue(IMAP_HOST, m_imapHost);
        user.setSettingValue(IMAP_PORT, m_imapPort);
        user.setSettingTypedValue(IMAP_TLS, m_imapTLS);
        user.setSettingValue(IMAP_PASSWORD, m_imapPassword);
        user.setSettingValue(IMAP_ACCOUNT, m_imapAccount);
        user.setSettingTypedValue(PRIMARY_EMAIL_ATTACH_AUDIO, m_includeAudioAttachment);
        user.setSettingTypedValue(ALT_EMAIL_ATTACH_AUDIO, m_includeAudioAttachmentAlternateEmail);
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

    public MailFormat getEmailFormat() {
        return m_emailFormat;
    }

    public void setEmailFormat(MailFormat emailFormat) {
        m_emailFormat = emailFormat;
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

    public AttachType getVoicemailToAlternateEmailNotification() {
        return m_voicemailToAlternateEmailNotification;
    }

    public void setVoicemailToAlternateEmailNotification(AttachType voicemailToAlternateEmailNotification) {
        m_voicemailToAlternateEmailNotification = voicemailToAlternateEmailNotification;
    }

    public boolean isSynchronizeWithImapServer() {
        return m_attachVoicemailToEmail == AttachType.IMAP;
    }

    public boolean isEmailNotificationEnabled() {
        return m_attachVoicemailToEmail == AttachType.YES;
    }

    public boolean isEmailNotificationAlternateEnabled() {
        return m_voicemailToAlternateEmailNotification == AttachType.YES;
    }

    public boolean isIncludeAudioAttachment() {
        return m_includeAudioAttachment;
    }

    public void setIncludeAudioAttachment(boolean includeAudioAttachment) {
        m_includeAudioAttachment = includeAudioAttachment;
    }

    public MailFormat getAlternateEmailFormat() {
        return m_alternateEmailFormat;
    }

    public void setAlternateEmailFormat(MailFormat emailFormat) {
        m_alternateEmailFormat = emailFormat;
    }

    public boolean isIncludeAudioAttachmentAlternateEmail() {
        return m_includeAudioAttachmentAlternateEmail;
    }

    public void setIncludeAudioAttachmentAlternateEmail(boolean audioAttachmentAlternateEmail) {
        m_includeAudioAttachmentAlternateEmail = audioAttachmentAlternateEmail;
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

    public boolean getImapTLS() {
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

    public String getImapAccount() {
        return m_imapAccount;
    }

    public void setImapAccount(String imapAccount) {
        m_imapAccount = imapAccount;
    }

    public boolean isImapServerConfigured() {
        return StringUtils.isNotEmpty(getImapHost()) && getImapPort() != null;
    }

    public AttachType[] getAttachOptions(boolean isAdmin) {
        if (isImapServerConfigured() || isAdmin) {
            return AttachType.values();
        }
        return new AttachType[] {
            AttachType.NO, AttachType.YES
        };

    }

    public AttachType[] getAttachOptionsForAlternateEmail() {
        return new AttachType[] {
            AttachType.NO, AttachType.YES
        };
    }
}
