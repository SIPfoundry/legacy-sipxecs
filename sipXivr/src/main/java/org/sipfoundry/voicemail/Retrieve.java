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
import java.io.IOException;
import java.text.DateFormat;
import java.util.Calendar;
import java.util.List;
import java.util.Vector;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.GreetingType;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.RestfulRequest;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;
import org.sipfoundry.voicemail.Messages.Folders;

public class Retrieve {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    static final long MSPERDAY = 1000*60*60*24; // number of mS per day

    VoiceMail m_vm;
    Localization m_loc;
    FreeSwitchEventSocketInterface m_fses;
    Mailbox m_mailbox;
    Messages m_messages;
    Vector<File> m_tempRecordings;  // Temp recordings that need to be destroyed on hangup
    String m_userEnteredPin;        // PIN the user entered at login
    
    String m_ident; // Used to identify a particular call in the logs
    static final int SKIPDURATION = 5; // in seconds 
    static final int DURATION_TO_END = 2; // in seconds -  the duration to the end of the message
    
    public Retrieve(VoiceMail vm) {
        m_vm = vm;
        m_loc = vm.getLoc();
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
        m_mailbox = m_vm.getMailbox();
        m_tempRecordings = new Vector<File>();
    }
    
    public String retrieveVoiceMail() {
        if (m_mailbox != null) { 
            m_ident = "Mailbox "+m_mailbox.getUser().getUserName();
        } else {
            m_ident = "Mailbox (unknown)";
        }
        String displayUri = m_fses.getDisplayUri();
        LOG.info("Retrieve::retrieveVoiceMail "+m_ident+" Retrieve Voicemail from "+displayUri);

        m_fses.setRedactDTMF(true);
        m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
        User user = login() ;
        m_fses.setRedactDTMF(false);
        if (user != null) {
            m_mailbox = new Mailbox(user);
            m_vm.setMailbox(m_mailbox);
            m_ident = "Mailbox "+m_mailbox.getUser().getUserName();
            LOG.info("Retrieve::retrieveVoiceMail "+m_ident+" logged in");
            // Create the mailbox if it isn't there
            Mailbox.createDirsIfNeeded(m_mailbox);
            m_messages = Messages.newMessages(m_mailbox);
            try {
                if (user.hasVoicemail()) {
                    // Those with voicemail permissions get the whole menu
                    main_menu();
                } else {
                    // Those without (thus are just in the directory), get to record their name
                    LOG.info("Retrieve::retrieveVoiceMail:recordName "+m_ident);
                    recordName();
                    m_vm.goodbye();
                }
            } finally {
                for (File tempRecording : m_tempRecordings) {
                    if (tempRecording.exists()) {
                        LOG.debug("Retrieve::retrieveVoiceMail "+m_ident+" deleting unused temporary recording "+tempRecording.getPath());
                        FileUtils.deleteQuietly(tempRecording);
                    }
                }
                Messages.releaseMessages(m_messages);
            }
        }
        
        return null ;
    }
    
    /**
     * Login to the mailbox.
     * 
     * @return
     */
    private User login() {
        int errorCount = 0;
        User user = null;
        boolean newExtension = false;

        // Welcome.  Your call has been answered by an automated communications system.
        PromptList welcomePl = m_loc.getPromptList("welcome");
        boolean playWelcome = true;

        if (m_mailbox == null) {
            newExtension = true;
        } else {
            user = m_mailbox.getUser();
        }
        
        for(;;) {
            if (errorCount > m_vm.getConfig().getInvalidResponseCount()) {
                m_vm.failure();
                return null;
            }

            if (newExtension) {
                newExtension = false;
                
                // "Enter your extension."
                PromptList extPl = m_loc.getPromptList("enter_extension");
                VmMenu extMenu = new VmMenu(m_vm);
                if (playWelcome) {
                    extMenu.setPrePromptPl(welcomePl);
                    playWelcome = false;
                }

                IvrChoice extChoice = extMenu.collectDigits(extPl, 10);
                if (extMenu.isCanceled()) {
                    continue;
                }
                if (!extMenu.isOkay()) {
                    return null ;
                }
                LOG.info("Retrieve::login "+m_ident+" changing to extension "+extChoice.getDigits());

                // See if the user exists
                user = m_vm.getValidUsers().getUser(extChoice.getDigits());
            }

            // "Enter your personal identification number, and then press #.":
            // "To log in as a different user, press #"
            PromptList menuPl = m_loc.getPromptList("enter_pin");
            VmMenu menu = new VmMenu(m_vm);
            if (playWelcome) {
                menu.setPrePromptPl(welcomePl);
                playWelcome = false;
            }
            
            // Note:  Not using collectDigits() here, as it doesn't allow initial "#" to barge,
            // and "*" to cancel doesn't really make sense.  Just treat as invalid.
            IvrChoice choice = menu.collectDtmf(menuPl, 10);
            
            if (!menu.isOkay()) {
                return null;
            }
            
            if (choice.getDigits().equals("#")) {
                newExtension = true;
                continue;
            }
            
            // Only users with voicemail, or are in the directory, are allowed
            // (directory users can record their name, that's all)
            if (user != null && !user.hasVoicemail() && !user.isInDirectory() ) {
                LOG.info("Retrieve::login user "+user.getUserName()+" doesn't have needed permissions.");
                user = null ;
            }
            
            if (user == null || !user.isPinCorrect(choice.getDigits(), m_loc.getConfig().getRealm())) {
                // WRONG, do it again!
                
                // "That personal identification number is not valid
                m_loc.play("invalid_pin", "");
                ++errorCount;
                continue;
            }
            m_userEnteredPin = choice.getDigits();
            break ;
        }
        return user ;
    }
    
