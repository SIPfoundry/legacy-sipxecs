/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.userdb.PersonalAttendant;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.common.DialByNameChoice;
import org.sipfoundry.sipxivr.common.IvrChoice;
import org.sipfoundry.sipxivr.common.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.mailbox.Folder;
import org.sipfoundry.voicemail.mailbox.GreetingType;
import org.sipfoundry.voicemail.mailbox.MailboxDetails;
import org.sipfoundry.voicemail.mailbox.MessageDescriptor;
import org.sipfoundry.voicemail.mailbox.TempMessage;

public class Retrieve extends AbstractVmAction {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    // alarm SPX00044 for failed VM login attempts
    static final String FAILED_LOGIN_ALARM_ID = "VM_LOGIN_FAILED";
    static final int SKIPDURATION_SEC = 5;
    static final int DURATION_TO_END_SEC = 2;
    private List<String> m_tempRecordings;

    @Override
    public String runAction() {
        m_tempRecordings = new ArrayList<String>();
        // Create the mailbox if it isn't there
        User user = getCurrentUser();
        try {
            if (user.hasVoicemail()) {
                // Those with voicemail permissions get the whole menu
                playMainMenu();
            } else {
                // Those without (thus are just in the directory), get to record their name
                LOG.info("Retrieve::retrieveVoiceMail:recordName " + user.getUserName());
                recordName();
                goodbye();
            }
        } finally {
            for (String tempRecording : m_tempRecordings) {
                File temp = new File(tempRecording);
                if (temp.exists()) {
                    LOG.debug("Retrieve::retrieveVoiceMail " + user.getUserName()
                            + " deleting unused temporary recording " + temp.getPath());
                    FileUtils.deleteQuietly(temp);
                }
            }
        }

        return null;
    }

    /**
     * Collect the mailbox status prompts
     *
     * like "You have 2 unheard messages, 5 heard messages and 1 saved message"
     *
     * @return
     */
    PromptList status(MailboxDetails details) {
        PromptList pl = getPromptList();
        // You have...
        pl.addFragment("status_start");
        int inboxCount = details.getInboxCount();
        int savedCount = details.getSavedCount();
        if (inboxCount == 0 && savedCount == 0) {
            // ...no messages in your inbox
            pl.addFragment("status_none");
            return pl;
        }
        int unheardCount = details.getUnheardCount();
        int heardCount = details.getHeardCount();
        if (unheardCount > 0) {
            PromptList unheard;
            if (unheardCount == 1) {
                unheard = getPromptList("status_unheard_1");
            } else {
                unheard = getPromptList("status_unheard_many");
            }
            // {num unheard} message{s}
            pl.addFragment("status_unheard", Integer.toString(unheardCount), unheard.toString());
        }

        if (heardCount > 0) {
            if (unheardCount > 0) {
                // ...and...
                pl.addFragment("status_and");
            }
            PromptList heard;
            if (heardCount == 1) {
                heard = getPromptList("status_heard_1");
            } else {
                heard = getPromptList("status_heard_many");
            }
            // {num heard} message{s}
            pl.addFragment("status_heard", Integer.toString(heardCount), heard.toString());
        }

        if (savedCount > 0) {
            if (inboxCount > 0) {
                // ...and...
                pl.addFragment("status_and");
            }
            PromptList saved;
            if (savedCount == 1) {
                saved = getPromptList("status_saved_1");
            } else {
                saved = getPromptList("status_saved_many");
            }
            // {num saved} message{s}
            pl.addFragment("status_saved", Integer.toString(savedCount), saved.toString());
        }

        return pl;
    }

