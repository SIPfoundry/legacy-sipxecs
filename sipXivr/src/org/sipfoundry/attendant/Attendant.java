/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.attendant;

import java.util.Calendar;
import java.util.Date;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.SipxIvrApp;
import org.sipfoundry.sipxivr.common.DialByName;
import org.sipfoundry.sipxivr.common.DialByNameChoice;
import org.sipfoundry.sipxivr.common.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.mailbox.MailboxManager;

public class Attendant extends SipxIvrApp {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private ValidUsers m_validUsers;
    private String m_operatorAddr;
    private MailboxManager m_mailboxManager;

    enum NextAction {
        repeat, exit, nextAttendant;
    }

    /**
     * Run each Attendant until there is nothing left to do. If the SIP URL didn't pass in a
     * particular attendant name, use the current time of day and the schedule to find which
     * attendant to run.
     * 
     * Keep running the next returned attendant until there are none left, then exit.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    @Override
    public void run() {
        AaEslRequestController controller = (AaEslRequestController) getEslRequestController();
        String aaId = controller.getAttendantId();
        String scheduleId = controller.getScheduleId();
        Schedule schedule = controller.getSchedule();
        String id = controller.getSpecialAttendantId();
        if (aaId == null) {
            // See if a special attendant is defined
            if (id != null) {
                // Always use the special AA if specialOperation is in effect
                LOG.info("Attendant::run Special Operation AutoAttendant is in effect.");
            } else {
                Date now = Calendar.getInstance().getTime();
                if (schedule != null) {
                    LOG.info(String.format("Attendant::run Attendant determined from schedule %s", schedule.getId()));
                    id = schedule.getAttendant(now);
                } else {
                    LOG.error(String.format("Attendant::run Cannot find schedule %s in autoattendants.xml.",
                            scheduleId != null ? scheduleId : "null"));
                }
                if (id == null) {
                    LOG.error("Attendant::run Cannot determine which attendant to use from schedule.");
                } else {
                    LOG.info(String.format("Attendant::run Attendant %s selected", id));
                }
            }
        } else {
            id = aaId;
            LOG.info(String.format("Attendant::run Attendant %s determined from URL parameter", id));
        }

        // Wait it bit so audio doesn't start too fast
        controller.sleep(1000);

        while (id != null) {
            // Keep running attendants until there are no more to run
            id = attendant(id, controller);
        }
    }

    /**
     * Do the specified Attendant.
     * 
     * @param id The id of the attendant.
     * @return The id of the next attendant, or null if there is no next.
     */
    String attendant(String id, AaEslRequestController controller) {
        String nextAttendant = null;

        // Find the configuration for the named attendant
        AttendantConfig config = controller.getAttendantConfig(id);

        if (config == null) {
            LOG.error(String.format("Attendant::attendant Unable to determine which configuration to use from (%s)",
                    id));
            return null;
        }

        LOG.info("Attendant::attendant Starting attendant id " + id + " (" + config.getName() + ") in locale "
                + controller.getLocale());

        String digits;
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            // Check for a failure condition
            if (invalidCount > config.getInvalidResponseCount() || timeoutCount > config.getNoInputCount()) {
                failure(config, controller);
                break;
            }

            // Play the initial prompt, or main menu.
            PromptList pl = null;
            if (config.getPrompt() != null && !config.getPrompt().contentEquals("")) {
                // Override default main menu prompts with user recorded one
                pl = controller.getPromptList();
                pl.addPrompts(config.getPrompt());
            } else {
                // Use default menu
                pl = controller.getPromptList("main_menu");
            }
            controller.play(pl, "0123456789#*");

            // Wait for the caller to enter a selection.
            digits = controller.collectDigits(config, "*#");
            LOG.info("Attendant::attendant Collected digits=" + digits);

            // See if it timed out (no digits)
            if (digits.equals("")) {
                timeoutCount++;
                continue;
            }

            AttendantMenuItem item = null;
            // Check if entered digits match any actions
            for (AttendantMenuItem menuItem : config.getMenuItems()) {
                if (menuItem.getDialpad().contentEquals(digits)) {
                    item = menuItem;
                    break;
                }
            }

            if (item != null) {
                // Do the action corresponding to that digit
                NextAction next = doAction(item, controller, config);
                if (next.equals(NextAction.repeat)) {
                    timeoutCount = 0;
                    invalidCount = 0;
                    continue;
                } else if (next.equals(NextAction.nextAttendant)) {
                    nextAttendant = item.getParameter();
                    break;
                } else if (next.equals(NextAction.exit)) {
                    break;
                }
                continue;
            }

            // None of the above...must be an extension.
            // For a nicer implementation, uncomment this
            // if (digits.length() >= 2)
            // but to match the original VoiceXML...
            // See if the entered digits matches a dialable extension
            // (keeps AA users from entering long distance numbers, 900 numbers,
            // call pickup, paging, etc.)
            User user = m_validUsers.getUser(digits);
            if (user != null) {
                String uri = user.getUri();
                LOG.info(String.format("Attendant::attendant Transfer to extension %s (%s) uuid=%s", digits, uri,
                        controller.getUuid()));
                // It's valid, transfer the call there.
                controller.transfer(uri, config.isPlayPrompt());
                break;
            }

            LOG.info("Attendant::attendant Extension " + digits + " is not valid");
            // "That extension is not valid."
            controller.play("invalid_extension", "");
            invalidCount++;
            continue;
        }

