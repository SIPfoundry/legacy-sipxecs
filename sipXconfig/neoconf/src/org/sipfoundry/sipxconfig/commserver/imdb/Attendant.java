/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ACCOUNT;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.CALL_FROM_ANY_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CALL_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_ENTRY_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_EXIT_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.HASHED_PASSTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.HOST;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ADVERTISE_ON_CALL_STATUS;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ON_THE_PHONE_MESSAGE;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_SHOW_ON_CALL_DETAILS;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_BEGIN_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_END_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.PASSWD;
import static org.sipfoundry.commons.mongo.MongoConstants.PINTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.PORT;
import static org.sipfoundry.commons.mongo.MongoConstants.SYNC;
import static org.sipfoundry.commons.mongo.MongoConstants.TLS;
import static org.sipfoundry.commons.mongo.MongoConstants.USERBUSYPROMPT;
import static org.sipfoundry.commons.mongo.MongoConstants.VMONDND;
import static org.sipfoundry.commons.mongo.MongoConstants.VOICEMAILTUI;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

import com.mongodb.DBObject;

public class Attendant extends AbstractDataSetGenerator {

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (!(entity instanceof User)) {
            return false;
        }
        User user = (User) entity;
        // The following settings used to be in validusers.xml
        top.put(USERBUSYPROMPT, user.getSettingValue("voicemail/mailbox/user-busy-prompt")); // can
                                                                                             // be
                                                                                             // null
        top.put(VOICEMAILTUI, user.getSettingValue("voicemail/mailbox/voicemail-tui")); // can be
                                                                                        // null
        top.put(DISPLAY_NAME, user.getDisplayName());
        top.put(PINTOKEN, user.getPintoken());
        top.put(HASHED_PASSTOKEN, user.getSipPasswordHash(getCoreContext().getAuthorizationRealm()));

        // not sure why this here too as it is already added from Mailstore.java
        MailboxPreferences mp = new MailboxPreferences(user);

        String emailAddress = mp.getEmailAddress();
        boolean enableNotification = StringUtils.isNotBlank(emailAddress) && mp.isEmailNotificationEnabled();
        if (StringUtils.isNotBlank(emailAddress)) {
            top.put(EMAIL, emailAddress);
        } else {
            removeField(top, EMAIL);
        }

        if (enableNotification) {
            top.put(NOTIFICATION, mp.getEmailFormat().name());
            top.put(ATTACH_AUDIO, Boolean.toString(mp.isIncludeAudioAttachment()));
        } else {
            removeField(top, NOTIFICATION);
            removeField(top, ATTACH_AUDIO);
        }

        String alternateEmailAddress = mp.getAlternateEmailAddress();
        boolean enableAltNotification = StringUtils.isNotBlank(alternateEmailAddress)
                && mp.isEmailNotificationAlternateEnabled();
        if (StringUtils.isNotBlank(alternateEmailAddress)) {
            top.put(ALT_EMAIL, alternateEmailAddress);
        }

        if (enableAltNotification) {
            top.put(ALT_NOTIFICATION, mp.getAlternateEmailFormat().name());
            top.put(ALT_ATTACH_AUDIO, Boolean.toString(mp.isIncludeAudioAttachmentAlternateEmail()));
        } else {
            removeField(top, ALT_NOTIFICATION);
            removeField(top, ALT_ATTACH_AUDIO);
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
        } else {
            removeField(top, SYNC);
            removeField(top, HOST);
            removeField(top, PORT);
            removeField(top, TLS);
            removeField(top, ACCOUNT);
            removeField(top, PASSWD);
        }

        ImAccount imAccount = new ImAccount(user);
        top.put(IM_ENABLED, imAccount.isEnabled());
        // The following settings used to be in contact-information.xml
        top.put(IM_ID, imAccount.getImId());
        top.put(IM_DISPLAY_NAME, imAccount.getImDisplayName());
        top.put(CONF_ENTRY_IM, user.getSettingValue("im_notification/conferenceEntryIM").toString());
        top.put(CONF_EXIT_IM, user.getSettingValue("im_notification/conferenceExitIM").toString());
        top.put(LEAVE_MESSAGE_BEGIN_IM, user.getSettingValue("im_notification/leaveMsgBeginIM").toString());
        top.put(LEAVE_MESSAGE_END_IM, user.getSettingValue("im_notification/leaveMsgEndIM").toString());
        top.put(CALL_IM, user.getSettingValue("im_notification/call").toString());
        top.put(CALL_FROM_ANY_IM, user.getSettingValue("im_notification/callFromAnyNumber").toString());
        // and this one in presencerouting-prefs.xml
        top.put(VMONDND, imAccount.isForwardOnDnd());
        // settings from xmpp-account-info.xml
        top.put(IM_ON_THE_PHONE_MESSAGE, imAccount.getOnThePhoneMessage());
        top.put(IM_ADVERTISE_ON_CALL_STATUS, imAccount.advertiseSipPresence());
        top.put(IM_SHOW_ON_CALL_DETAILS, imAccount.includeCallInfo());
        return true;
    }

    @Override
    protected DataSet getType() {
        return DataSet.ATTENDANT;
    }
}