    /**
     * Collect the mailbox status prompts
     * 
     *  like "You have 2 unheard messages, 5 heard messages and 1 saved message"
     *
     * @return
     */
    PromptList status() {
        PromptList pl = m_loc.getPromptList();
        // You have...
        pl.addFragment("status_start");
        if (m_messages.getInboxCount() == 0 && m_messages.getSavedCount() == 0) {
            // ...no messages in your inbox
            pl.addFragment("status_none");
            return pl;
        }
        
        
        int unheardCount;
        int heardCount;
        int savedCount;
        synchronized (m_messages) {
            unheardCount = m_messages.getUnheardCount();
            heardCount = m_messages.getHeardCount();
            savedCount = m_messages.getSavedCount();
        }
        if (unheardCount > 0) {
            PromptList unheard;
            if (unheardCount == 1) {
                unheard = m_loc.getPromptList("status_unheard_1");
            } else {
                unheard = m_loc.getPromptList("status_unheard_many");
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
                heard = m_loc.getPromptList("status_heard_1");
            } else {
                heard = m_loc.getPromptList("status_heard_many");
            }
            // {num heard} message{s}
            pl.addFragment("status_heard", Integer.toString(heardCount), heard.toString());
        }
        
        if (savedCount > 0) {
            if (m_messages.getInboxCount() > 0) {
                // ...and...
                pl.addFragment("status_and");
            }
            PromptList saved;
            if (savedCount == 1) {
                saved = m_loc.getPromptList("status_saved_1");
            } else {
                saved = m_loc.getPromptList("status_saved_many");
            }
            // {num saved} message{s}
            pl.addFragment("status_saved", Integer.toString(savedCount), saved.toString());
        }
        
        return pl;
    }

    File makeTempWavFile() {
        File wavFile = null;
        try {
            // Create in the deleted directory, so if somehow things fail, they'll get removed
            // as they age out.
            wavFile = File.createTempFile("temp_recording_", ".wav", 
                    new File(m_mailbox.getDeletedDirectory()));
            m_tempRecordings.add(wavFile);
        } catch (IOException e) {
            throw new RuntimeException("Cannot create temp recording file", e);
        }
        return wavFile;
    }

    void dontDeleteTempFile(File tempFile) {
        if (tempFile != null) {
            m_tempRecordings.remove(tempFile);
        }
    }
    
