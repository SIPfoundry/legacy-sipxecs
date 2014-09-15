/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.springframework.beans.factory.annotation.Required;

public class ExportCsv {
    private static final int DEFAULT_PAGE_SIZE = 250;

    private CoreContext m_coreContext;

    private PhoneContext m_phoneContext;

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    private Collection<String> exportPhoneAndUsers(SimpleCsvWriter csv) throws IOException {
        Set<String> usernames = new HashSet<String>();
        final String[] order = new String[] {
            "serialNumber"
        };
        int phoneIndex = 0;
        int size = 0;
        do {
            List<Phone> phones = m_phoneContext.loadPhonesByPage(null, phoneIndex, DEFAULT_PAGE_SIZE, order,
                    true);
            size = phones.size();
            phoneIndex += size;
            for (Phone phone : phones) {
                usernames.addAll(exportPhone(csv, phone));
            }
        } while (size == DEFAULT_PAGE_SIZE);
        return usernames;
    }

    /**
     * Exports users for a single phone
     *
     * @param csv csw writere
     * @param phone phone to be exported
     * @param realm
     * @return list of user IDs exported with this phone
     */
    Collection<String> exportPhone(SimpleCsvWriter csv, Phone phone) throws IOException {
        String[] row = Index.newRow();

        String phoneSerialNumber = phone.getSerialNumber();
        if (phoneSerialNumber == null) {
            // nothing to export
            return Collections.emptyList();
        }
        Index.SERIAL_NUMBER.set(row, phoneSerialNumber);
        Index.MODEL_ID.set(row, phone.getModelId());
        Index.PHONE_GROUP.set(row, phone.getGroupsNames());
        Index.PHONE_DESCRIPTION.set(row, phone.getDescription());
        Index.ADDITIONAL_PHONE_SETTINGS.set(row, phone.getAdditionalPhoneSettings());

        // Now get the user(s) for each phone.
        List<Line> lines = phone.getLines();
        List<String> usernames = new ArrayList<String>(lines.size());
        for (Line line : lines) {
            User user = line.getUser();
            if (user == null) {
                // skip external lines
                continue;
            }

            line.setPaths(phone.getLinePaths());
            Index.ADDITIONAL_LINE_SETTINGS.set(row, line.getAdditionalLineSettings());

            String userName = user.getUserName();
            // Add username to list that shows this user is associated with a phone.
            usernames.add(userName);

            exportUser(csv, row, user);
        }

        if (usernames.isEmpty()) {
            // no lines or external lines only - write "phone only" line
            csv.write(row, true);
        }
        return usernames;
    }