    private void playMainMenu() {
        boolean playStatus = true;
        String userName = getCurrentUser().getUserName();
        for (;;) {
            LOG.info("Retrieve::main_menu " + userName);
            VmMenu mainMenu = createVmMenu();
            mainMenu.setSpeakCanceled(false);

            MailboxDetails details = m_mailboxManager.getMailboxDetails(userName);
            if (playStatus) {
                // Include the status as pre-menu info
                PromptList statusPl = status(details);
                mainMenu.setPrePromptPl(statusPl);
                playStatus = false;
            }

            // Main menu.
            // To listen to your inbox messages, press 1.
            // To listen to your saved messages, press 2.
            // To listen to your deleted messages, press 3.
            // To send a message, press 4.
            // For voicemail options, press 5.
            // To logoff, press 8.
            // To reach the company operator, press 0"
            String validDigits = "123458";
            PromptList menuPl = getPromptList("main_menu");

            if (getCurrentUser().canRecordPrompts()) {
                // For system administration options, press 7.
                menuPl.addFragment("main_menu_options");
                validDigits += "7";
            }
            mainMenu.collectDigit(menuPl, validDigits);

            if (mainMenu.getChoice().getIvrChoiceReason().equals(IvrChoiceReason.CANCELED)) {
                // Canceled has no meaning at the top level.
                continue;
            }

            // Timeout, or errors
            if (!mainMenu.isOkay()) {
                return;
            }

            String digit = mainMenu.getChoice().getDigits();
            if (digit.equals("1")) {
                if (details.getInboxCount() == 0) {
                    // "You have no messages in your inbox."
                    play("no_inbox_messages", "");
                    continue;
                }
                playMessages(Folder.INBOX, details);
                continue;
            }

            if (digit.equals("2")) {
                if (details.getSavedCount() == 0) {
                    // "You have no saved messages."
                    play("no_saved_messages", "");
                    continue;
                }
                playMessages(Folder.SAVED, details);
                continue;
            }

            if (digit.equals("3")) {
                if (details.getDeletedCount() == 0) {
                    // "You have no deleted messages."
                    play("no_deleted_messages", "");
                    continue;
                }
                playMessages(Folder.DELETED, details);
                continue;
            }

            if (digit.equals("4")) {
                sendMessage();
                continue;
            }

            if (digit.equals("5")) {
                voicemailOptions();
                continue;
            }

            if (digit.equals("7")) {
                adminOptions();
                continue;
            }

            if (digit.equals("8")) {
                goodbye();
                continue;
            }
        }
    }

