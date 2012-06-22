/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.voicemail;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.DistributionList;
import org.sipfoundry.commons.userdb.Distributions;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.common.DialByName;
import org.sipfoundry.sipxivr.common.DialByNameChoice;
import org.sipfoundry.sipxivr.common.IvrChoice;
import org.sipfoundry.sipxivr.common.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.mailbox.GreetingType;
import org.sipfoundry.voicemail.mailbox.MailboxManager;
import org.sipfoundry.voicemail.mailbox.TempMessage;

public abstract class AbstractVmAction implements VmAction {

    private VmEslRequestController m_controller;
    protected MailboxManager m_mailboxManager;
    private HashMap<String, DistributionList> m_sysDistLists;
    private ValidUsers m_validUsers;

    public User getCurrentUser() {
        return m_controller.getCurrentUser();
    }

    public void changeLocale(String localeString) {
        m_controller.changeLocale(localeString);
    }

    public GreetingType getActiveGreeting() {
        if (getCurrentUser().getActiveGreeting() != null) {
            GreetingType active = GreetingType.valueOfById(getCurrentUser().getActiveGreeting());
            if (active != null) {
                return active;
            }
        }
        return GreetingType.STANDARD;
    }

    public Greeting createGreeting() {
        return new Greeting(getCurrentUser(), m_mailboxManager.getRecordedName(getCurrentUser().getUserName()));
    }

    public String getDisplayUri() {
        return m_controller.getDisplayUri();
    }

    public String getChannelUniqueId() {
        return m_controller.getChannelUniqueId();
    }

    public void play(PromptList pl, String digitMask) {
        m_controller.play(pl, digitMask);
    }

    public void play(String fragment, String digitMask) {
        m_controller.play(fragment, digitMask);
    }

    public void transfer(String dest, boolean playGreeting, boolean disconnect) {
        m_controller.transfer(dest, playGreeting, disconnect);
    }

    public void recordMessage(TempMessage message) {
        m_controller.recordMessage(message.getTempPath());
    }

    public String collectDigits(int maxDigits, int firstDigitTimer, int interDigitTimer, int extraDigitTimer,
            String termChars) {
        return m_controller.collectDigits(maxDigits, firstDigitTimer, interDigitTimer, extraDigitTimer, termChars);
    }

    public VmMenu createVmMenu() {
        return new VmMenu(m_controller);
    }

    public VmDialog createVmDialog(String menuFragment) {
        return new VmDialog(m_controller, menuFragment);
    }

    public PromptList getPromptList() {
        return m_controller.getPromptList();
    }

    public void goodbye() {
        m_controller.goodbye();
    }

    public PromptList getPromptList(String fragment) {
        return m_controller.getPromptList(fragment);
    }

    public PromptList getPromptList(String fragment, String... vars) {
        return m_controller.getPromptList(fragment, vars);
    }

    public String getDtmfDigit() {
        return m_controller.getDtmfDigit();
    }

    public String getChannelCallerIdName() {
        return m_controller.getChannelCallerIdName();
    }

    public String getChannelCallerIdNumber() {
        return m_controller.getChannelCallerIdNumber();
    }

    public DialByNameChoice createDialByNameDialog() {
        Vector<User> userList = new Vector<User>();

        for (;;) {
            // "Please dial an extension."
            // "Press 8 to use a distribution list,"
            // "Or press 9 for the dial by name directory."
            PromptList pl = getPromptList("dial_extension");
            VmMenu menu = createVmMenu();
            IvrChoice choice = menu.collectDigits(pl, 10);

            if (!menu.isOkay()) {
                return new DialByNameChoice(choice);
            }

            String digits = choice.getDigits();

            if (digits.equals("8")) {
                Vector<User> users = selectDistributionList();
                if (users == null) {
                    continue;
                }
                userList.addAll(users);
                break;

            } else if (digits.equals("9")) {
                // Do the DialByName dialog
                DialByName dbn = new DialByName();
                dbn.setApplicationConfiguration(m_controller.getVoicemailConfiguration());
                dbn.setLocalization(m_controller.getLocalization());
                dbn.setValidUsers(m_validUsers);
                dbn.setMailboxManager(m_mailboxManager);
                dbn.setOnlyVoicemailUsers(true);
                DialByNameChoice dbnChoice = dbn.dialByName();

                // If they canceled DialByName, backup
                if (dbnChoice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                    continue;
                }

                // If an error occurred, failure
                if (dbnChoice.getUsers() == null) {
                    m_controller.failure();
                    return new DialByNameChoice(dbnChoice);
                }

                // Add the selected user to the list
                userList.addAll(dbnChoice.getUsers());
                break;
            } else {
                User user = m_validUsers.getUser(digits);
                if (user == null || !user.hasVoicemail()) {
                    // "that extension is not valid"
                    play("invalid_extension", "");
                    continue;
                }
                userList.add(user);
                break;
            }
        }
        return new DialByNameChoice(userList, "", IvrChoiceReason.SUCCESS);
    }

    /**
     * Select a distribution list from the list of lists. Boy is this confuzing!
     * 
     * @return A list of users on the distribution list, null on error
     */
    public Vector<User> selectDistributionList() {
        String validDigits = "123456789";
        Distributions distributions = null;

        // See if the new way to get distribution lists is being used.
        Map<String, DistributionList> dlists = null;
        if (dlists == null) {
            // Use the old way (distributionListsFile in the user's mailbox directory)
            distributions = getCurrentUser().getDistributions();
            if (distributions != null) {
                validDigits = distributions.getIndices();
            }
        }

        for (;;) {
            // "Please select the distribution list.  Press * to cancel."
            PromptList pl = getPromptList("deposit_select_distribution");
            Menu menu = createVmMenu();
            IvrChoice choice = menu.collectDigit(pl, validDigits);

            if (!menu.isOkay()) {
                return null;
            }
            String digit = choice.getDigits();

            Collection<String> userNames = null;
            if (dlists != null) {
                DistributionList list = dlists.get(digit);
                if (list != null) {
                    userNames = list.getList(m_sysDistLists);
                }
            } else if (distributions != null) {
                userNames = Arrays.asList(distributions.getList(digit));
            }

            if (userNames != null) {
                Vector<User> users = new Vector<User>();
                for (String userName : userNames) {
                    User u = m_validUsers.getUser(userName);
                    if (u != null && u.hasVoicemail()) {
                        users.add(u);
                    }
                }

                return users;
            }

            // "The list you have selected is not valid"
            play("deposit_distribution_notvalid", "");
        }
    }

    public User getValidUser(String from) {
        return m_validUsers.getUser(from);
    }

    public HashMap<String, DistributionList> getSysDistList() {
        return m_sysDistLists;
    }

    public void failure() {
        m_controller.failure();
    }

    public void setRedactDTMF(boolean redactDTMF) {
        m_controller.setRedactDTMF(redactDTMF);
    }

    public String getRealm() {
        return m_controller.getFsConfig().getRealm();
    }

    public void recordMessage(String fileName) {
        m_controller.recordMessage(fileName);
    }

    public void transferToOperator() {
        m_controller.transferToOperator();
    }

    public int getInvalidResponseCount() {
        return m_controller.getVoicemailConfiguration().getInvalidResponseCount();
    }

    public void setEslRequestController(VmEslRequestController controller) {
        m_controller = controller;
    }

    public void setMailboxManager(MailboxManager manager) {
        m_mailboxManager = manager;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

}