    void exportUser(SimpleCsvWriter csv, String[] row, User user) throws IOException {
        Index.USERNAME.set(row, user.getUserName());

        Index.SIP_PASSWORD.set(row, user.getSipPassword());
        Index.FIRST_NAME.set(row, user.getFirstName());
        Index.LAST_NAME.set(row, user.getLastName());
        Index.ALIAS.set(row, user.getAliasesString());
        Index.USER_GROUP.set(row, user.getGroupsNames());
        Index.EMAIL.set(row, user.getEmailAddress());

        Index.PIN.set(row, user.getPintoken());
        Index.VOICEMAIL_PIN.set(row, user.getVoicemailPintoken());
        // XMPP
        Index.IM_ID.set(row, user.getImId());

        if (user.getUserProfile() != null) {
            UserProfile profile = user.getUserProfile();

            Index.SALUTATION.set(row, profile.getSalutation());
            Index.MANAGER.set(row, profile.getManager());
            Index.EMPLOYEE_ID.set(row, profile.getEmployeeId());
            Index.JOB_TITLE.set(row, profile.getJobTitle());
            Index.JOB_DEPT.set(row, profile.getJobDept());
            Index.COMPANY_NAME.set(row, profile.getCompanyName());
            Index.ASSISTANT_NAME.set(row, profile.getAssistantName());
            Index.CELL_PHONE_NUMBER.set(row, profile.getCellPhoneNumber());
            Index.HOME_PHONE_NUMBER.set(row, profile.getHomePhoneNumber());
            Index.ASSISTANT_PHONE_NUMBER.set(row, profile.getAssistantPhoneNumber());
            Index.FAX_NUMBER.set(row, profile.getFaxNumber());
            Index.DID_NUMBER.set(row, profile.getDidNumber());
            Index.ALTERNATE_EMAIL.set(row, profile.getAlternateEmailAddress());
            Index.ALTERNATE_IM_ID.set(row, profile.getAlternateImId());
            Index.LOCATION.set(row, profile.getLocation());

            if (profile.getHomeAddress() != null) {
                Address homeAddress = profile.getHomeAddress();
                Index.HOME_STREET.set(row, homeAddress.getStreet());
                Index.HOME_CITY.set(row, homeAddress.getCity());
                Index.HOME_STATE.set(row, homeAddress.getState());
                Index.HOME_COUNTRY.set(row, homeAddress.getCountry());
                Index.HOME_ZIP.set(row, homeAddress.getZip());
            }
            if (profile.getOfficeAddress() != null) {
                Address officeAddress = profile.getOfficeAddress();
                Index.OFFICE_STREET.set(row, officeAddress.getStreet());
                Index.OFFICE_CITY.set(row, officeAddress.getCity());
                Index.OFFICE_STATE.set(row, officeAddress.getState());
                Index.OFFICE_COUNTRY.set(row, officeAddress.getCountry());
                Index.OFFICE_ZIP.set(row, officeAddress.getZip());
                Index.OFFICE_MAIL_STOP.set(row, officeAddress.getOfficeDesignation());
            }
            Index.TWITTER_NAME.set(row, profile.getTwiterName());
            Index.LINKEDIN_NAME.set(row, profile.getLinkedinName());
            Index.FACEBOOK_NAME.set(row, profile.getFacebookName());
            Index.XING_NAME.set(row, profile.getXingName());
            Index.AUTH_ACCOUNT_NAME.set(row, profile.getAuthAccountName());
            Index.EMAIL_ADDRESS_ALIASES.set(row, profile.getEmailAddressAliases());
            Index.CUSTOM_1.set(row, profile.getCustom1());
            Index.CUSTOM_2.set(row, profile.getCustom2());
            Index.CUSTOM_3.set(row, profile.getCustom3());
        }

        // voice mail settings
        MailboxPreferences mailboxPreferences = new MailboxPreferences(user);
        Index.ACTIVE_GREETING.set(row, mailboxPreferences.getActiveGreeting().getId());
        Index.PRIMARY_EMAIL_NOTIFICATION.set(row, mailboxPreferences.getAttachVoicemailToEmail().getValue());
        Index.PRIMARY_EMAIL_FORMAT.set(row, mailboxPreferences.getEmailFormat().name());
        Index.PRIMARY_EMAIL_ATTACH_AUDIO.set(row, String.valueOf(mailboxPreferences.isIncludeAudioAttachment()));
        Index.ALT_EMAIL_NOTIFICATION.set(row, mailboxPreferences.getVoicemailToAlternateEmailNotification()
                .getValue());
        Index.ALT_EMAIL_FORMAT.set(row, mailboxPreferences.getAlternateEmailFormat().name());
        Index.ALT_EMAIL_ATTACH_AUDIO.set(row,
                String.valueOf(mailboxPreferences.isIncludeAudioAttachmentAlternateEmail()));
        Index.VOICEMAIL_SERVER.set(row, String.valueOf(user.isVoicemailServer()));

        // user caller alias
        UserCallerAliasInfo callerAlias = new UserCallerAliasInfo(user);
        Index.EXTERNAL_NUMBER.set(row, callerAlias.getExternalNumber());
        Index.ANONYMOUS_CALLER_ALIAS.set(row, String.valueOf(callerAlias.isAnonymous()));

        csv.write(row);
    }

    private void exportUsersNotAttachedToPhones(SimpleCsvWriter csv, Collection<String> usernames)
        throws IOException {
        int userIndex = 0;
        int size = 0;
        do {
            List<User> users = m_coreContext.loadUsersByPage(null, null, null, userIndex, DEFAULT_PAGE_SIZE,
                    "userName", true);
            size = users.size();
            userIndex += size;
            for (User user : users) {
                String userName = user.getUserName();
                if (!usernames.contains(userName)) {
                    String[] row = Index.newRow();
                    exportUser(csv, row, user);
                }
            }
        } while (size == DEFAULT_PAGE_SIZE);
    }

    public void exportCsv(Writer writer) throws IOException {
        try {
            // Write the Header of the CSV file.
            SimpleCsvWriter csv = new SimpleCsvWriter(writer);
            csv.write(Index.labels(), false);

            // Export Phones and associated users
            Collection<String> usernames = exportPhoneAndUsers(csv);

            // Export Users that are not associated with any phone.
            exportUsersNotAttachedToPhones(csv, usernames);

        } finally {
            writer.flush();
        }
    }
}
