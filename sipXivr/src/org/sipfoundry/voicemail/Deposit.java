/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.util.Locale;
import java.util.Map;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.PersonalAttendant;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.IMSender;
import org.sipfoundry.commons.util.IMSender.HttpResult;
import org.sipfoundry.sipxivr.common.DialByNameChoice;
import org.sipfoundry.sipxivr.common.IvrChoice;
import org.sipfoundry.sipxivr.common.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.mailbox.MailboxManager;
import org.sipfoundry.voicemail.mailbox.TempMessage;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class Deposit extends AbstractVmAction implements ApplicationContextAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String m_sendIMUrl;
    private Map<String, String> m_depositMap;
    private ApplicationContext m_appContext;
    private String m_operatorAddr;

    /**
     * The depositVoicemail dialog
     *
     * @return
     */
    @Override
    public String runAction() {
        User user = getCurrentUser();
        PersonalAttendant pa = user.getPersonalAttendant();

        String localeString = pa.getLanguage();
        if (localeString != null) {
            LOG.debug("Changing locale for this call to " + localeString);
            changeLocale(localeString);
            user.setLocale(new Locale(localeString));
        }
        TempMessage tempMessage = null;

        Greeting greeting = createGreeting();
        PromptList pl = greeting.getPromptList(getPromptList(), getActiveGreeting(),
                m_mailboxManager.getGreetingPath(getCurrentUser(), getActiveGreeting()), user.shouldPlayDefaultVmOption());
        try {

            LOG.info("Mailbox " + user + " Deposit Voicemail from " + getDisplayUri());

            putChannelUUID(user, getChannelUniqueId());
            Greeting: for (;;) {
                // When you are finished, press 1 for more options.
                // To reach the operator, dial 0 at any time.

                // Allow caller to barge with 0, *, and any defined Personal Attendant digit
                // Also, allow barge to recording with "#"

                play(pl, "#0*i" + pa.getValidDigits());
                String digits = collectDigits(1, 100, 0, 0, "#");
                LOG.info("depositVoicemail Collected digits=" + digits);

                if (digits.equals("i")) {
                    play("please_hold", "");
                    transfer(user.getUri(), true, true);
                    return null;
                }

                if (digits.equals("*")) {
                    return "retrieve";
                }

                // See if the digit they pressed was defined in the Personal Attendant
                String transferUrl = null;
                if (digits.equals("0")) {
                    transferUrl = getOperatorUrl(pa);
                } else {
                    // See if the Personal Attendant defined that digit to mean anything
                    transferUrl = pa.getMenuValue(digits);
                }

                if (transferUrl != null) {
                    LOG.info(String.format("Transfer to %s", transferUrl));
                    transfer(transferUrl, true, true);
                    return null;
                }

                tempMessage = m_mailboxManager.createTempMessage(user.getUserName(), getDisplayUri(), true);

                boolean recorded = false;
                boolean playMessage = false;
                for (;;) {
                    // Record the message
                    if (!recorded) {
                        // So if they hang up now, we'll save what we got.
                        tempMessage.setIsToBeStored(true);
                        recordMessage(tempMessage);

                        String digit = getDtmfDigit();
                        if (digit != null && digit.equals("0")) {
                            if (tempMessage.getDuration() > 2) {
                                m_mailboxManager.storeInInbox(user, tempMessage);
                                play("msg_sent", "");
                            } else {
                                tempMessage.setIsToBeStored(false);
                            }

                            transfer(getOperatorUrl(pa), true, true);
                            return null;
                        }

                        if (digit != null && digit.equals("i")) {
                            tempMessage.setIsToBeStored(true);
                            play("please_hold", "");
                            transfer(user.getUri(), true, true);
                            return null;
                        }

                        LOG.info("Mailbox " + user.getUserName() + " Deposit Voicemail recorded message");
                        recorded = true;
                    }

                    // Confirm caller's intent for this message

                    Menu menu = createVmMenu();
                    if (playMessage) {
                        // (pre-menu: message)
                        PromptList messagePl = getPromptList();
                        messagePl.addPrompts(tempMessage.getTempPath());
                        menu.setPrePromptPl(messagePl);
                        playMessage = false; // Only play it once
                    }

                    // To play this message, press 1. To send this message, press 2.
                    // To delete this message and try again, press 3. To cancel, press *."
                    pl = getPromptList("deposit_options");
                    IvrChoice choice = ((VmMenu) menu).collectDigitIgnoreFailureOrTimeout(pl, "123");

                    // bad entry, timeout, canceled
                    if (!menu.isOkay()) {
                        if (choice.getIvrChoiceReason().equals(IvrChoiceReason.TIMEOUT)
                                || choice.getIvrChoiceReason().equals(IvrChoiceReason.FAILURE)) {
                            tempMessage.setIsToBeStored(true);
                            break;
                        } else {
                            tempMessage.setIsToBeStored(false);
                            goodbye();
                            return null;
                        }
                    }

                    String digit = choice.getDigits();

                    // "1" means play the message
                    if (digit.equals("1")) {
                        playMessage = true;
                        continue;
                    }

                    // "2" means send the message.
                    if (digit.equals("2")) {
                        if (tempMessage.getDuration() > 2) {
                            m_mailboxManager.storeInInbox(user, tempMessage);
                            break;
                        }
                        // Message was too short. Don't save the message.
                        tempMessage.setIsToBeStored(false);

                        // "Sorry, your message was too short and was not delivered."
                        play("msg_too_short", "");
                        continue Greeting;
                    }

                    // "3" means "erase" and re-record
                    if (digit.equals("3")) {
                        // if the user hangs up during playback of this prompt
                        // we don't want to save the message.
                        tempMessage.setIsToBeStored(false);
                        play("send_record_message", "");
                        recorded = false;
                        continue;
                    }
                }
                break;
            }

            clearChannelUUID(user, tempMessage);

            // "Your message has been recorded."
            play("deposit_recorded", "");

            // Message sent, now see what else they want to do
            moreOptions(tempMessage);

            goodbye();

        } catch (DisconnectException e) {
        } finally {
            try {
                // Let FS finish any recording's it might be doing
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }

            clearChannelUUID(user, tempMessage);

            if (tempMessage != null) {
                // Deliver message that is pending; don't store "click" messages
                if (tempMessage.getDuration() > 1) {
                    m_mailboxManager.storeInInbox(user, tempMessage);
                }
                m_mailboxManager.deleteTempMessage(tempMessage);
            }
        }
        return null;
    }

    private String getOperatorUrl(PersonalAttendant pa) {
        String transferUrl = pa.getOperator();
        if (transferUrl == null) {
            transferUrl = m_operatorAddr;
        }
        return transferUrl;
    }

    private void putChannelUUID(User user, String uuid) {
        m_depositMap.put(user.getUserName(), uuid);
        String instantMsg = getChannelCallerIdName() + " (" + getChannelCallerIdName() + ") "
            + m_appContext.getMessage("leaving_msg", null, "is leaving a voice message.", user.getLocale());
        try {
            if (user.getVMEntryIM()) {
                HttpResult result = IMSender.sendVmEntryIM(user, instantMsg, m_sendIMUrl);
                if (!result.isSuccess()) {
                    LOG.error("Deposit::sendIM Trouble with RemoteRequest: "
                        + result.getResponse(), result.getException());
                }
            }
        } catch (Exception ex) {
            LOG.error("Deposit::sendIM failed", ex);
        }

    }

    private void clearChannelUUID(User user, TempMessage tempMessage) {
        if (m_depositMap.remove(user.getUserName()) != null) {

            String description = m_appContext.getMessage("did_not_leave_msg", null,
                    "disconnected without leaving a voice message.", user.getLocale());
            if (tempMessage != null) {

                if (tempMessage.isToBeStored() && tempMessage.getDuration() > 1) {
                    description = m_appContext.getMessage("just_left_msg", null, "just left a voice message.",
                            user.getLocale());
                }
            }
            String instantMsg = getChannelCallerIdName() + " (" + getChannelCallerIdNumber() + ") " + description;
            try {
                if (user.getVMExitIM()) {
                    HttpResult result = IMSender.sendVmExitIM(user, instantMsg, m_sendIMUrl);
                    if (!result.isSuccess()) {
                        LOG.error("Deposit::sendIM Trouble with RemoteRequest: "
                            + result.getResponse(), result.getException());
                    }
                }
            } catch (Exception ex) {
                LOG.error("Deposit::sendIM failed", ex);
            }
        }
    }

    /**
     * See if the caller wants to send this message to other mailboxes
     *
     * @param existingMessage the message they want to send
     */
    private void moreOptions(TempMessage message) {
        for (;;) {
            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            PromptList pl = getPromptList("deposit_more_options");
            VmMenu menu1 = createVmMenu();
            menu1.setSpeakCanceled(false);
            IvrChoice choice = menu1.collectDigit(pl, "1");

            if (!menu1.isOkay()) {
                return;
            }

            String digit = choice.getDigits();
            LOG.info("Mailbox " + getCurrentUser().getUserName() + " MoreOptions (" + digit + ")");

            // Do the copy dialog
            copyMessage(message);
        }
    }

    private boolean copyMessage(TempMessage message) {
        DialByNameChoice choice = createDialByNameDialog();
        if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
            return false;
        }

        // Store the message with each user in the list that has a mailbox
        for (User user : choice.getUsers()) {
            if (user.hasVoicemail()) {
                m_mailboxManager.copyMessage(user, message);
            }
        }
        // "Your message has been copied."
        play("deposit_copied", "");
        return true;
    }

    public void setSendImUrl(String url) {
        m_sendIMUrl = url;
    }

    public void setMailboxManager(MailboxManager manager) {
        m_mailboxManager = manager;
    }

    public void setDepositMap(Map depositMap) {
        m_depositMap = depositMap;
    }

    @Override
    public void setApplicationContext(ApplicationContext context) {
        m_appContext = context;
    }

    public void setOperatorAddr(String operatorAddr) {
        m_operatorAddr = operatorAddr;
    }
}