    void main_menu() {
        boolean playStatus = true ;
        for(;;) {
            LOG.info("Retrieve::main_menu "+m_ident);
            VmMenu mainMenu = new VmMenu(m_vm);
            mainMenu.setSpeakCanceled(false);

            if (playStatus) {
                // Include the status as pre-menu info
                PromptList statusPl = status();
                mainMenu.setPrePromptPl(statusPl);
                playStatus = false;
            } else {
                // Update the messages each time through (but not on the first one)
                m_messages.update();
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
            PromptList menuPl = m_loc.getPromptList("main_menu");

            if (m_mailbox.getUser().canRecordPrompts()) {
                // For system administration options, press 7.
                menuPl.addFragment("main_menu_options");
                validDigits += "7";
            }
            mainMenu.collectDigit(menuPl, validDigits);
            
            if (mainMenu.getChoice().getIvrChoiceReason().equals(IvrChoiceReason.CANCELED)) {
                // Canceled has no meaning at the top level.
                continue ;
            }
            
            // Timeout, or errors
            if (!mainMenu.isOkay()) {
                return;
            }
            
            String digit = mainMenu.getChoice().getDigits();
            if (digit.equals("1")) {
                if (m_messages.getInboxCount() == 0) {
                    // "You have no messages in your inbox."
                    m_loc.play("no_inbox_messages", "");
                    continue ;
                }
                playMessages(Messages.Folders.INBOX);
                continue;
            }

            if (digit.equals("2")) {
                if (m_messages.getSavedCount() == 0) {
                    // "You have no saved messages."
                    m_loc.play("no_saved_messages", "");
                    continue ;
                }
                playMessages(Messages.Folders.SAVED);
                continue;
            }

            if (digit.equals("3")) {
                if (m_messages.getDeletedCount() == 0) {
                    // "You have no deleted messages."
                    m_loc.play("no_deleted_messages", "");
                    continue ;
                }
                playMessages(Messages.Folders.DELETED);
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
                m_vm.goodbye();
                continue;
            }
        }
    }

    void playMessages(Messages.Folders folder) {
        List<VmMessage> folderList = null;
        String menuFragment = null;
        String validDigits = null;
        int startPos = 0;
        int duration = 0;
        
        switch (folder) {
            case INBOX: folderList = m_messages.getInbox(); break;
            case SAVED: folderList = m_messages.getSaved(); break;
            case DELETED: folderList = m_messages.getDeleted(); break;
        }
        for (VmMessage vmMessage : folderList) {
            boolean playMessage = true;
            boolean playInfo = false;
            PromptList messagePl= null;
            PromptList prePromptPl= null;
            for(;;) {
                messagePl = m_loc.getPromptList();
                prePromptPl = m_loc.getPromptList();
                m_messages.LoadWaveFile(vmMessage);
                // {the message}
                messagePl.addPrompts(vmMessage.getAudioFile().getPath());
                // Read the message descriptor file to obtain the info we need
                MessageDescriptor md = new MessageDescriptorReader().readObject(vmMessage.getDescriptorFile());
                // Determine if the message is from a known user
                String from = ValidUsersXML.getUserPart(md.getFromUri());                
                User user = m_vm.getValidUsers().getUser(from);
                if (user != null) {
                    // If user doesn't have voicemail, don't allow reply
                    if (!user.hasVoicemail()) {
                        user = null;
                    }
                }
                
                switch (folder) {
                case INBOX: 
                    LOG.info("Retrieve::playMessages INBOX "+m_ident);

                    validDigits="1234579#";
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
                    LOG.info("Retrieve::playMessages SAVED "+m_ident);

                    validDigits="124579#";
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
                    LOG.info("Retrieve::playMessages DELETED "+m_ident);

                    validDigits="1234579#";
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
                    prePromptPl.addPrompts(messageInfo(md));
                    playInfo = false;
                }

                VmDialog vmd = new VmDialog(m_vm, menuFragment);
                vmd.setPrePromptList(prePromptPl);
                vmd.setSpeakCanceled(false);
                long playStart = System.currentTimeMillis();
                String digit = vmd.collectDigit(validDigits); //message starts playing here
                duration = (int) ((System.currentTimeMillis() - playStart)/1000);

                if (digit == null) {
                    // Timeout, cancel, or errors
                    return;
                }
                
                // Mark the message heard (if it wasn't before)
                m_messages.markMessageHeard(vmMessage, true);
                
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
                    // If in inbox, moved to saved.  If deleted move to inbox.
                    m_messages.saveMessage(vmMessage);
                    if (folder == Folders.INBOX) {
                        m_loc.play("msg_saved", "");
                    } else {
                        m_loc.play("msg_restored", "");
                    }
                    startPos = 0;
                    break;
                }
                if (digit.equals("4")) {
                    // If in inbox or saved, moved to deleted.  If in deleted, destroy
                    m_messages.deleteMessage(vmMessage);
                    if (folder == Folders.DELETED) {
                        m_loc.play("msg_destroyed", "");
                    } else {
                        m_loc.play("msg_deleted", "");
                    }
                    startPos = 0;
                    break;
                }
                if (digit.equals("5")) {
                    forward(vmMessage);
                    startPos = 0;
                    continue;
                }
                if (digit.equals("6")) {
                    reply(vmMessage, user);
                    startPos = 0;
                    continue;
                }
                if (digit.equals("#")) {
                    startPos = 0;
                    break;
                }
                // press '7' to rewind the message
                if (digit.equals("7")) {
                    startPos = startPos + duration - SKIPDURATION;
                    if(startPos >= vmMessage.getDuration()) {
                        startPos = (int) vmMessage.getDuration() - DURATION_TO_END;
                    }
                    else if(startPos < 0) {
                        startPos = 0;
                    }
                    if(prePromptPl != null) {
                        playMessage = true;
                    }
                    continue;
                }
                
                //press '9'to fast forward the message
                if (digit.equals("9")) {
                    startPos = startPos + duration + SKIPDURATION;
                    if(startPos >= vmMessage.getDuration()) {
                        startPos = (int) vmMessage.getDuration() - DURATION_TO_END;
                    }
                    if(prePromptPl != null) {
                        playMessage = true;
                    }
                    continue;
                }
            }
        }
        // "End of messages.
        m_loc.play("end_of_messages", "");
        
    }

