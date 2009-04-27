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

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Collect;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.Menu;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.Record;
import org.sipfoundry.sipxivr.TextToPrompts;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.Messages.Folders;

public class Retrieve {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    static final long MSPERDAY = 1000*60*60*24; // number of mS per day

    VoiceMail m_vm;
    Localization m_loc;
    FreeSwitchEventSocketInterface m_fses;
    Mailbox m_mailbox;
    Messages m_messages;

    public Retrieve(VoiceMail vm, Localization loc) {
        m_vm = vm;
        m_loc = loc ;
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
        m_mailbox = m_vm.getMailbox();
    }
    
    public String retrieveVoiceMail() {
        String displayUri = m_fses.getDisplayUri();
        LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Retrieve Voicemail from "+displayUri);

        m_fses.setRedactDTMF(true);
        m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
        User user = login() ;
        m_fses.setRedactDTMF(false);
        if (user != null) {
            m_mailbox = new Mailbox(user, m_loc);
            LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" logged in");
            main_menu();
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
        User user = m_mailbox.getUser();

        // Welcome.  Your call has been answered by an automated communications system.
        PromptList welcomePl = m_loc.getPromptList("welcome");
        boolean playWelcome = true;
        
        for(;;) {
            if (errorCount > m_vm.getConfig().getInvalidResponseCount()) {
                m_vm.failure();
                return null;
            }
            
            // "Enter your personal identification number, and then press #.":
            // "To log in as a different user, press #"
            PromptList menuPl = m_loc.getPromptList("enter_pin");
            VmMenu menu = new VmMenu(m_loc, m_vm);
            if (playWelcome) {
                menu.setPrePromptPl(welcomePl);
                playWelcome = false;
            }
            IvrChoice choice = menu.collectDtmf(menuPl, 10);

            if (!menu.isOkay()) {
                return null;
            }
            
            if (choice.getDigits().equals("#")) {
                // "Enter your extension."
                PromptList extPl = m_loc.getPromptList("enter_extension");
                VmMenu extMenu = new VmMenu(m_loc, m_vm);
                IvrChoice extChoice = extMenu.collectDigits(extPl, 10);
                if (!extMenu.isOkay()) {
                    return null ;
                }
                // See if the user exists
                user = m_vm.getValidUsers().isValidUser(extChoice.getDigits());
                continue;
            }
            
            if (user == null || !user.isPinCorrect(choice.getDigits(), m_loc.getIvrConfig().getRealm())) {
                // WRONG, do it again!
                
                // "That personal identification number is not valid
                m_loc.play("invalid_pin", "");
                ++errorCount;
                continue;
            }
            LOG.info("Retrieve::getpasscode PIN is valid");
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
        
        int unheardCount = m_messages.getUnheardCount();
        int heardCount = m_messages.getHeardCount();
        int savedCount = m_messages.getSavedCount();
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
    
    private void main_menu() {
        m_messages = new Messages(m_mailbox);
        
        boolean playStatus = true ;
        for(;;) {
            VmMenu mainMenu = new VmMenu(m_loc, m_vm);
            mainMenu.setSpeakCanceled(false);

            if (playStatus) {
                // Include the status as pre-menu info
                PromptList statusPl = status();
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
            PromptList menuPl = m_loc.getPromptList("main_menu");
            
            // TODO what selects this prompt?
            // For system administration options, press 7.
            menuPl.addFragment("main_menu_options");
            mainMenu.collectDigit(menuPl, "1234578");
            
            if (mainMenu.getChoice().equals(IvrChoiceReason.CANCELED)) {
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
        }
    }

    void playMessages(Messages.Folders folder) {
        List<VmMessage> folderList = null;;
        String menuFragment = null;
        String validDigits = null ;
        switch (folder) {
            case INBOX: folderList = m_messages.getInbox(); break;
            case SAVED: folderList = m_messages.getSaved(); break;
            case DELETED: folderList = m_messages.getDeleted(); break;
        }
        for (VmMessage vmMessage : folderList) {
            boolean playMessage = true;
            boolean playInfo = false;
            for(;;) {
                PromptList messagePl = m_loc.getPromptList();
                PromptList prePromptPl = m_loc.getPromptList();
                
                // {the message}
                messagePl.addPrompts(vmMessage.getAudioFile().getPath());
                VmMenu menu = new VmMenu(m_loc, m_vm);
                menu.setPrePromptPl(prePromptPl);
                
                // Read the message descriptor file to obtain the info we need
                MessageDescriptor md = new MessageDescriptorReader().readObject(vmMessage.getDescriptorFile());
                // Determine if the message is from a known user
                String from = ValidUsersXML.getUserPart(md.getFromUri());                
                User user = m_vm.getValidUsers().isValidUser(from);
                
                switch (folder) {
                case INBOX: 
                    validDigits="12345#";
                    if (user != null) {
                        menuFragment = "msg_inbox_options_reply";
                        validDigits += "6";
                    } else {
                        menuFragment = "msg_inbox_options";
                    }
                    // "To play information about this message, press 1.  "
                    // "To replay press 2."
                    // "To save, press 3."
                    // "To delete press 4."
                    // "To forward to another inbox, press 5."
                    // {if from sipXuser} "To reply, press 6."
                    // "To play the next message, press #."
                    // "To return to the main menu, press *."
                    break;
                case SAVED: 
                    validDigits="1245#";
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
                    break;
                case DELETED: 
                    validDigits="12345#";
                    if (user != null) {
                        menuFragment = "msg_deleted_options_reply";
                        validDigits += "6";
                    } else {
                        menuFragment = "msg_deleted_options";
                    }
                    // "To play information about this message, press 1.
                    // To replay press 2. 
                    // To restore to inbox, press 3. 
                    // To delete permanently press 4.  
                    // forward to another inbox, press 5.  
                    // {if from sipXuser} To reply, press 6.  
                    // To play the next message, press #. 
                    // To return to the main menu, press *.

                    break;
                }
                PromptList menuPl = m_loc.getPromptList(menuFragment);
                
                // If we need to play the message, add it as a prePrompt to the menu.
                // This is so we can barge it with a digit press and act on the digit in the menu.
                if (playMessage) {
                    prePromptPl.addPrompts(messagePl);
                    playMessage = false;
                }
                // Same with the message info.
                if (playInfo) {
                    prePromptPl.addPrompts(messageInfo(md));
                    playInfo = false;
                }
                menu.collectDigit(menuPl, validDigits);
     
                // Timeout, cancel, or errors
                if (!menu.isOkay()) {
                    return;
                }
     
                // Mark the message heard (if it wasn't before)
                m_messages.markMessageHeard(vmMessage);
                
                String digit = menu.getChoice().getDigits();
                if (digit.equals("1")) {
                    playInfo = true;
                    continue;
                }
                if (digit.equals("2")) {
                    playMessage = true;
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
                    break;
                }
                if (digit.equals("5")) {
                    forward(vmMessage);
                    continue;
                }
                if (digit.equals("6")) {
                    // TODO the reply thing
                    continue;
                }
                if (digit.equals("#")) {
                    break;
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
        User user = m_vm.getValidUsers().isValidUser(from);
        if (user != null) {
            // Well, looky here!  A sipXecs user!  Get his recorded name if we can
            Mailbox userMbox = new Mailbox(user, m_loc);
            String nameWav = userMbox.getRecordedName();
            if (nameWav != null) {
                fromPrompts = nameWav;
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
        String commentsWav;
        boolean askAboutComments = true;
        for(;;) {
            if (askAboutComments) {
                VmMenu menu = new VmMenu(m_loc, m_vm);
                // "To record comments, press 1."
                // "To forward this message without comments, press 2."
                // "To cancel, press *."
                PromptList menuPl = m_loc.getPromptList("msg_forward");
                menu.collectDigit(menuPl, "12");
                
                // Timeout, cancel, or errors
                if (!menu.isOkay()) {
                    return;
                }
    
                String digit = menu.getChoice().getDigits();
                LOG.debug("Retrieve::forward collected ("+digit+")");
                
                if (digit.equals("1")) {
                    commentsWav = recordComments();
                    if (commentsWav == null) {
                        continue ;
                    }
                } else  if (digit.equals("2")) {
                } else {
                    continue;
                }
                askAboutComments = false;
            }

            // Get a list of extensions 
            EnterExtension ee = new EnterExtension(m_vm, m_loc);
            DialByNameChoice choice = ee.extensionDialog();
            if (choice.getIvrChoiceReason() == IvrChoiceReason.SUCCESS) {
                // TODO the actual work of forwarding
                
                // "Message forwarded."
                m_loc.play("msg_forwarded", "");

                // "To deliver this message to another address, press 1."
                // "If you are finished, press *."
                PromptList pl = m_loc.getPromptList("deposit_more_options");
                VmMenu menu1 = new VmMenu(m_loc, m_vm);
                menu1.setSpeakCanceled(false);
                menu1.collectDigit(pl, "1");
        
                if (!menu1.isOkay()) {
                    return;
                }
                // Back to enter another extension
            }
        }
    }
    
    String recordComments() {
        String wavName = "oops" ;
        File wavFile;

        // Time to record a message
        try {
            wavFile = File.createTempFile("temp_recording_", ".wav"); // TODO which tmp dir?
            wavName = wavFile.getPath();
        } catch (IOException e) {
            throw new RuntimeException("Cannot create temp recording file", e);
        }
        boolean recordComments = true ;
        boolean playComments = false;
        for(;;) {

            if (recordComments) {
                // Record your comments, then press #
                m_loc.play("msg_record_comments", "*");
                // Give the user 1 second to press "*" to cancel, then start recording
                Collect c = new Collect(m_fses, 1, 1000, 0, 0);
                c.setTermChars("*");
                c.go();
                String digit = c.getDigits();
                LOG.debug("Retrieve::comments collected ("+digit+")");
                if (digit.length() == 0 || !"*0".contains(digit) ) {
                    m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
                    Record rec = m_vm.recordMessage(wavName);
                    digit = m_fses.getDtmfDigit();
                    if (digit == null) {
                        digit = "";
                    }
                    LOG.debug("Retrieve::comments record terminated collected ("+digit+")");
                }
        
                if (digit.equals("0")) {
                    FileUtils.deleteQuietly(wavFile);
                    m_vm.operator();
                    return null;
                }
        
                if (digit.equals("*")) {
                 // "Canceled."
                    FileUtils.deleteQuietly(wavFile);
                    m_loc.play("canceled", "");
                    return null;
                }
                recordComments = false;
            }
        
            // Confirm caller's intent for this message
            Menu menu2 = new VmMenu(m_loc, m_vm);
            if (playComments) {
                // (pre-menu: message)
                PromptList messagePl = new PromptList(m_loc);
                messagePl.addPrompts(wavName);
                menu2.setPrePromptPl(messagePl);
                playComments = false ; // Only play it once
            }

            // "To play your comments, press 1."
            // "To accept your comments, press 2."  
            // "To delete these comments and try again, press 3."  
            // To cancel, press *"
            PromptList pl = m_loc.getPromptList("msg_confirm_comments");
            IvrChoice choice = menu2.collectDigit(pl, "123");

            // bad entry, timeout, canceled
            if (!menu2.isOkay()) {
                FileUtils.deleteQuietly(wavFile);
                return null;
            }
                
            String digit = choice.getDigits();

            LOG.info("Retrieve::comments: Mailbox "+m_mailbox.getUser().getUserName()+" options ("+digit+")");
    
            // "1" means play the recording
            if (digit.equals("1")) {
                LOG.info(String.format("Playing back comment (%s)", wavName));
                playComments = true ;
                continue ;
            }
    
            // "2" means accept the comments recording
            if (digit.equals("2")) {
                return wavName;
            }
    
            // "3" means "erase" and re-record
            if (digit.equals("3")) {
                recordComments = true ;
                continue ;
            }
        }
    }
}