    void playMessages(Folder folder, MailboxDetails details) {
        List<String> messages = null;
        String menuFragment = null;
        String validDigits = null;
        int startPos = 0;
        int duration = 0;

        switch (folder) {
        case INBOX:
            messages = details.getInbox();
            break;
        case SAVED:
            messages = details.getSaved();
            break;
        case DELETED:
            messages = details.getDeleted();
            break;
        }
        for (String messageId : messages) {
            boolean playMessage = true;
            boolean playInfo = false;
            PromptList messagePl = null;
            PromptList prePromptPl = null;
            org.sipfoundry.voicemail.mailbox.VmMessage message = m_mailboxManager.getVmMessage(getCurrentUser()
                    .getUserName(), folder, messageId, true);

            try {
                for (;;) {
                    messagePl = getPromptList();
                    prePromptPl = getPromptList();
                    // {the message}
                    messagePl.addPrompts(message.getAudioFile().getPath());
                    // Determine if the message is from a known user
                    String from = ValidUsers.getUserPart(message.getDescriptor().getFromUri());
                    User user = getValidUser(from);
                    if (user != null) {
                        // If user doesn't have voicemail, don't allow reply
                        if (!user.hasVoicemail()) {
                            user = null;
                        }
                    }

                    switch (folder) {
                    case INBOX:
                        LOG.info("Retrieve::playMessages INBOX " + getCurrentUser().getUserName());

                        validDigits = "1234579#";
                        if (user != null) {
                            menuFragment = "msg_inbox_options_reply";
                            validDigits += "6";
                        } else {
                            menuFragment = "msg_inbox_options";
                        }
                        // "To play information about this message, press 1."
                        // "To replay press 2."
                        // "To save, press 3."
                        // "To delete press 4."
                        // "To forward to another inbox, press 5."
                        // {if from sipXuser} "To reply, press 6."
                        // "To play the next message, press #."
                        // "To return to the main menu, press *."
                        // "To fast forward message press 9."
                        // "To rewind message press 7."
                        break;
                    case SAVED:
                        LOG.info("Retrieve::playMessages SAVED " + getCurrentUser().getUserName());

                        validDigits = "124579#";
                        if (user != null) {
                            menuFragment = "msg_saved_options_reply";
                            validDigits += "6";
                        } else {
                            menuFragment = "msg_saved_options";
                        }
                        // "To play information about this message, press 1."
                        // "To replay press 2."
                        // "To delete press 4."
                        // "To forward to another inbox, press 5."
                        // {if from sipXuser} "To reply, press 6."
                        // "To play the next message, press #."
                        // "To return to the main menu, press *."
                        // "To fast forward message press 9."
                        // "To rewind message press 7."
                        break;
                    case DELETED:
                        LOG.info("Retrieve::playMessages DELETED " + getCurrentUser().getUserName());

                        validDigits = "1234579#";
                        if (user != null) {
                            menuFragment = "msg_deleted_options_reply";
                            validDigits += "6";
                        } else {
                            menuFragment = "msg_deleted_options";
                        }
                        // "To play information about this message, press 1."
                        // "To replay press 2."
                        // "To restore to inbox, press 3."
                        // "To delete permanently press 4."
                        // "To forward to another inbox, press 5."
                        // {if from sipXuser} "To reply, press 6."
                        // "To play the next message, press #."
                        // "To return to the main menu, press *."
                        // "To fast forward message press 9."
                        // "To rewind message press 7."
                        break;
                    }

                    // If we need to play the message, add it as a prePrompt to the menu.
                    // This is so we can barge it with a digit press and act on the digit in the menu.
                    if (playMessage) {
                        prePromptPl.addPrompts(messagePl);
                        prePromptPl.setOffset(startPos);
                        playMessage = false;
                    }
                    // Same with the message info.
                    if (playInfo) {
                        prePromptPl.addPrompts(messageInfo(message.getDescriptor()));
                        playInfo = false;
                    }

                    VmDialog vmd = createVmDialog(menuFragment);
                    vmd.setPrePromptList(prePromptPl);
                    vmd.setSpeakCanceled(false);

                    // Mark the message heard (if it wasn't before)
                    m_mailboxManager.markMessageHeard(getCurrentUser(), message);

                    long playStart = System.currentTimeMillis();
                    String digit = vmd.collectDigit(validDigits); // message starts playing here
                    duration = (int) ((System.currentTimeMillis() - playStart) / 1000);

                    if (digit == null) {
                        // Timeout, cancel, or errors
                        return;
                    }

                    if (digit.equals("1")) {
                        playInfo = true;
                        startPos = 0;
                        continue;
                    }
                    if (digit.equals("2")) {
                        playMessage = true;
                        startPos = 0;
                        continue;
                    }
                    if (digit.equals("3")) {
                        // If in inbox, moved to saved. If deleted move to inbox.
                        m_mailboxManager.saveMessage(getCurrentUser(), message);
                        if (folder == Folder.INBOX) {
                            play("msg_saved", "");
                        } else {
                            play("msg_restored", "");
                        }
                        startPos = 0;
                        break;
                    }
                    if (digit.equals("4")) {
                        // If in inbox or saved, moved to deleted. If in deleted, destroy
                        m_mailboxManager.deleteMessage(getCurrentUser(), message);
                        if (folder == Folder.DELETED) {
                            play("msg_destroyed", "");
                        } else {
                            play("msg_deleted", "");
                        }
                        startPos = 0;
                        break;
                    }
                    if (digit.equals("5")) {
                        forward(message);
                        startPos = 0;
                        continue;
                    }
                    if (digit.equals("6")) {
                        reply(message, user);
                        startPos = 0;
                        continue;
                    }
                    if (digit.equals("#")) {
                        startPos = 0;
                        break;
                    }
                    long msgDuration = message.getDescriptor().getDurationSecsLong();
                    // press '7' to rewind the message
                    if (digit.equals("7")) {
                        startPos = startPos + duration - SKIPDURATION_SEC;
                        if (startPos >= msgDuration) {
                            startPos = (int) msgDuration - DURATION_TO_END_SEC;
                        } else if (startPos < 0) {
                            startPos = 0;
                        }
                        if (prePromptPl != null) {
                            playMessage = true;
                        }
                        continue;
                    }

                    // press '9'to fast forward the message
                    if (digit.equals("9")) {
                        startPos = startPos + duration + SKIPDURATION_SEC;
                        if (startPos >= msgDuration) {
                            startPos = (int) msgDuration - DURATION_TO_END_SEC;
                        }
                        if (prePromptPl != null) {
                            playMessage = true;
                        }
                        continue;
                    }
                }
            } finally {
                message.cleanup();
            }
        }
        // "End of messages.
        play("end_of_messages", "");

    }