    /**
     * Play the "envelope information" about the message
     * @param md
     */
    PromptList messageInfo(MessageDescriptor md) {
        DateFormat ttsDateFormat = TextToPrompts.ttsDateFormat();

        Calendar rxCal = Calendar.getInstance();
        rxCal.setTime(md.getTimeStampDate());
        Calendar nowCal =  Calendar.getInstance();

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
                diffDays = nowCal.get(Calendar.DAY_OF_YEAR) + daysInrxYear -
                    rxCal.get(Calendar.DAY_OF_YEAR);
                // This is done so if playing a message recorded in the last week of December
                // while in the first week of January, we can still detect if the message is
                // less than a week old.  Any older than that, and we'll add the year when speaking
                // the recording time.
            }
            else {
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
        LOG.debug(String.format("Retrieve::messageInfo (%s) (%s) (%s) %d %d days old", md.getTimestampString(), rxCalString,
                fragment, diffYears, diffDays));
        PromptList datePl = m_loc.getPromptList(fragment, rxCalString);
        
        // Build the appropriate "from" based on what we know about the caller who left this message
        String fromPrompts;
        String from = ValidUsersXML.getUserPart(md.getFromUri());
        User user = m_vm.getValidUsers().getUser(from);
        if (user != null) {
            // Well, looky here!  A sipXecs user!  Get his recorded name if we can
            Mailbox userMbox = new Mailbox(user);
            File nameFile = userMbox.getRecordedNameFile();
            if (nameFile.exists()) {
                // "{name} at Extension {extension}"
                PromptList ext = m_loc.getPromptList("atextension", nameFile.getPath(), user.getUserName());
                fromPrompts = ext.toString();
            } else { 
                PromptList ext = m_loc.getPromptList("extension", user.getUserName());
                // "Extension {extension}"
                fromPrompts = ext.toString();
            }
        } else {
            // Call from outside.  
            
            if (from != null && from.matches("[0-9]{1,16}")) {
                // Up to 16 digits (no non-digits)
                PromptList digitsPl = m_loc.getPromptList();
                digitsPl.addPrompts("{0, digits}", from);
                fromPrompts = digitsPl.toString();
            } else {
                // "An outside caller"
                PromptList outsidePl = m_loc.getPromptList("msg_info_outsidecall");
                fromPrompts = outsidePl.toString();
            }
        }

        // "Message Received {date} {time} from {caller}"
        return m_loc.getPromptList("msg_info", datePl.toString(), rxCalString, fromPrompts);
        
    }
    
