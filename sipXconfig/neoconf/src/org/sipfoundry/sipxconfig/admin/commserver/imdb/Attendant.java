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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ACCOUNT;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.ALT_NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.ASSISTANT_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.ASSISTANT_PHONE;
import static org.sipfoundry.commons.mongo.MongoConstants.ATTACH_AUDIO;
import static org.sipfoundry.commons.mongo.MongoConstants.CELL_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.COMPANY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_ENTRY_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_EXIT_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.FAX_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.HASHED_PASSTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_PHONE_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.HOME_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.HOST;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ADVERTISE_ON_CALL_STATUS;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ON_THE_PHONE_MESSAGE;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_PASSWORD;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_SHOW_ON_CALL_DETAILS;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_DEPT;
import static org.sipfoundry.commons.mongo.MongoConstants.JOB_TITLE;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_BEGIN_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.LEAVE_MESSAGE_END_IM;
import static org.sipfoundry.commons.mongo.MongoConstants.LOCATION;
import static org.sipfoundry.commons.mongo.MongoConstants.NOTIFICATION;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_CITY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_COUNTRY;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_DESIGNATION;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STATE;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_STREET;
import static org.sipfoundry.commons.mongo.MongoConstants.OFFICE_ZIP;
import static org.sipfoundry.commons.mongo.MongoConstants.PASSWD;
import static org.sipfoundry.commons.mongo.MongoConstants.PORT;
import static org.sipfoundry.commons.mongo.MongoConstants.SYNC;
import static org.sipfoundry.commons.mongo.MongoConstants.TLS;
import static org.sipfoundry.commons.mongo.MongoConstants.USERBUSYPROMPT;
import static org.sipfoundry.commons.mongo.MongoConstants.VMONDND;
import static org.sipfoundry.commons.mongo.MongoConstants.VOICEMAILTUI;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

import com.mongodb.DBObject;

public class Attendant extends DataSetGenerator {

    @Override
    public void generate(Replicable entity, DBObject top) {
        if (!(entity instanceof User)) {
            return;
        }
        User user = (User) entity;
        // The following settings used to be in validusers.xml
        top.put(USERBUSYPROMPT, user.getSettingValue("voicemail/mailbox/user-busy-prompt")); // can
                                                                                             // be
                                                                                             // null
        top.put(VOICEMAILTUI, user.getSettingValue("voicemail/mailbox/voicemail-tui")); // can be
                                                                                        // null
        top.put(DISPLAY_NAME, user.getDisplayName());
        top.put(HASHED_PASSTOKEN, user.getSipPasswordHash(getCoreContext().getAuthorizationRealm()));
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
        ImAccount imAccount = new ImAccount(user);
        top.put(IM_ENABLED, imAccount.isEnabled());
        // The following settings used to be in contact-information.xml
        top.put(IM_ID, imAccount.getImId());
        top.put(IM_DISPLAY_NAME, imAccount.getImDisplayName());
        AddressBookEntry abe = user.getAddressBookEntry();
        if (abe != null) {
            top.put(ALT_IM_ID, abe.getAlternateImId());
            top.put(JOB_TITLE, abe.getJobTitle());
            top.put(JOB_DEPT, abe.getJobDept());
            top.put(COMPANY_NAME, abe.getCompanyName());
            top.put(ASSISTANT_NAME, abe.getAssistantName());
            top.put(ASSISTANT_PHONE, abe.getAssistantPhoneNumber());
            top.put(FAX_NUMBER, abe.getFaxNumber());
            top.put(HOME_PHONE_NUMBER, abe.getHomePhoneNumber());
            top.put(CELL_PHONE_NUMBER, abe.getCellPhoneNumber());
            top.put(LOCATION, abe.getLocation());
            // FIXME abe.getOfficeAddress should be accurate enough to get real office address
            // complete fix should be when XX-8002 gets solved
            Address officeAddress = null;
            Branch site = user.getSite();
            if (abe.getUseBranchAddress() && site != null) {
                officeAddress = site.getAddress();
            } else {
                officeAddress = abe.getOfficeAddress();
            }
            Address home = abe.getHomeAddress();
            if (home != null) {
                top.put(HOME_STREET, home.getStreet());
                top.put(HOME_CITY, home.getCity());
                top.put(HOME_COUNTRY, home.getCountry());
                top.put(HOME_STATE, home.getState());
                top.put(HOME_ZIP, home.getZip());
            }
            if (officeAddress != null) {
                top.put(OFFICE_STREET, officeAddress.getStreet());
                top.put(OFFICE_CITY, officeAddress.getCity());
                top.put(OFFICE_COUNTRY, officeAddress.getCountry());
                top.put(OFFICE_STATE, officeAddress.getState());
                top.put(OFFICE_ZIP, officeAddress.getZip());
                top.put(OFFICE_DESIGNATION, officeAddress.getOfficeDesignation());
            }
        }
        top.put(CONF_ENTRY_IM, user.getSettingValue("im_notification/conferenceEntryIM").toString());
        top.put(CONF_EXIT_IM, user.getSettingValue("im_notification/conferenceExitIM").toString());
        top.put(LEAVE_MESSAGE_BEGIN_IM, user.getSettingValue("im_notification/leaveMsgBeginIM").toString());
        top.put(LEAVE_MESSAGE_END_IM, user.getSettingValue("im_notification/leaveMsgEndIM").toString());
        //and this one in presencerouting-prefs.xml
        top.put(VMONDND, imAccount.isForwardOnDnd());
        //settings from xmpp-account-info.xml
        top.put(IM_ON_THE_PHONE_MESSAGE, imAccount.getOnThePhoneMessage());
        top.put(IM_ADVERTISE_ON_CALL_STATUS, imAccount.advertiseSipPresence());
        top.put(IM_SHOW_ON_CALL_DETAILS, imAccount.includeCallInfo());
        top.put(IM_PASSWORD, imAccount.getImPassword());
        getDbCollection().save(top);
    }

    @Override
    protected DataSet getType() {
        return DataSet.ATTENDANT;
    }
}
