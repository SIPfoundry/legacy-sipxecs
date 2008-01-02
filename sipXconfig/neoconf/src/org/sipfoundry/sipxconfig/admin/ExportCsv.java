/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class ExportCsv {
    private CoreContext m_coreContext;

    private PhoneContext m_phoneContext;

    private MailboxManager m_mailboxManager;

    private List<String> m_usersLinkedToPhones;

    private String m_realmString;

    private String m_emptyString = "";

    private String m_hashString = "#";

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    private void exportPhoneAndUsers(CsvWriter csv) {

        // Now get all of the phone/user information.
        int phoneCount = m_phoneContext.getPhonesCount();
        int phoneIndex = 0;
        int pageSize;
        while (phoneIndex != phoneCount) {
            if ((phoneCount - phoneIndex) < 25) {
                pageSize = phoneCount - phoneIndex;
            } else {
                pageSize = 25;
            }

            List<Phone> phones = m_phoneContext.loadPhonesByPage(null, phoneIndex, pageSize,
                                 new String[] {"serialNumber"}, true);
            phoneIndex += pageSize;
            for (int i = 0; i < phones.size(); i++) {
                // Get the phone information first.
                String phoneSerialNumber = m_emptyString;
                String phoneModel = m_emptyString;
                String phoneGroup = m_emptyString;
                String phoneDescription = m_emptyString;

                // There could be multiple users per phone.
                String userName = m_emptyString;
                String userPin = m_emptyString;
                String userPinToken = m_emptyString;
                String userSipPassword = m_emptyString;
                String userFirstName = m_emptyString;
                String userLastName = m_emptyString;
                String userAlias = m_emptyString;
                String userEmail = m_emptyString;
                String userGroup = m_emptyString;

                phoneSerialNumber = phones.get(i).getSerialNumber();
                if (phoneSerialNumber == null) {
                    phoneSerialNumber = m_emptyString;
                } else {
                    //
                    // serialNumber is not blank.
                    // Get the rest of the phone fields
                    //
                    phoneModel = phones.get(i).getModelId();
                    if (phoneModel == null) {
                        phoneModel = m_emptyString;
                    }

                    List<Group> groupList = phones.get(i).getGroupsAsList();
                    if (groupList.size() != 0) {
                        phoneGroup = groupList.get(0).getName();
                        if (phoneGroup == null) {
                            phoneGroup = m_emptyString;
                        }
                    }
                    phoneDescription = phones.get(i).getDescription();
                    if (phoneDescription == null) {
                        phoneDescription = m_emptyString;
                    }

                    // Now get the user(s) for each phone.
                    List<Line> lineList = phones.get(i).getLines();
                    if (lineList.size() == 0) {
                        //
                        // No users are associated with this phone.
                        // So write this out to the file.
                        //
                        String[] userPhoneStringArray = {userName, userPinToken, userSipPassword, userFirstName,
                            userLastName, userAlias, userEmail, userGroup,
                            phoneSerialNumber, phoneModel, phoneGroup, phoneDescription};
                        try {
                            csv.write(userPhoneStringArray, false);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        }
                    } else {
                        for (int k = 0; k < lineList.size(); k++) {
                            User user = lineList.get(k).getUser();
                            userName = user.getUserName();
                            if (userName == null) {
                                userName = m_emptyString;
                            } else {
                                //
                                // Add username to list that shows this user is associated with a phone.
                                //
                                m_usersLinkedToPhones.add(userName);
                            }

                            userPinToken = user.getPintoken();
                            if (userPinToken == null) {
                                userPinToken = m_emptyString;
                            }
                            userSipPassword = user.getSipPassword();
                            if (userSipPassword == null) {
                                userSipPassword = m_emptyString;
                            }
                            userFirstName = user.getFirstName();
                            if (userFirstName == null) {
                                userFirstName = m_emptyString;
                            }
                            userLastName = user.getLastName();
                            if (userLastName == null) {
                                userLastName = m_emptyString;
                            }
                            userAlias = user.getAliasesString();
                            if (userAlias == null) {
                                userAlias = m_emptyString;
                            }

                            // userEmail..
                            if (m_mailboxManager.isEnabled()) {
                                Mailbox mailbox = m_mailboxManager.getMailbox(userName);
                                MailboxPreferences mboxPrefs = m_mailboxManager
                                        .loadMailboxPreferences(mailbox);
                                userEmail = mboxPrefs.getEmailAddress();
                            }
                            List<Group> userGroupList = user.getGroupsAsList();
                            if (userGroupList.size() != 0) {
                                userGroup = userGroupList.get(0).getName();
                                if (userGroup == null) {
                                    userGroup = m_emptyString;
                                }
                            }
                            String tempStr = m_realmString + m_hashString + userPinToken;
                            userPinToken = tempStr;

                            // Now write phone/user out to the file.
                            String[] userPhoneStringArray = {userName, userPinToken, userSipPassword, userFirstName,
                                userLastName, userAlias, userEmail, userGroup, phoneSerialNumber,
                                phoneModel, phoneGroup, phoneDescription};

                            try {
                                csv.write(userPhoneStringArray, false);
                            } catch (IOException e) {
                                throw new RuntimeException(e);
                            }
                        }
                    }
                }
            }
        }
    }

    private void exportUsersNotAttachedToPhones(CsvWriter csv) {
        int userCount = m_coreContext.getUsersCount();
        int userIndex = 0;
        int pageSize;

        String userName = m_emptyString;
        String userPinToken = m_emptyString;
        String userSipPassword = m_emptyString;
        String userFirstName = m_emptyString;
        String userLastName = m_emptyString;
        String userAlias = m_emptyString;
        String userEmail = m_emptyString;
        String userGroup = m_emptyString;

        while (userIndex != userCount) {
            if ((userCount - userIndex) < 25) {
                pageSize = userCount - userIndex;
            } else {
                pageSize = 25;
            }

            List<User> users = m_coreContext.loadUsersByPage(null, null, userIndex, pageSize, "userName", true);
            userIndex += pageSize;
            for (int i = 0; i < users.size(); i++) {
                userName = users.get(i).getUserName();

                if ((userName != null) && (!m_usersLinkedToPhones.contains(userName))) {
                    //
                    // This user is not linked to a phone.
                    // So write it out to the export file.
                    //
                    userPinToken = users.get(i).getPintoken();
                    if (userPinToken == null) {
                        userPinToken = m_emptyString;
                    }
                    userSipPassword = users.get(i).getSipPassword();
                    if (userSipPassword == null) {
                        userSipPassword = m_emptyString;
                    }
                    userFirstName = users.get(i).getFirstName();
                    if (userFirstName == null) {
                        userFirstName = m_emptyString;
                    }
                    userLastName = users.get(i).getLastName();
                    if (userLastName == null) {
                        userLastName = m_emptyString;
                    }
                    userAlias = users.get(i).getAliasesString();
                    if (userAlias == null) {
                        userAlias = m_emptyString;
                    }

                    // userEmail..
                    if (m_mailboxManager.isEnabled()) {
                        Mailbox mailbox = m_mailboxManager.getMailbox(userName);
                        MailboxPreferences mboxPrefs = m_mailboxManager
                                .loadMailboxPreferences(mailbox);
                        userEmail = mboxPrefs.getEmailAddress();
                    }
                    List<Group> userGroupList = users.get(i).getGroupsAsList();
                    if (userGroupList.size() != 0) {
                        userGroup = userGroupList.get(0).getName();
                        if (userGroup == null) {
                            userGroup = m_emptyString;
                        }
                    }
                    String tempStr = m_realmString + m_hashString + userPinToken;
                    userPinToken = tempStr;

                    // Now finally write this out to the file.
                    String[] userPhoneStringArray = {userName, userPinToken, userSipPassword, userFirstName,
                        userLastName, userAlias, userEmail, userGroup, m_emptyString,
                        m_emptyString, m_emptyString, m_emptyString};

                    try {
                        csv.write(userPhoneStringArray, false);
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                }
            }
        } // end while
    }

    public void exportCsv(Writer writer) {

        try {
            //
            // Get the realmString to save with the Voice-mail PIN.
            //
            m_realmString = m_coreContext.getAuthorizationRealm();

            //
            // Create a list for usernames that are associated with phones.
            // This is required so we can determine if there are any users
            // that are not associated with phones.
            //
            m_usersLinkedToPhones = new ArrayList<String>();

            //
            // Write the Header of the CSV file.
            //
            CsvWriter csv = new CsvWriter(writer);
            String[] exportHeaderStringArray = {
                "User name", "Voice-mail PIN", "SIP password", "First name", "Last name",
                "User alias", "EMail address", "User group", "Phone serial number", "Phone model",
                "Phone group", "Phone description"
            };
            csv.write(exportHeaderStringArray, false);

            //
            // Export Phones and associated users (if any).
            //
            exportPhoneAndUsers(csv);

            //
            // Export Users that are not associated with any phone.
            //
            exportUsersNotAttachedToPhones(csv);

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