    /** 
     * Forward a message, optionally add a recorded comment
     * @param vmMessage
     */
    void forward(VmMessage vmMessage) {
        File commentsFile = null;
        Message comments = null;
        boolean askAboutComments = true;
        for(;;) {
            LOG.info("Retrieve::forward "+m_ident);

            if (askAboutComments) {
                // "To record comments, press 1."
                // "To forward this message without comments, press 2."
                // "To cancel, press *."
                VmDialog vmd = new VmDialog(m_vm, "msg_forward");
                String digit = vmd.collectDigit("12");
                if (digit == null) {
                    // Timeout, cancel, or errors
                    return;
                }
                
                LOG.debug("Retrieve::forward collected ("+digit+")");
                
                if (digit.equals("1")) {
                    // "Record your comments, then press #"
                    // 
                    // "To play your comments, press 1."
                    // "To accept your comments, press 2."  
                    // "To delete these comments and try again, press 3."  
                    // To cancel, press *"
                    commentsFile = recordDialog("msg_record_comments", "msg_confirm_comments");
                    
                    if (commentsFile == null) {
                        continue ;
                    }
                } else  if (digit.equals("2")) {
                } else {
                    continue;
                }
                askAboutComments = false;
             // Build a message with the comments (if any) from the logged in user
                comments = Message.newMessage(null, commentsFile, 
                        m_mailbox.getUser().getUri(), Priority.NORMAL, null);
            }

            
            // Get a list of extensions 
            DialByNameChoice choice = EnterExtension.dialog(m_vm, m_loc);
            if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
                return;
            }

            // Forward the message to each destination that has a mailbox
            for (User destUser : choice.getUsers()) {
                if (destUser.hasVoicemail()) {
                    Mailbox destMailbox = new Mailbox(destUser);
                    vmMessage.forward(destMailbox, comments);
                    dontDeleteTempFile(commentsFile);
                }
            }
            
            // "Message forwarded."
            m_loc.play("msg_forwarded", "");

            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            VmDialog vmd = new VmDialog(m_vm, "deposit_more_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                return;
            }
            
            // Back to enter another extension
        }
    }
    
    /** 
     * Reply to a message, with a recorded comment back to the sender
     * @param vmMessage
     */
    void reply(VmMessage vmMessage, User sendingUser) {
        LOG.info("Retrieve::reply "+m_ident);

        // "Record your comments, then press #"
        // 
        // "To play your comments, press 1."
        // "To accept your comments, press 2."  
        // "To delete these comments and try again, press 3."  
        // To cancel, press *"
        File commentsFile = recordDialog("msg_record_comments", "msg_confirm_comments");

        if (commentsFile == null) {
            return;
        }

        // Find the user who sent this message
        Mailbox destMailbox = new Mailbox(sendingUser);
        
        // Build a message with the comments sent by this user.
        Message comments = Message.newMessage(destMailbox, commentsFile, 
                m_mailbox.getUser().getUri(), Priority.NORMAL, null);

        // Send the message.
        comments.storeInInbox();
        dontDeleteTempFile(commentsFile);
        
        // "Message sent."
        m_loc.play("msg_replyed", "");
    }
    
    /** 
     * Record a new message and send to a selected destination
     * @param vmMessage
     */
    void sendMessage() {
        LOG.info("Retrieve::sendMessage "+m_ident);

        Message message = null;
        
        // "Record your message, then press #"
        // 
        // "To play this message, press 1."
        // "To send this message, press 2." 
        // "To delete this message and try again, press 3."
        // "To cancel, press *."
        File recordingFile = recordDialog("send_record_message", "send_confirm_record");

        if (recordingFile == null) {
            return ;
        }
        
        for(;;) {
            // Get a list of extensions 
            DialByNameChoice choice = EnterExtension.dialog(m_vm, m_loc);
            if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
                return;
            }
            
            // Forward the message to each destination that has a mailbox
            for (User destUser : choice.getUsers()) {
                if (destUser.hasVoicemail()) {
                    Mailbox destMailbox = new Mailbox(destUser);
                    if (message == null) {
                        // Build a message with the recording sent by this user.
                        message = Message.newMessage(destMailbox, recordingFile, 
                                m_mailbox.getUser().getUri(), Priority.NORMAL, null);
                        // Send the message.
                        message.storeInInbox();
                        dontDeleteTempFile(recordingFile);
                    } else {
                        // Copy the existing message
                        message.getVmMessage().copy(destMailbox) ;
                    }
                }
            }
                
            // "Message sent."
            m_loc.play("msg_sent", "");

            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            VmDialog vmd = new VmDialog(m_vm, "send_more_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                return;
            }
            
            // Back to enter another extension
        }
    }


    void recordName() {
        // "Record your name, then press #"
        // 
        // "To listen to your recording, press 1."
        // "To use this recording, press 2." 
        // "To delete this recording and try again, press 3."
        // "To cancel, press *."
        File recordingFile = recordDialog("record_name", "name_confirm");

        if (recordingFile == null) {
            return ;
        }

        // Save the recording as the name
        File nameFile = m_mailbox.getRecordedNameFile();
        if (nameFile.exists()) {
            nameFile.delete();
        }
        // Do this before the rename, as once renamed recordingFile is changed
        dontDeleteTempFile(recordingFile);
        recordingFile.renameTo(nameFile);
        
        ExtMailStore.SaveSpokenNameInFolder(m_mailbox, nameFile);
        
        // Name recorded.
        m_loc.play("name_recorded", "");
    }
    
    void voicemailOptions() {
        User user = m_mailbox.getUser();
        
        voicemailOptions:
        for(;;) {
            LOG.info("Retrieve::voicemailOptions "+m_ident);
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
                vmd = new VmDialog(m_vm, "vm_options");
                validDigits = "12345";
            } else {
                // "Voicemail Options."
                // "To record a personal greeting, press 1."
                // "To record your name, press 2."
                // "To select the greeting to play, press 3."
                // "To clear your deleted messages folder, press 4."
                // "To return to the main menu, press *."            
                vmd = new VmDialog(m_vm, "vm_options_nopin");
                validDigits = "1234";
            }
            vmd.setSpeakCanceled(false);
            String digit = vmd.collectDigit(validDigits);
            if (digit == null) {
                // bad entry, timeout, canceled
                return ;
            }
            
            if (digit.equals("1")) {
                for (;;) {
                    // "To record a standard greeting, press 1."
                    // "To record an out-of-office greeting, press 2."
                    // "To record an extended absence greeting, press 3."
                    // "To cancel, press *."
                    vmd = new VmDialog(m_vm, "greeting_choice");
                    String digit1 = vmd.collectDigit("123");
                    if (digit1 == null) {
                        continue voicemailOptions;
                    }
                    
                    LOG.info("Retrieve::voicemailOptions:recordGreeting "+m_ident);

                    // "Record your greeting, then press #"
                    // 
                    // "To listen to this greeting, press 1."
                    // "To use this greeting, press 2." 
                    // "To delete this greeting and try again, press 3."
                    // "To cancel, press *."
                    File recordingFile = recordDialog("record_greeting", "greeting_confirm");
    
                    if (recordingFile == null) {
                        continue;
                    }
                    
                    // Save the recording as the appropriate greeting 
                    Greeting greeting = new Greeting(m_vm) ;
                    GreetingType type = GreetingType.NONE;
                    if (digit1.equals("1")) {
                        type = GreetingType.STANDARD;
                    } else if (digit1.equals("2")) {
                        type = GreetingType.OUT_OF_OFFICE;
                    } else if (digit1.equals("3")) {
                        type = GreetingType.EXTENDED_ABSENCE;
                    }
                    dontDeleteTempFile(recordingFile);
                    greeting.saveGreetingFile(type, recordingFile);
                    
                    // Greeting recorded.
                    m_loc.play("greeting_recorded", "");

                    continue voicemailOptions;
                }
            }

            if (digit.equals("2")) {
                LOG.info("Retrieve::voicemailOptions:recordName "+m_ident);
                recordName();
                continue voicemailOptions;
            }
            
            if (digit.equals("3")) {
                PromptList greetings = null;
                for(;;) {
                    LOG.info("Retrieve::voicemailOptions:selectGreeting "+m_ident);

                    // "Select the greeting to play to your callers."
                    // "To listen to your greetings before making a selection, press 1."
                    // "To select the standard greeting, press 2."
                    // "To select the out-of-office greeting, press 3."
                    // "To select the extended absence greeting, press 4."
                    // "To select the default system greeting, press 5."
                    // "To cancel, press *."
                    vmd = new VmDialog(m_vm, "select_greeting");
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
                        Greeting greeting = new Greeting(m_vm);
                        greetings = m_loc.getPromptList();
                        // "Your standard greeting is...{prompts}"
                        // "Your out-of-office greeting is...{prompts}"
                        // "Your extended absence greeting is...{prompts}"
                        // "The default system greeting is...{prompts}"
                        // "Your active greeting is...{prompts}"
                        greetings.addFragment("play_greetings", 
                                greeting.getPromptList(GreetingType.STANDARD).toString(),
                                greeting.getPromptList(GreetingType.OUT_OF_OFFICE).toString(),
                                greeting.getPromptList(GreetingType.EXTENDED_ABSENCE).toString(),
                                greeting.getPromptList(GreetingType.NONE).toString(),
                                greeting.getPromptList().toString());
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
                    PromptList pl = m_loc.getPromptList(greetingFrag);
                    
                    // "If this is correct, press 1."
                    // "To select a different greeting, press 2."
                    pl.addFragment("selected_greeting_confirm");
                    vmd = new VmDialog(m_vm, null);
                    vmd.setPromptList(pl);
                    String digit2 = vmd.collectDigit("12");
                    if (digit2 == null) {
                        continue voicemailOptions;
                    }

                    if (digit2.equals("1")) {
                        m_mailbox.getMailboxPreferences().getActiveGreeting().setGreetingType(type);
                        m_mailbox.writeMailboxPreferences();
                        // Active greeting set successfully.
                        m_loc.play("selected_greeting_okay", "");
                        continue voicemailOptions;
                    }
                }
            }
            
            if (digit.equals("4")) {
                LOG.info("Retrieve::voicemailOptions:destroyDeletedMessages "+m_ident);

                m_messages.destroyDeletedMessages();
                // Your deleted messages folder is now empty.
                m_loc.play("deleted_okay", "");
                continue voicemailOptions;
            }
            
            if (user.canTuiChangePin() && digit.equals("5")) {
                int errorCount = 0;
                for(;;) {
                    LOG.info("Retrieve::voicemailOptions:changePin "+m_ident);

                    if (errorCount > m_vm.getConfig().getInvalidResponseCount()) {
                        m_vm.failure();
                        return;
                    }

                    // "Enter your personal identification number, and then press #."
                    // (Oh I was born an original pinner, I was born from original pin...)
                    PromptList pl1 = m_loc.getPromptList("original_pin");
                    VmMenu menu1 = new VmMenu(m_vm);
                    menu1.setOperatorOn0(false);
                    m_fses.setRedactDTMF(true);
                    IvrChoice choice1 = menu1.collectDigits(pl1, 10);
                    m_fses.setRedactDTMF(false);

                    if (!menu1.isOkay()) {
                        continue voicemailOptions;
                    }
                    String originalPin = choice1.getDigits();
                    String newPin = "";

                    for(;;) {
                        // "Enter your new personal identification number, and then press #."
                        pl1 = m_loc.getPromptList("new_pin");
                        menu1 = new VmMenu(m_vm);
                        menu1.setOperatorOn0(false);
                        m_fses.setRedactDTMF(true);
                        choice1 = menu1.collectDigits(pl1, 10);
                        m_fses.setRedactDTMF(false);
    
                        if (!menu1.isOkay()) {
                            continue voicemailOptions;
                        }
                        newPin = choice1.getDigits();
    
                        // "Enter your new personal identification number again, and then press #."
                        pl1 = m_loc.getPromptList("new_pin2");
                        menu1 = new VmMenu(m_vm);
                        menu1.setOperatorOn0(false);
                        m_fses.setRedactDTMF(true);
                        choice1 = menu1.collectDigits(pl1, 10);
                        m_fses.setRedactDTMF(false);
    
                        if (!menu1.isOkay()) {
                            continue voicemailOptions;
                        }
                        String newPin2 = choice1.getDigits();
                        
                        if (newPin.equals(newPin2)) {
                            break;
                        }
                        errorCount++;
                        LOG.info("Retrieve::voicemailOptions:changePin "+m_ident+" Pins do not match.");
                        // "The two personal identification numbers you have entered do not match."
                        m_loc.play("pin_mismatch", "");
                    }
                    
                    String realm = m_loc.getConfig().getRealm();
                    if (!user.isPinCorrect(originalPin, realm)) {
                        errorCount++;
                        LOG.info("Retrieve::voicemailOptions:changePin "+m_ident+" Pin invalid.");
                        // "The personal identification number you have entered is not valid."
                        m_loc.play("pin_invalid", "");
                        continue;
                    }
                    
                    try {
                        // Use sipXconfig's RESTful interface to change the PIN
                        RestfulRequest rr = new RestfulRequest(
                                ((IvrConfiguration)m_loc.getConfig()).getConfigUrl()+"/sipxconfig/rest/my/voicemail/pin/", 
                                user.getUserName(), originalPin);
                        if (rr.put(newPin)) {
                            LOG.info("Retrieve::voicemailOptions:changePin "+m_ident+" Pin changed.");

                            // "Personal identification number changed."
                            m_loc.play("pin_changed","");
                            continue voicemailOptions;
                        }
                        LOG.error("Retrieve::voicemailOptions new pin trouble "+rr.getResponse());
                    } catch (Exception e) {
                        LOG.error("Retrieve::voicemailOptions new pin trouble", e);
                    }
                    // "An error occurred while processing your request."
                    // "Your personal identification number is not changed."
                    m_loc.play("pin_change_failed", "");
                    continue voicemailOptions;
                }
            }
        }
    }
    
    void adminOptions() {
        adminOptions:
        for(;;) {
            LOG.info("Retrieve::adminOptions "+m_ident);

            // "System Administration Options."
            // "To manage the auto attendant prompts, press 1."
            // "To return to the main menu, press *."
            VmDialog vmd = new VmDialog(m_vm, "sysadmin_options");
            vmd.setSpeakCanceled(false);
            if (vmd.collectDigit("1") == null) {
                // bad entry, timeout, canceled
                return;
            }
            
            // No need to check what they pressed, if it wasn't "1", then we aren't here!
            
            // "To record the Auto Attendant prompt, press 1."
            // "To Manage the special Auto Attendant menu, press 2."
            // "To cancel, press *."
            vmd = new VmDialog(m_vm, "sysadmin_opts2");
            String digit = vmd.collectDigit("12");
            if (digit == null) {
                // bad entry, timeout, canceled
                continue adminOptions;
            }
            
            if (digit.equals("1")) {
                LOG.info("Retrieve::adminOptions:recordAA "+m_ident);

                // "Record the Auto Attendant prompt, then press #"
                // 
                // "To listen to your recording, press 1."
                // "To use this recording, press 2." 
                // "To delete this recording and try again, press 3."
                // "To cancel, press *."
                File recordingFile = recordDialog("record_aa", "aa_confirm");

                if (recordingFile == null) {
                    continue adminOptions;
                }

                String aaName = String.format("customautoattendant-%d.wav", System.currentTimeMillis()/1000);
                File aaFile = new File(((IvrConfiguration)m_loc.getConfig()).getPromptsDirectory(), aaName);
                
                // Save the recording as the aaFile
                if (aaFile.exists()) {
                    aaFile.delete();
                }
                // Do this before the rename, as once renamed recordingFile is changed
                dontDeleteTempFile(recordingFile);
                recordingFile.renameTo(aaFile);
                
                // Auto Attendant prompt recorded.
                m_loc.play("aa_recorded", "");
                
                continue adminOptions;
            }
            
            if (digit.equals("2")) {
                // "To enable the special autoattendant menu, press 1."
                // "To disable it, press 2."
                // "To cancel, press *."
                vmd = new VmDialog(m_vm, "special_menu_options");
                String digit1 = vmd.collectDigit("12");
                if (digit1 == null) {
                    continue adminOptions;
                }
                // Tell sipXconfig about the change.
                
                try {
                    // Use sipXconfig's RESTful interface to change the special mode
                    RestfulRequest rr = new RestfulRequest(
                            ((IvrConfiguration)m_loc.getConfig()).getConfigUrl()+"/sipxconfig/rest/auto-attendant/specialmode", 
                            m_mailbox.getUser().getUserName(), m_userEnteredPin);
                    
                    if (digit1.equals("1")) {
                        if (rr.put(null)) {
                            LOG.info("Retrieve::adminOptions:specialmode "+m_ident+" specialmode enabled.");
                            // "Special Auto Attendant menu is enabled."
                            m_loc.play("special_menu_enabled","");
                            continue adminOptions;
                        }
                    } else if (digit1.equals("2")) {
                        if (rr.delete()) {
                            LOG.info("Retrieve::adminOptions:specialmode "+m_ident+" specialmode disabled.");
                            // "Special Auto Attendant menu is disabled."
                            m_loc.play("special_menu_disabled","");
                            continue adminOptions;
                        }
                    }
                    LOG.error("Retrieve::adminOptions:specialmode trouble "+rr.getResponse());
                } catch (Exception e) {
                    LOG.error("Retrieve::adminOptions:specialmode trouble", e);
                }
                // "An error occurred while processing your request."
                m_loc.play("special_menu_failed", "");
            }
        }

    }

    /**
     * Record a wav file with confirmation dialog.
     * 
     * @param recordFragment  To play before the recording
     * @param confirmMenuFragment  To play after the recording
     * @return the temporary wav file.  null if recording is to be tossed
     */
    File recordDialog(String recordFragment, String confirmMenuFragment) {
        File wavFile = makeTempWavFile();
        String wavPath = wavFile.getPath();
    
        boolean recordWav = true ;
        boolean playWav = false;
        for(;;) {
    
            if (recordWav) {
                // Record your {thingy} then press #
                m_loc.play(recordFragment, "*");
                // Give the user 1 second to press "*" to cancel, then start recording
                Collect c = new Collect(m_fses, 1, 1000, 0, 0);
                c.setTermChars("*");
                c.go();
                String digit = c.getDigits();
                LOG.debug("Retrieve::recordDialog collected ("+digit+")");
                if (digit.length() == 0 || !"*0".contains(digit) ) {
                    m_vm.recordMessage(wavPath);
                    digit = m_fses.getDtmfDigit();
                    if (digit == null) {
                        digit = "";
                    }
                    LOG.debug("Retrieve::recordDialog record terminated collected ("+digit+")");
                }
        
                if (digit.equals("0")) {
                    m_vm.operator();
                    return null;
                }
        
                if (digit.equals("*")) {
                 // "Canceled."
                    m_loc.play("canceled", "");
                    return null;
                }
                recordWav = false;
            }
        
            // Confirm caller's intent for this {thingy}

            // (pre-menu: {recording})

            // "To play your {thingy}, press 1."
            // "To accept your {thingy}, press 2."  
            // "To delete this {thingy} and try again, press 3."  
            // "To cancel, press *"
            VmDialog vmd = new VmDialog(m_vm, confirmMenuFragment);
            if (playWav) {
                PromptList messagePl = new PromptList(m_loc);
                messagePl.addPrompts(wavPath);
                vmd.setPrePromptList(messagePl);
            }
    
            String digit = vmd.collectDigit("123");
    
            // bad entry, timeout, canceled
            if (digit == null) {
                return null;
            }
                
            LOG.info("Retrieve::recordDialog "+m_ident+" options ("+digit+")");
    
            // "1" means play the recording
            if (digit.equals("1")) {
                LOG.info(String.format("Retrieve::recordDialog "+m_ident+" Playing back recording (%s)", wavPath));
                playWav = true ;
                continue ;
            }
    
            // "2" means accept the recording
            if (digit.equals("2")) {
                LOG.info(String.format("Retrieve::recordDialog "+m_ident+" accepted recording (%s)", wavPath));
                return wavFile;
            }
    
            // "3" means "erase" and re-record
            if (digit.equals("3")) {
                recordWav = true ;
                continue ;
            }
        }
    }
}
