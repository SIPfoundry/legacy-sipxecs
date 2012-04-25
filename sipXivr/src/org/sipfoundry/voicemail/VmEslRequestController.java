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

import java.util.Hashtable;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.commons.alarm.SipXAlarmClient;
import org.sipfoundry.commons.freeswitch.Play;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.Record;
import org.sipfoundry.commons.userdb.PersonalAttendant;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.ApplicationConfiguraton;
import org.sipfoundry.sipxivr.common.IvrChoice;
import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;
import org.sipfoundry.voicemail.mailbox.TempMessage;

public class VmEslRequestController extends AbstractEslRequestController {
    private static final String RESOURCE_NAME = "org.sipfoundry.voicemail.VoiceMail";
    private static final String FAILED_LOGIN_ALARM_ID = "VM_LOGIN_FAILED";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String m_action;
    private String m_mailboxString;
    private ValidUsers m_validUsers;
    private User m_currentUser;
    private String m_operatorAddr;
    private ApplicationConfiguraton m_config;
    private SipXAlarmClient m_alarmClient;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        parseDiversionHeader(parameters);
        m_action = parameters.get("action");
        extractCurrentUser(parameters);
    }

    @Override
    public void loadConfig() {
        initLocalization("VoiceMail", RESOURCE_NAME);
        // set user's locale
        if (m_currentUser != null) {
            if (m_currentUser.getLocale() == null) {
                // Set the locale for this user to be that passed with the call
                m_currentUser.setLocale(getLocalization().getLocale());
            }
        }
    }

    public boolean hasValidMailbox() {
        if (m_currentUser == null || !m_currentUser.hasVoicemail()) {
            // That extension is not valid.
            LOG.info("Extension " + m_mailboxString + " is not valid.");
            play("invalid_extension", "");
            goodbye();
            return false;
        }
        return true;
    }

    public void login() {
        setRedactDTMF(true);
        trimDtmfQueue("");

        boolean playWelcome = true;
        int errorCount = 0;
        String ident;
        if (m_currentUser != null) {
            ident = "Mailbox " + m_currentUser.getUserName();
            if (!m_currentUser.hasVoicemail() && !m_currentUser.isInDirectory()) {
                m_currentUser = null;
                play("invalid_extension", "");
                return;
            }
        } else {
            ident = "Mailbox (unknown)";
        }
        PromptList welcomePl = getPromptList("welcome");
        for (;;) {
            if (errorCount > m_config.getInvalidResponseCount() && m_currentUser != null) {
                try {
                    m_alarmClient.raiseAlarm(FAILED_LOGIN_ALARM_ID, m_currentUser.getUserName());
                } catch (XmlRpcException ex) {
                    LOG.error("Problem sending alarm failed login alarm", ex);
                }
                failure();
                m_currentUser = null;
                break;
            }

            if (m_currentUser == null) {
                // "Enter your extension."
                PromptList extPl = getPromptList("enter_extension");
                VmMenu extMenu = new VmMenu(this);
                if (playWelcome) {
                    extMenu.setPrePromptPl(welcomePl);
                    playWelcome = false;
                }

                IvrChoice extChoice = extMenu.collectDigits(extPl, 10);
                if (extMenu.isCanceled()) {
                    continue;
                }
                if (!extMenu.isOkay()) {
                    break;
                }
                LOG.info("Retrieve::login " + ident + " changing to extension " + extChoice.getDigits());

                // See if the user exists
                m_currentUser = m_validUsers.getUser(extChoice.getDigits());
            }

            // "Enter your personal identification number, and then press #.":
            // "To log in as a different user, press #"
            PromptList menuPl = getPromptList("enter_pin");
            VmMenu menu = new VmMenu(this);
            if (playWelcome) {
                menu.setPrePromptPl(welcomePl);
                playWelcome = false;
            }

            // Note: Not using collectDigits() here, as it doesn't allow initial "#" to barge,
            // and "*" to cancel doesn't really make sense. Just treat as invalid.
            IvrChoice choice = menu.collectDtmf(menuPl, 10);

            if (!menu.isOkay()) {
                m_currentUser = null;
                break;
            }

            if (choice.getDigits().equals("#")) {
                // reset user, login again
                m_currentUser = null;
                continue;
            }

            // Only users with voicemail, or are in the directory, are allowed
            // (directory users can record their name, that's all)
            if (m_currentUser != null && !m_currentUser.hasVoicemail() && !m_currentUser.isInDirectory()) {
                LOG.info("Retrieve::login user " + m_currentUser.getUserName() + " doesn't have needed permissions.");
            }

            if (m_currentUser == null || !m_currentUser.isPinCorrect(choice.getDigits(), getFsConfig().getRealm())) {
                // WRONG, do it again!

                // "That personal identification number is not valid
                play("invalid_pin", "");
                ++errorCount;
                continue;
            }
            break;
        }
        setRedactDTMF(false);
    }

    public void transferToOperator() {
        transfer(getOperator(getCurrentUser().getPersonalAttendant()), true);
    }

    /**
     * Get the operator URL defined for this user. Uses the PersonalAttendant's operator if
     * defined, else the systems.
     * 
     * @param pa
     * @return
     */
    public String getOperator(PersonalAttendant pa) {
        String transferUrl;
        // Try the Personal Attendant's definition of operator
        transferUrl = pa.getOperator();
        if (transferUrl == null) {
            // Try the system's definition of operator
            transferUrl = m_operatorAddr;
        }
        return transferUrl;
    }

    public void recordMessage(TempMessage message) {
        recordMessage(message.getTempWavPath());
    }

    /**
     * Record a message into a file named wavName
     * 
     * @param wavName
     * @return The Recording object
     */
    public void recordMessage(String wavName) {
        // Flush any typed ahead digits
        getFsEventSocket().trimDtmfQueue("");
        Record rec = new Record(getFsEventSocket(), getLocalization().getPromptList("beep"));
        rec.setRecordFile(wavName);
        rec.setRecordTime(300);
        rec.setDigitMask("0123456789*#i"); // Any digit can stop the recording
        rec.go();
    }

    /**
     * Perform the configured "failure" behavior, which can be either just hangup or transfer to a
     * destination after playing a prompt.
     * 
     */
    public void failure() {
        boolean playGoodbye = true;
        if (m_config.isTransferOnFailure()) {
            new Play(getFsEventSocket(), m_config.getTransferPrompt()).go();

            String dest = m_config.getTransferURL();
            if (!dest.toLowerCase().contains("sip:")) {
                dest = extensionToUrl(dest);
            }
            transfer(dest, false);
            playGoodbye = false;
        }

        goodbye(playGoodbye);
    }

    public String getDtmfDigit() {
        return getFsEventSocket().getDtmfDigit();
    }

    public String getAction() {
        return m_action;
    }

    public User getCurrentUser() {
        return m_currentUser;
    }

    public String getMailboxString() {
        return m_mailboxString;
    }

    public void setOperatorAddr(String operatorAddr) {
        m_operatorAddr = operatorAddr;
    }

    public void setVoicemailConfiguration(ApplicationConfiguraton config) {
        m_config = config;
    }

    public ApplicationConfiguraton getVoicemailConfiguration() {
        return m_config;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    private void extractCurrentUser(Hashtable<String, String> parameters) {
        // if division header origCalledNumber corresponds to a mailbox
        // then use it
        m_mailboxString = parameters.get("origCalledNumber");
        if (m_mailboxString != null) {
            // validate
            m_currentUser = m_validUsers.getUser(m_mailboxString);
            if (m_currentUser == null) {
                m_mailboxString = null;
            }
        }

        if (m_mailboxString == null) {
            m_mailboxString = parameters.get("mailbox");
        }

        if (m_mailboxString == null) {
            // Use the From: user as the mailbox
            m_mailboxString = getFsEventSocket().getFromUser();
        }
        m_currentUser = m_validUsers.getUser(m_mailboxString);

    }

    /*
     * diversion header looks like:
     * variable_sip_h_diversion=<tel:3948809>;reason=no-answer;counter=1;screen=no;privacy=off
     */
    private void parseDiversionHeader(Hashtable<String, String> parameters) {

        String divHeader = getFsEventSocket().getVariable("variable_sip_h_diversion");

        if (divHeader != null) {
            LOG.debug("SipXivr::parseDiversionHeader header=" + divHeader);
            divHeader = divHeader.toLowerCase();
            String[] subParms = divHeader.split(";");
            String ocn = null;

            // Look for the OCN format <tel:3948809>
            if (ocn == null) {
                ocn = getOcn(divHeader, "<tel:");
            }

            // Look for the OCN format <sip:3948809@196.8.1.7>
            if (ocn == null) {
                ocn = ValidUsers.getUserPart(getOcn(divHeader, "<sip:"));
            }

            if (ocn != null) {
                LOG.debug("SipXivr::parseDiversionHeader OCN=" + ocn);
                parameters.put("action", "deposit");
                parameters.put("origCalledNumber", ocn);

                // now look for call forward reason
                for (String param : subParms) {
                    if (param.startsWith("reason=")) {
                        param = param.substring("reason=".length());
                        param.trim();
                        parameters.put("call-forward-reason", param);
                        break;
                    }
                }
            }
        }
    }

    private String getOcn(String divHeader, String prefix) {
        int index = divHeader.indexOf(prefix);
        if (index >= 0) {
            divHeader = divHeader.substring(index + 5);
            index = divHeader.indexOf(">");
            if (index > 0) {
                return divHeader.substring(0, index);
            }
        }
        return null;
    }

    public void setAlarmClient(SipXAlarmClient client) {
        m_alarmClient = client;
    }

}
