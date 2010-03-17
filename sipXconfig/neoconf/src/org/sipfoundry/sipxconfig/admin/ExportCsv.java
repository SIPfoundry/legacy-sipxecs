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
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class ExportCsv {
    private static final int DEFAULT_PAGE_SIZE = 250;

    private CoreContext m_coreContext;

    private PhoneContext m_phoneContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

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

        // Now get the user(s) for each phone.
        List<Line> lines = phone.getLines();
        List<String> usernames = new ArrayList<String>(lines.size());
        for (Line line : lines) {
            User user = line.getUser();
            if (user == null) {
                // skip external lines
                continue;
            }
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