    /**
     * Play the "envelope information" about the message
     *
     * @param md
     */
    PromptList messageInfo(MessageDescriptor md) {
        DateFormat ttsDateFormat = TextToPrompts.ttsDateFormat();

        Calendar rxCal = Calendar.getInstance();
        rxCal.setTime(md.getTimeStampDate());
        Calendar nowCal = Calendar.getInstance();

        long daysInrxYear = rxCal.getMaximum(Calendar.DAY_OF_YEAR);
        long diffDays;
        long diffYears = nowCal.get(Calendar.YEAR) - rxCal.get(Calendar.YEAR);

        if (diffYears == 0) {
            // In the same year, so calculate
            // the difference in days between now and when the message was recorded
            diffDays = nowCal.get(Calendar.DAY_OF_YEAR) - rxCal.get(Calendar.DAY_OF_YEAR);
        } else {
            if (diffYears == 1) {
                // In different years, so offset by # days in the year
                diffDays = nowCal.get(Calendar.DAY_OF_YEAR) + daysInrxYear - rxCal.get(Calendar.DAY_OF_YEAR);
                // This is done so if playing a message recorded in the last week of December
                // while in the first week of January, we can still detect if the message is
                // less than a week old. Any older than that, and we'll add the year when speaking
                // the recording time.
            } else {
                diffDays = 42; // Long enough!
            }
        }

        // Build the appropriate date prompts based on how old the message is
        String rxCalString = ttsDateFormat.format(rxCal.getTime());
        String fragment = "msg_info_old";
        if (diffDays == 0) {
            // Same date
            fragment = "msg_info_today";
        } else if (diffDays == 1) {
            // Yesterday
            fragment = "msg_info_yesterday";
        } else if (diffDays < 7) {
            // within a week
            fragment = "msg_info_thisweek";
        } else if (diffYears == 0) {
            // In the same year
            fragment = "msg_info_thisyear";
        }
        LOG.debug(String.format("Retrieve::messageInfo (%s) (%s) (%s) %d %d days old", md.getTimestampString(),
                rxCalString, fragment, diffYears, diffDays));
        PromptList datePl = getPromptList(fragment, rxCalString);

        // Build the appropriate "from" based on what we know about the caller who left this
        // message
        String fromPrompts;
        String from = ValidUsers.getUserPart(md.getFromUri());
        User user = getValidUser(from);
        if (user != null) {
            // Well, looky here! A sipXecs user! Get his recorded name if we can
            File nameFile = m_mailboxManager.getRecordedName(user.getUserName());
            if (nameFile.exists()) {
                // "{name} at Extension {extension}"
                PromptList ext = getPromptList("atextension", nameFile.getPath(), user.getUserName());
                fromPrompts = ext.toString();
            } else {
                PromptList ext = getPromptList("extension", user.getUserName());
                // "Extension {extension}"
                fromPrompts = ext.toString();
            }
        } else {
            // Call from outside.

            if (from != null && from.matches("[0-9]{1,16}")) {
                // Up to 16 digits (no non-digits)
                PromptList digitsPl = getPromptList();
                digitsPl.addPrompts("{0, digits}", from);
                fromPrompts = digitsPl.toString();
            } else {
                // "An outside caller"
                PromptList outsidePl = getPromptList("msg_info_outsidecall");
                fromPrompts = outsidePl.toString();
            }
        }

        // "Message Received {date} {time} from {caller}"
        return getPromptList("msg_info", datePl.toString(), rxCalString, fromPrompts);

    }

