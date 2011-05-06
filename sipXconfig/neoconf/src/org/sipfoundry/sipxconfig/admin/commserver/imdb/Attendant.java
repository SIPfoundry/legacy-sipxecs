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
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

import static org.sipfoundry.commons.mongo.MongoConstants.*;

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

        // The following settings used to be in contact-information.xml
        ImAccount imAccount = new ImAccount(user);
        top.put(IM_ID, imAccount.getImId());
        top.put(IM_DISPLAY_NAME, imAccount.getImDisplayName());
        //and this one in presencerouting-prefs.xml
        top.put(VMONDND, imAccount.isForwardOnDnd());
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
        getDbCollection().save(top);
    }

    @Override
    protected DataSet getType() {
        return DataSet.ATTENDANT;
    }
}
