/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class Attendant extends DataSetGenerator {
    private static final String USERBUSYPROMPT = "bsyprmpt";
    private static final String VOICEMAILTUI = "vcmltui";
    private static final String EMAIL = "email";
    private static final String NOTIFICATION = "notif";
    private static final String ATTACH_AUDIO = "attaudio";
    private static final String ALT_EMAIL = "altemail";
    private static final String ALT_NOTIFICATION = "altnotif";
    private static final String ALT_ATTACH_AUDIO = "altattaudio";
    private static final String SYNC = "synch";
    private static final String HOST = "host";
    private static final String PORT = "port";
    private static final String TLS = "tls";
    private static final String ACCOUNT = "acnt";
    private static final String PASSWD = "pswd";

    @Override
    public void generate(Replicable entity) {
        if (!(entity instanceof User)) {
            return;
        }
        User user = (User) entity;
        DBObject top = findOrCreate(user);
        top.put(USERBUSYPROMPT, user.getSettingValue("voicemail/mailbox/user-busy-prompt")); // can
                                                                                             // be
                                                                                             // null
        top.put(VOICEMAILTUI, user.getSettingValue("voicemail/mailbox/voicemail-tui")); // can be
                                                                                        // null
        MailboxPreferences mp = new MailboxPreferences(user);
        String emailAddress = mp.getEmailAddress();
        if (StringUtils.isNotBlank(emailAddress)) {
            top.put(EMAIL, emailAddress);
            if (mp.isEmailNotificationEnabled()) {
                top.put(NOTIFICATION, mp.getEmailFormat().name());
                top.put(ATTACH_AUDIO, Boolean.toString(mp.isIncludeAudioAttachment()));
            }
        }
        String alternateEmailAddress = mp.getAlternateEmailAddress();
        if (StringUtils.isNotBlank(alternateEmailAddress)) {
            top.put(ALT_EMAIL, alternateEmailAddress);
            if (mp.isEmailNotificationAlternateEnabled()) {
                top.put(ALT_NOTIFICATION, mp.getAlternateEmailFormat().name());
                top.put(ALT_ATTACH_AUDIO, Boolean.toString(mp.isIncludeAudioAttachmentAlternateEmail()));
            }
        }
        boolean imapServerConfigured = mp.isImapServerConfigured();
        if (imapServerConfigured) {
            top.put(SYNC, mp.isSynchronizeWithImapServer());
            top.put(HOST, mp.getImapHost());
            top.put(PORT, mp.getImapPort());
            top.put(TLS, mp.getImapTLS());
            top.put(ACCOUNT, StringUtils.defaultString(mp.getImapAccount()));
            String pwd = StringUtils.defaultString(mp.getImapPassword());
            String encodedPwd = new String(Base64.encodeBase64(pwd.getBytes()));
            top.put(PASSWD, encodedPwd);
        }
        getDbCollection().save(top);
    }

    @Override
    protected DataSet getType() {
        return DataSet.ATTENDANT;
    }

}