    /**
     * Forward a message, optionally add a recorded comment
     *
     * @param vmMessage
     */
    void forward(org.sipfoundry.voicemail.mailbox.VmMessage vmMessage) {
        TempMessage comments = null;
        boolean askAboutComments = true;
        for (;;) {
            LOG.info("Retrieve::forward " + getCurrentUser().getUserName());

            if (askAboutComments) {
                // "To record comments, press 1."
                // "To forward this message without comments, press 2."
                // "To cancel, press *."
                VmDialog vmd = createVmDialog("msg_forward");
                String digit = vmd.collectDigit("12");
                if (digit == null) {
                    // Timeout, cancel, or errors
                    return;
                }

                LOG.debug("Retrieve::forward collected (" + digit + ")");

                if (digit.equals("1")) {
                    // "Record your comments, then press #"
                    //
                    // "To play your comments, press 1."
                    // "To accept your comments, press 2."
                    // "To delete these comments and try again, press 3."
                    // To cancel, press *"
                    comments = recordForwardComment("msg_record_comments", "msg_confirm_comments");

                    if (comments.getTempPath() == null) {
                        continue;
                    }
                } else if (digit.equals("2")) {
                } else {
                    continue;
                }
                askAboutComments = false;
            }

            // Get a list of extensions
            DialByNameChoice choice = createDialByNameDialog();
            if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
                return;
            }

            if (comments == null) {
                comments = m_mailboxManager.createTempMessage(getCurrentUser().getUserName(), getCurrentUser()
                        .getUri(), false);
            }
            // Forward the message to each destination that has a mailbox
            for (User destUser : choice.getUsers()) {
                if (destUser.hasVoicemail()) {
                    PersonalAttendant pa = destUser.getPersonalAttendant();
                    String localeString = pa.getLanguage();
                    if (localeString != null) {
                        destUser.setLocale(new Locale(localeString));
                    }
                    m_mailboxManager.forwardMessage(destUser, vmMessage, comments);
                }
            }

            // "Message forwarded."
            play("msg_forwarded", "");

            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            VmDialog vmd = createVmDialog("deposit_more_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                return;
            }

            // Back to enter another extension
        }
    }

    /**
     * Reply to a message, with a recorded comment back to the sender
     *
     * @param vmMessage
     */
    void reply(org.sipfoundry.voicemail.mailbox.VmMessage vmMessage, User sendingUser) {
        LOG.info("Retrieve::reply " + getCurrentUser().getUserName());

        // "Record your comments, then press #"
        //
        // "To play your comments, press 1."
        // "To accept your comments, press 2."
        // "To delete these comments and try again, press 3."
        // To cancel, press *"
        TempMessage commentsFile = recordDialog("msg_record_comments", "msg_confirm_comments");

        if (commentsFile == null) {
            return;
        }

        // Build a message with the comments sent by this user.
        m_mailboxManager.storeInInbox(sendingUser, commentsFile);

        // "Message sent."
        play("msg_replyed", "");
    }

    /**
     * Record a new message and send to a selected destination
     *
     * @param vmMessage
     */
    void sendMessage() {
        LOG.info("Retrieve::sendMessage " + getCurrentUser().getUserName());

        // "Record your message, then press #"
        //
        // "To play this message, press 1."
        // "To send this message, press 2."
        // "To delete this message and try again, press 3."
        // "To cancel, press *."
        TempMessage message = recordDialog("send_record_message", "send_confirm_record");

        if (message == null) {
            return;
        }

        for (;;) {
            // Get a list of extensions
            DialByNameChoice choice = createDialByNameDialog();
            if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
                return;
            }

            // Forward the message to each destination that has a mailbox
            for (User destUser : choice.getUsers()) {
                if (destUser.hasVoicemail()) {
                    // reset stored flag so we can save same message for other extensions
                    message.resetStoredFlag();
                    m_mailboxManager.storeInInbox(destUser, message);
                }
            }

            // "Message sent."
            play("msg_sent", "");

            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            VmDialog vmd = createVmDialog("send_more_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                return;
            }

            // Back to enter another extension
        }
    }

    private void recordName() {
        // "Record your name, then press #"
        //
        // "To listen to your recording, press 1."
        // "To use this recording, press 2."
        // "To delete this recording and try again, press 3."
        // "To cancel, press *."
        TempMessage recordingFile = recordDialog("record_name", "name_confirm");

        if (recordingFile == null) {
            return;
        }
        m_mailboxManager.saveRecordedName(recordingFile);

        // TODO ExtMailStore.SaveSpokenNameInFolder(getCurrentUser(), nameFile);

        // Name recorded.
        play("name_recorded", "");
    }

    void voicemailOptions() {
        User user = getCurrentUser();

        voicemailOptions: for (;;) {
            LOG.info("Retrieve::voicemailOptions " + user.getUserName());
            VmDialog vmd;
            String validDigits;

            if (user.canTuiChangePin()) {
                // "Voicemail Options."
                // "To record a personal greeting, press 1."
                // "To record your name, press 2."
                // "To select the greeting to play, press 3."
                // "To clear your deleted messages folder, press 4."
                // "To change your personal identification number, press 5."
                // "To return to the main menu, press *."
                vmd = createVmDialog("vm_options");
                validDigits = "12345";
            } else {
                // "Voicemail Options."
                // "To record a personal greeting, press 1."
                // "To record your name, press 2."
                // "To select the greeting to play, press 3."
                // "To clear your deleted messages folder, press 4."
                // "To return to the main menu, press *."
                vmd = createVmDialog("vm_options_nopin");
                validDigits = "1234";
            }
            vmd.setSpeakCanceled(false);
            String digit = vmd.collectDigit(validDigits);
            if (digit == null) {
                // bad entry, timeout, canceled
                return;
            }

            if (digit.equals("1")) {
                for (;;) {
                    // "To record a standard greeting, press 1."
                    // "To record an out-of-office greeting, press 2."
                    // "To record an extended absence greeting, press 3."
                    // "To cancel, press *."
                    vmd = createVmDialog("greeting_choice");
                    String digit1 = vmd.collectDigit("123");
                    if (digit1 == null) {
                        continue voicemailOptions;
                    }

                    LOG.info("Retrieve::voicemailOptions:recordGreeting " + user.getUserName());

                    // "Record your greeting, then press #"
                    //
                    // "To listen to this greeting, press 1."
                    // "To use this greeting, press 2."
                    // "To delete this greeting and try again, press 3."
                    // "To cancel, press *."
                    TempMessage recordingFile = recordDialog("record_greeting", "greeting_confirm");

                    if (recordingFile == null) {
                        continue;
                    }

                    // Save the recording as the appropriate greeting
                    GreetingType type = GreetingType.NONE;
                    if (digit1.equals("1")) {
                        type = GreetingType.STANDARD;
                    } else if (digit1.equals("2")) {
                        type = GreetingType.OUT_OF_OFFICE;
                    } else if (digit1.equals("3")) {
                        type = GreetingType.EXTENDED_ABSENCE;
                    }
                    m_mailboxManager.saveGreetingFile(type, recordingFile);

                    // Greeting recorded.
                    play("greeting_recorded", "");

                    continue voicemailOptions;
                }
            }

            if (digit.equals("2")) {
                LOG.info("Retrieve::voicemailOptions:recordName " + user.getUserName());
                recordName();
                continue voicemailOptions;
            }

            if (digit.equals("3")) {
                PromptList greetings = null;
                for (;;) {
                    LOG.info("Retrieve::voicemailOptions:selectGreeting " + user.getUserName());

                    // "Select the greeting to play to your callers."
                    // "To listen to your greetings before making a selection, press 1."
                    // "To select the standard greeting, press 2."
                    // "To select the out-of-office greeting, press 3."
                    // "To select the extended absence greeting, press 4."
                    // "To select the default system greeting, press 5."
                    // "To cancel, press *."
                    vmd = createVmDialog("select_greeting");
                    if (greetings != null) {
                        // {all the greetings}
                        vmd.setPrePromptList(greetings);
                        greetings = null;
                    }
                    String digit1 = vmd.collectDigit("12345");
                    if (digit1 == null) {
                        continue voicemailOptions;
                    }

                    if (digit1.equals("1")) {
                        Greeting greeting = createGreeting();
                        greetings = getPromptList();
                        // "Your standard greeting is...{prompts}"
                        // "Your out-of-office greeting is...{prompts}"
                        // "Your extended absence greeting is...{prompts}"
                        // "The default system greeting is...{prompts}"
                        // "Your active greeting is...{prompts}"
                        boolean playVmOption = user.shouldPlayDefaultVmOption();
                        greetings.addFragment(
                                "play_greetings",
                                greeting.getPromptList(greetings, GreetingType.STANDARD,
                                        m_mailboxManager.getGreetingPath(user, GreetingType.STANDARD), playVmOption).toString(),
                                greeting.getPromptList(greetings, GreetingType.OUT_OF_OFFICE,
                                        m_mailboxManager.getGreetingPath(user, GreetingType.OUT_OF_OFFICE), playVmOption)
                                        .toString(),
                                greeting.getPromptList(greetings, GreetingType.EXTENDED_ABSENCE,
                                        m_mailboxManager.getGreetingPath(user, GreetingType.EXTENDED_ABSENCE), playVmOption)
                                        .toString(),
                                greeting.getPromptList(greetings, GreetingType.NONE,
                                        m_mailboxManager.getGreetingPath(user, GreetingType.NONE), playVmOption).toString(),
                                greeting.getPromptList(greetings, getActiveGreeting(),
                                        m_mailboxManager.getGreetingPath(user, getActiveGreeting()), playVmOption).toString());
                        continue;
                    }

                    GreetingType type = GreetingType.NONE;
                    String greetingFrag = "default_selected";
                    if (digit1.equals("2")) {
                        type = GreetingType.STANDARD;
                        greetingFrag = "standard_selected";
                    } else if (digit1.equals("3")) {
                        type = GreetingType.OUT_OF_OFFICE;
                        greetingFrag = "outofoffice_selected";
                    } else if (digit1.equals("4")) {
                        type = GreetingType.EXTENDED_ABSENCE;
                        greetingFrag = "extended_selected";
                    }

                    // "You selected the {type} greeting."
                    PromptList pl = getPromptList(greetingFrag);

                    // "If this is correct, press 1."
                    // "To select a different greeting, press 2."
                    pl.addFragment("selected_greeting_confirm");
                    vmd = createVmDialog(null);
                    vmd.setPromptList(pl);
                    String digit2 = vmd.collectDigit("12");
                    if (digit2 == null) {
                        continue voicemailOptions;
                    }

                    if (digit2.equals("1")) {
                        m_mailboxManager.saveActiveGreeting(user, type);
                        // Active greeting set successfully.
                        play("selected_greeting_okay", "");
                        continue voicemailOptions;
                    }
                }
            }

            if (digit.equals("4")) {
                LOG.info("Retrieve::voicemailOptions:destroyDeletedMessages " + user.getUserName());

                m_mailboxManager.removeDeletedMessages(user.getUserName());
                // Your deleted messages folder is now empty.
                play("deleted_okay", "");
                continue voicemailOptions;
            }

            if (user.canTuiChangePin() && digit.equals("5")) {
                int errorCount = 0;
                for (;;) {
                    LOG.info("Retrieve::voicemailOptions:changePin " + user.getUserName());

                    if (errorCount > getInvalidResponseCount()) {
                        failure();
                        return;
                    }

                    // "Enter your personal identification number, and then press #."
                    // (Oh I was born an original pinner, I was born from original pin...)
                    PromptList pl1 = getPromptList("original_pin");
                    VmMenu menu1 = createVmMenu();
                    menu1.setOperatorOn0(false);
                    setRedactDTMF(true);
                    IvrChoice choice1 = menu1.collectDigits(pl1, 10);
                    setRedactDTMF(false);

                    if (!menu1.isOkay()) {
                        continue voicemailOptions;
                    }
                    String originalPin = choice1.getDigits();

                    if (!user.isVoicemailPinCorrect(originalPin)) {
                        errorCount++;
                        LOG.info("Retrieve::voicemailOptions:changePin " + getCurrentUser().getUserName()
                                + " Pin invalid.");
                        // "The personal identification number you have entered is not valid."
                        play("pin_invalid", "");
                        continue;
                    }

                    String newPin = "";

                    for (;;) {
                        // "Enter your new personal identification number, and then press #."
                        pl1 = getPromptList("new_pin");
                        menu1 = createVmMenu();
                        menu1.setOperatorOn0(false);
                        setRedactDTMF(true);
                        choice1 = menu1.collectDigits(pl1, 10);
                        setRedactDTMF(false);

                        if (!menu1.isOkay()) {
                            continue voicemailOptions;
                        }
                        newPin = choice1.getDigits();

                        // "Enter your new personal identification number again, and then press #."
                        pl1 = getPromptList("new_pin2");
                        menu1 = createVmMenu();
                        menu1.setOperatorOn0(false);
                        setRedactDTMF(true);
                        choice1 = menu1.collectDigits(pl1, 10);
                        setRedactDTMF(false);

                        if (!menu1.isOkay()) {
                            continue voicemailOptions;
                        }
                        String newPin2 = choice1.getDigits();

                        if (newPin.equals(newPin2)) {
                            break;
                        }
                        errorCount++;
                        LOG.info("Retrieve::voicemailOptions:changePin " + user.getUserName()
                                + " Pins do not match.");
                        // "The two personal identification numbers you have entered do not match."
                        play("pin_mismatch", "");
                    }
                    if (m_mailboxManager.changePin(user, newPin)) {
                        play("pin_changed", "");
                        continue voicemailOptions;
                    }
                    // "An error occurred while processing your request."
                    // "Your personal identification number is not changed."
                    play("pin_change_failed", "");
                    continue voicemailOptions;
                }
            }
        }
    }

    void adminOptions() {
        adminOptions: for (;;) {
            LOG.info("Retrieve::adminOptions " + getCurrentUser().getUserName());

            // "System Administration Options."
            // "To manage the auto attendant prompts, press 1."
            // "To return to the main menu, press *."
            VmDialog vmd = createVmDialog("sysadmin_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                // bad entry, timeout, canceled
                return;
            }

            // No need to check what they pressed, if it wasn't "1", then we aren't here!
            adminOptions2: for (;;) {
                // "To record the Auto Attendant prompt, press 1."
                // "To Manage the special Auto Attendant menu, press 2."
                // "To cancel, press *."
                vmd = createVmDialog("sysadmin_opts2");
                String digit = vmd.collectDigit("12");
                if (digit == null) {
                    // bad entry, timeout, canceled
                    continue adminOptions;
                }

                if (digit.equals("1")) {
                    LOG.info("Retrieve::adminOptions:recordAA " + getCurrentUser().getUserName());

                    // "Record the Auto Attendant prompt, then press #"
                    //
                    // "To listen to your recording, press 1."
                    // "To use this recording, press 2."
                    // "To delete this recording and try again, press 3."
                    // "To cancel, press *."
                    TempMessage recordingFile = recordDialog("record_aa", "aa_confirm");

                    if (recordingFile == null) {
                        continue adminOptions2;
                    }

                    m_mailboxManager.saveCustomAutoattendantPrompt(recordingFile);

                    // Auto Attendant prompt recorded.
                    play("aa_recorded", "");

                    continue adminOptions2;
                }

                if (digit.equals("2")) {
                    // "To enable the special autoattendant menu, press 1."
                    // "To disable it, press 2."
                    // "To cancel, press *."
                    vmd = createVmDialog("special_menu_options");
                    String digit1 = vmd.collectDigit("12");
                    if (digit1 == null) {
                        continue adminOptions2;
                    }
                    // Tell sipXconfig about the change.

                    if (digit1.equals("1")) {
                        if (m_mailboxManager.manageSpecialMode(getCurrentUser(), true)) {
                            LOG.info("Retrieve::adminOptions:specialmode " + getCurrentUser().getUserName()
                                    + " specialmode enabled.");
                            // "Special Auto Attendant menu is enabled."
                            play("special_menu_enabled", "");
                            continue adminOptions2;
                        }
                    } else if (digit1.equals("2")) {
                        if (m_mailboxManager.manageSpecialMode(getCurrentUser(), false)) {
                            LOG.info("Retrieve::adminOptions:specialmode " + getCurrentUser().getUserName()
                                    + " specialmode disabled.");
                            // "Special Auto Attendant menu is disabled."
                            play("special_menu_disabled", "");
                            continue adminOptions2;
                        }
                    }
                    // "An error occurred while processing your request."
                    play("special_menu_failed", "");
                }
            }
        }
    }

    TempMessage recordForwardComment(String recordFragment, String confirmMenuFragment) {
        TempMessage message = recordDialog(recordFragment, confirmMenuFragment);
        if (message == null) {
            return m_mailboxManager.createTempMessage(getCurrentUser().getUserName(), getCurrentUser().getUri(),
                    false);
        }
        return message;
    }

    /**
     * Record a wav / mp3 file with confirmation dialog.
     *
     * @param recordFragment To play before the recording
     * @param confirmMenuFragment To play after the recording
     * @return the temporary wav / mp3 file. null if recording is to be tossed
     */
    TempMessage recordDialog(String recordFragment, String confirmMenuFragment) {
        TempMessage tempMessage = m_mailboxManager.createTempMessage(getCurrentUser().getUserName(),
                getCurrentUser().getUri(), true);
        String audioPath = tempMessage.getTempPath();
        m_tempRecordings.add(audioPath);

        boolean recordAudio = true;
        boolean playAudio = false;
        for (;;) {

            if (recordAudio) {
                // Record your {thingy} then press #
                play(recordFragment, "*");
                // Give the user 1 second to press "*" to cancel, then start recording
                String digit = collectDigits(1, 1000, 0, 0, "*");
                LOG.debug("Retrieve::recordDialog collected (" + digit + ")");
                if (digit.length() == 0 || !"*0".contains(digit)) {
                    recordMessage(audioPath);
                    digit = getDtmfDigit();
                    if (digit == null) {
                        digit = "";
                    }
                    LOG.debug("Retrieve::recordDialog record terminated collected (" + digit + ")");
                }

                if (digit.equals("0")) {
                    transferToOperator();
                    return null;
                }

                if (digit.equals("*")) {
                    // "Canceled."
                    play("canceled", "");
                    return null;
                }
                recordAudio = false;
            }

            // Confirm caller's intent for this {thingy}

            // (pre-menu: {recording})

            // "To play your {thingy}, press 1."
            // "To accept your {thingy}, press 2."
            // "To delete this {thingy} and try again, press 3."
            // "To cancel, press *"
            VmDialog vmd = createVmDialog(confirmMenuFragment);
            if (playAudio) {
                PromptList messagePl = getPromptList();
                messagePl.addPrompts(audioPath);
                vmd.setPrePromptList(messagePl);
            }

            String digit = vmd.collectDigit("123");

            // bad entry, timeout, canceled
            if (digit == null) {
                return null;
            }

            LOG.info("Retrieve::recordDialog " + getCurrentUser().getUserName() + " options (" + digit + ")");

            // "1" means play the recording
            if (digit.equals("1")) {
                LOG.info(String.format("Retrieve::recordDialog " + getCurrentUser().getUserName()
                        + " Playing back recording (%s)", audioPath));
                playAudio = true;
                continue;
            }

            // "2" means accept the recording
            if (digit.equals("2")) {
                LOG.info(String.format("Retrieve::recordDialog " + getCurrentUser().getUserName()
                        + " accepted recording (%s)", audioPath));
                return tempMessage;
            }

            // "3" means "erase" and re-record
            if (digit.equals("3")) {
                recordAudio = true;
                continue;
            }
        }
    }

}