        LOG.info("Attendant::attendant Ending attendant " + config.getName());
        return nextAttendant;
    }

    /**
     * Do the action from the corresponding menu item.
     * 
     * @param item
     * @return The next action to perform
     */
    NextAction doAction(AttendantMenuItem item, AaEslRequestController controller, AttendantConfig config) {
        String dest = null;

        switch (item.getAction()) {
        case repeat_prompt:
            LOG.info("Attendant::doAction Repeat Prompt");
            return NextAction.repeat;

        case voicemail_access:
            // Transfer to the voicemailUrl
            LOG.info("Attendant::doAction Voicemail Access.  Transfer to " + dest);
            controller.transfer(item.getParameter(), false);
            return NextAction.exit;

        case voicemail_deposit: {
            // Transfer to the specific extension's VM.

            // Lookup the extension (it may be an alias)
            String extension = item.getExtension();
            User u = m_validUsers.getUser(extension);
            if (u != null) {
                // Use the internal ~~vm~xxxx user to do this.
                dest = controller.extensionToUrl("~~vm~" + u.getUserName());
                LOG.info("Attendant::doAction Voicemail Deposit.  Transfer to " + dest);
                controller.transfer(dest, false);
            } else {
                LOG.error("Attendant::doAction Voicemail Deposit cannot find user for extension " + extension);
            }
            return NextAction.exit;
        }
        case dial_by_name:
            // Enter the Dial By Name dialog.
            LOG.info("Attendant::doAction Dial by Name for groups: " + item.getParameter());
            DialByName dbn = new DialByName();
            dbn.setApplicationConfiguration(config);
            dbn.setLocalization(controller.getLocalization());
            dbn.setValidUsers(m_validUsers);
            dbn.setMailboxManager(m_mailboxManager);
            DialByNameChoice choice = dbn.dialByName(item.getParameter());
            if (choice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                return NextAction.repeat;
            }
            Vector<User> u = choice.getUsers();
            if (u == null) {
                controller.goodbye();
                return NextAction.exit;
            }
            LOG.info(String.format("Attendant::doAction Transfer to extension %s (%s) uuid=%s", u.get(0)
                    .getUserName(), u.get(0).getUri(), controller.getUuid()));
            controller.transfer(u.get(0).getUri(), config.isPlayPrompt());
            return NextAction.exit;

        case disconnect:
            LOG.info("Attendant::doAction Disconnect");
            controller.goodbye();
            return NextAction.exit;

        case operator:
            // Transfer to the operator's address
            LOG.info("Attendant::doAction Operator.  Transfer to " + m_operatorAddr);
            controller.transfer(m_operatorAddr, config.isPlayPrompt());
            return NextAction.exit;

        case transfer_out:
            // Transfer to the specified extension
            String extensionOrOther = item.getExtension();
            if (extensionOrOther.contains("@")) {
                // If it's got an @, assume its a SIP url (or close enough)
                dest = extensionOrOther;
            } else {
                // Assume it's an extension, and tack on "sip:" and @ourdomain
                dest = controller.extensionToUrl(extensionOrOther);
            }
            LOG.info("Attendant::doAction Transfer Out.  Transfer to " + dest);
            controller.transfer(dest, config.isPlayPrompt());
            return NextAction.exit;

        case transfer_to_another_aa_menu:
            // "Transfer" to another attendant. Not really a call transfer
            // as it stays in this thread. See run().
            LOG.info("Attendant::doAction Transfer to attendant " + item.getParameter());
            return NextAction.nextAttendant;

        default:
        }
        return NextAction.exit;
    }

    /**
     * Perform the configured "failure" behavior, which can be either just hangup or transfer to a
     * destination after playing a prompt.
     * 
     */
    void failure(AttendantConfig config, AaEslRequestController controller) {
        LOG.info("Attendant::failure");
        boolean playGoodbye = true;
        if (config.isTransferOnFailure()) {
            String transferPrompt = config.getTransferPrompt();
            if (transferPrompt != null) {
                PromptList pl = controller.getPromptList();
                pl.addPrompts(config.getTransferPrompt());
                controller.play(pl, "");
            }

            String dest = config.getTransferURL();
            if (!dest.toLowerCase().contains("sip:")) {
                LOG.error("Attendant::failure transferUrl should be a sip: URL.  Assuming extension");
                dest = controller.extensionToUrl(dest);
            }
            LOG.info("Attendant::failure Transfer on falure to " + dest);

            controller.play("please_hold", "");
            playGoodbye = false;
            String domainPart = ValidUsers.getDomainPart(dest);
            if (domainPart.equalsIgnoreCase(controller.getSipxchangeDomainName())) {
                String userpart = ValidUsers.getUserPart(dest);
                User user = m_validUsers.getUser(userpart);
                if (user != null) {
                    String uri = user.getUri();
                    LOG.info(String.format("Attendant::attendant Transfer to extension %s (%s) uuid=%s", dest, uri,
                            controller.getUuid()));
                    // It's valid, transfer the call there.
                    controller.transfer(dest, false);
                } else {
                    LOG.info("Attendant::attendant Extension " + dest + " is not valid");
                    // "That extension is not valid."
                    controller.play("invalid_extension", "");
                }
            } else {
                controller.transfer(dest, false);
            }
        }

        // This is an enhancement over the original VoiceXML
        controller.goodbye(playGoodbye);
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    public void setOperatorAddr(String operatorAddr) {
        m_operatorAddr = operatorAddr;
    }

    public void setMailboxManager(MailboxManager mgr) {
        m_mailboxManager = mgr;
    }

}
