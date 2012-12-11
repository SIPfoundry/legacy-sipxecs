/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
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

    private Collection<String> exportPhoneAndUsers(CsvWriter csv, String realm) throws IOException {
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
                usernames.addAll(exportPhone(csv, phone, realm));
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
    Collection<String> exportPhone(CsvWriter csv, Phone phone, String realm) throws IOException {
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

            exportUser(csv, row, user, realm);
        }

        if (usernames.isEmpty()) {
            // no lines or external lines only - write "phone only" line
            csv.write(row, true);
        }
        return usernames;
    }

    void exportUser(CsvWriter csv, String[] row, User user, String realm) throws IOException {
        Index.USERNAME.set(row, user.getUserName());

        Index.SIP_PASSWORD.set(row, user.getSipPassword());
        Index.FIRST_NAME.set(row, user.getFirstName());
        Index.LAST_NAME.set(row, user.getLastName());
        Index.ALIAS.set(row, user.getAliasesString());
        Index.USER_GROUP.set(row, user.getGroupsNames());
        Index.EMAIL.set(row, user.getEmailAddress());

        String userPinToken = user.getPintoken();
        Index.PIN.set(row, formatRealmAndHash(realm, userPinToken));
        // XMPP
        Index.IM_ID.set(row, user.getImId());

        if (user.getAddressBookEntry() != null) {
            AddressBookEntry addressBookEntry = user.getAddressBookEntry();

            Index.JOB_TITLE.set(row, addressBookEntry.getJobTitle());
            Index.JOB_DEPT.set(row, addressBookEntry.getJobDept());
            Index.COMPANY_NAME.set(row, addressBookEntry.getCompanyName());
            Index.ASSISTANT_NAME.set(row, addressBookEntry.getAssistantName());
            Index.CELL_PHONE_NUMBER.set(row, addressBookEntry.getCellPhoneNumber());
            Index.HOME_PHONE_NUMBER.set(row, addressBookEntry.getHomePhoneNumber());
            Index.ASSISTANT_PHONE_NUMBER.set(row, addressBookEntry.getAssistantPhoneNumber());
            Index.FAX_NUMBER.set(row, addressBookEntry.getFaxNumber());
            Index.ALTERNATE_EMAIL.set(row, addressBookEntry.getAlternateEmailAddress());
            Index.ALTERNATE_IM_ID.set(row, addressBookEntry.getAlternateImId());
            Index.LOCATION.set(row, addressBookEntry.getLocation());
            Index.DID_NUMBER.set(row, addressBookEntry.getDidNumber());

            if (addressBookEntry.getHomeAddress() != null) {
                Address homeAddress = addressBookEntry.getHomeAddress();
                Index.HOME_STREET.set(row, homeAddress.getStreet());
                Index.HOME_CITY.set(row, homeAddress.getCity());
                Index.HOME_STATE.set(row, homeAddress.getState());
                Index.HOME_COUNTRY.set(row, homeAddress.getCountry());
                Index.HOME_ZIP.set(row, homeAddress.getZip());
            }
            if (addressBookEntry.getOfficeAddress() != null) {
                Address officeAddress = addressBookEntry.getOfficeAddress();
                Index.OFFICE_STREET.set(row, officeAddress.getStreet());
                Index.OFFICE_CITY.set(row, officeAddress.getCity());
                Index.OFFICE_STATE.set(row, officeAddress.getState());
                Index.OFFICE_COUNTRY.set(row, officeAddress.getCountry());
                Index.OFFICE_ZIP.set(row, officeAddress.getZip());
                Index.OFFICE_MAIL_STOP.set(row, officeAddress.getOfficeDesignation());
            }
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

        csv.write(row, true);
    }

    private String formatRealmAndHash(String realm, String userPinToken) {
        return String.format("%s#%s", realm, userPinToken);
    }

    private void exportUsersNotAttachedToPhones(CsvWriter csv, Collection<String> usernames, String realm)
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
                    exportUser(csv, row, user, realm);
                }
            }
        } while (size == DEFAULT_PAGE_SIZE);
    }

    public void exportCsv(Writer writer) throws IOException {
        try {
            // Write the Header of the CSV file.
            CsvWriter csv = new CsvWriter(writer);
            csv.write(Index.labels(), false);

            // Export Phones and associated users
            String realm = m_coreContext.getAuthorizationRealm();
            Collection<String> usernames = exportPhoneAndUsers(csv, realm);

            // Export Users that are not associated with any phone.
            exportUsersNotAttachedToPhones(csv, usernames, realm);

        } finally {
            writer.flush();
        }
    }
}
