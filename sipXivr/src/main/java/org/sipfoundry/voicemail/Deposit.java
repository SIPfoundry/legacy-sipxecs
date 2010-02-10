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
import java.net.URL;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.apache.log4j.Logger;
import org.sipfoundry.callpilot.CpCmd;
import org.sipfoundry.callpilot.CpDialog;
import org.sipfoundry.callpilot.CpThruDial;
import org.sipfoundry.callpilot.CpCmd.Command;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.GreetingType;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.Menu;
import org.sipfoundry.sipxivr.PersonalAttendant;
import org.sipfoundry.sipxivr.RemoteRequest;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class Deposit {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private  VoiceMail m_vm;
    private  Localization m_loc;
    private FreeSwitchEventSocketInterface m_fses;
    private Mailbox m_mailbox;
    private PersonalAttendant m_pa;
    private Greeting m_greeting;
    private Message m_message;
    
    // maps username to freeswitch channel UUID
    private static Map<String, String> m_depositMap = Collections.synchronizedMap(new HashMap<String, String>());

    public static String getChannelUUID(User user) {
        return m_depositMap.get(user.getUserName());
    }
    
    private void sendIM(User user, boolean vmEntry, String instantMsg) {
        URL sendIMUrl;
        try {
            if(vmEntry) {
                sendIMUrl = new URL(IvrConfiguration.get().getSendIMUrl() + "/" +
                                    user.getUserName() + "/SendVMEntryIM");
            
            } else {
                sendIMUrl = new URL(IvrConfiguration.get().getSendIMUrl() + "/" +
                        user.getUserName() + "/SendVMExitIM");
            }
                
            RemoteRequest rr = new RemoteRequest(sendIMUrl, "text/plain", instantMsg);
            if (!rr.http()) {
                LOG.error("Deposit::sendIM Trouble with RemoteRequest "+ rr.getResponse());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }       
    }
        
    private void putChannelUUID(User user, String uuid) {
        m_depositMap.put(user.getUserName(), uuid);     
        
        sendIM(m_mailbox.getUser(), true, m_fses.getVariable("channel-caller-id-name") +
               " (" + m_fses.getVariable("channel-caller-id-number") + ")" +        
               " is leaving a voice message.");
    }
    
    private void clearChannelUUID(User user) {
        if(m_depositMap.remove(user.getUserName()) != null) {
            
            String description = " disconnected without leaving a voice message.";
            if(m_message != null) {
                 
                if(m_message.isToBeStored() && m_message.getDuration() > 1) {
                    description = " just left a voice message.";
                }
            }            
            
            sendIM(m_mailbox.getUser(), false, m_fses.getVariable("channel-caller-id-name") +
                   " (" + m_fses.getVariable("channel-caller-id-number") + ")" +     
                   description);
        }
    }
    
    public Deposit(VoiceMail vm) {
        m_vm = vm;
        
        m_loc = m_vm.setLoc(m_vm.usingCpUi(null));
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
        m_mailbox = vm.getMailbox();
        m_pa = m_mailbox.getPersonalAttendant();
        m_message = null;
        
        String localeString = m_pa.getLanguage();
        if (localeString != null) {
            LOG.debug("Changing locale for this call to "+localeString);
            m_loc.changeLocale(localeString);
        }
        
        m_greeting = new Greeting(m_vm);
    }
    
    private Command processDeletion() {
        Command cmd;
        
        m_message.setIsToBeStored(false);
        m_loc.play("msg_deleted", "");
        
        for(;;) {
            CpCmd thecmd = new CpCmd(m_vm, "lve_msg_empty_immed", 
                                           "lve_msg_empty_delay", 
                                           "lve_msg_empty_help");
            cmd = thecmd.CollectCmd();
                       
            switch (cmd) {  
            case REVERT:             
            case CANCELED:
            case RECORD:
            case LOGIN:
                return cmd;

            default:
                m_vm.playError("bad_cmd");
                break;
            }
        }
    }
    
    private void depositMsg() { 
        
        if(m_message.getDuration() > 1) {
            m_message.storeInInbox(); 
            if(m_message.isToBeStored()) {
                m_loc.play("msg_left", "");
            }
        } else {
            m_message.setIsToBeStored(false);
        }
        
        clearChannelUUID(m_mailbox.getUser());
    }
            
    private String depositCPUI(boolean isOnThePhone) {                    
        
        boolean msgLeft = false;
        boolean recorded = false ;
        boolean playMessage = false;
        Command cmd = null;
        String digit;
        PromptList messagePl;
        boolean annOnPhone = false;
        boolean grtPlayedOnce = false;
        boolean isTag = false;
        CpDialog cpDialog;
        
        PromptList pl = new PromptList(m_loc);    
        
        GreetingType type = m_mailbox.getMailboxPreferences().getActiveGreeting().getGreetingType();
        boolean grtRecorded = m_greeting.getGreetingPath(type) != null;        
        boolean userBusyPrompt = m_mailbox.getUser().userBusyPrompt();
        
        for(;;) {    
        
            // Check if the user wants to generate the "on the phone" message.
            if(userBusyPrompt) {
                if(isOnThePhone) {
                    File nameFile = m_mailbox.getRecordedNameFile();
                    if (nameFile.exists()) {           
                        // {name} is on the phone
                        pl.addFragment("on_phone_hasname", nameFile.getPath());
                    } else {  
                        // extension {letters} is on the phone
                        pl.addFragment("on_phone_spellout", m_mailbox.getUser().getUserName());
                    }                 
                    annOnPhone = true;
                }
            }
            
            // if not on the phone and the type is out_of_office or extended_away then play the CHIME
            if(!annOnPhone && grtRecorded &&  !grtPlayedOnce) {
                if((type == GreetingType.OUT_OF_OFFICE) || (type == GreetingType.OUT_OF_OFFICE)) {
                    isTag = true;
                    pl.addFragment("chime");
                }         
            }      
            
            if(!annOnPhone || grtRecorded) {
                // if we are presenting the on the phone prompt then 
                // only add greeting if user recorded one
                pl.addPrompts(m_greeting.getPromptList());          
            }
        
            m_loc.play(pl, "#0*i58"+m_pa.getValidDigits());
            
            Collect c = new Collect(m_fses, 1, 100, 0, 0);
            c.setTermChars("#");
            c.go();
            digit = c.getDigits();  
            
            if(digit.equals("#") && isTag && !grtPlayedOnce) {        
                grtPlayedOnce = true;
                cpDialog = new CpDialog(m_vm, "is_tag_greeting");
                
                String tagCmd = cpDialog.collectDigit("2#");
                
                if(tagCmd.equals("2")) {
                    pl = new PromptList(m_loc);  
                    continue;
                }    
                
                break;
            }                   
                
            if (digit.equals("*")) {
                pl = new PromptList(m_loc);  
                continue;
            }  
            
            if(digit.equals("i")) {
                m_loc.play("please_hold", "");
                m_vm.transfer(m_mailbox.getUser().getUri());
                return null;
            }
            
            cmd = processCPUI(digit, null);
            if(cmd == Command.LOGIN) {
                clearChannelUUID(m_mailbox.getUser());
                return "retrieve";
            }
                                    
            if(cmd == Command.CANCELED) {
                pl = new PromptList(m_loc);  
                continue;
            } else {
                break;   
            }
        }   
        
        String wavPath = createMsg();
          
        record:
        while(!msgLeft) {
            // Record the message
            digit = null;
            if (!recorded) {
                m_message.setIsToBeStored(true);  // So if they hang up now, we'll save what we got.
                m_vm.recordMessage(wavPath);
    
                digit = m_fses.getDtmfDigit();
                               
                if(digit != null && digit.equals("i")) {
                    m_loc.play("please_hold", "");
                    m_vm.transfer(m_mailbox.getUser().getUri());
                    return null;
                }
                                 
                if (digit != null && digit.equals("0")) {
                    depositMsg();
                    CpThruDial thrudial = new CpThruDial(m_vm);
                    thrudial.go();
                    return null;
                } 
                                
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Deposit Voicemail recorded message");
                recorded = true ;
            }            
                
            while(!msgLeft) {
                
                messagePl = null;
                if(playMessage) {                      
                    messagePl = new PromptList(m_loc);
                    messagePl.addPrompts(wavPath);                
                    playMessage = false;
                }
                
                cmd = processCPUI(null, messagePl);
                            
                switch (cmd) {  
                case DELETE:     
                    cmd = processDeletion();
                    if(cmd == Command.RECORD) {
                        recorded = false;
                        continue record;
                    }
                    continue;
                    
                case SEND:
                    depositMsg();
                    msgLeft = true;
                    break;
                    
                case RECORD: 
                    recorded = false;
                    continue record;
                    
                case PLAY:
                    playMessage = true;
                    break;
                    
                case LOGIN:
                    depositMsg();
                    return "retrieve";
                    
                case TOGGLE_URGENCY:
                    m_message.togglePriority();
                    if(m_message.getPriority() == Priority.URGENT) {
                        m_loc.play("msg_tagged_urgent", "");
                    } else {
                        m_loc.play("msg_tagged_normal", "");
                    }
                    break;
                    
                case CANCELED:
                    break;    
                    
                case REVERT:
                    depositMsg();
                    CpThruDial thrudial = new CpThruDial(m_vm);
                    thrudial.go();
                    return null; 
                
                default:
                    m_vm.playError("bad_cmd");
                    break;
                } 
            }
            
            // message left now check for last minute options: 81 (login) and 0
            for(;;) {
                CpCmd thecmd = new CpCmd(m_vm, null, "lve_msg_end_help", "lve_msg_end_help");
                cmd = thecmd.CollectCmd();
                
                if(cmd == Command.LOGIN) {
                    return "retrieve";
                } 
                
                if(cmd == Command.REVERT) {                 
                    CpThruDial thrudial = new CpThruDial(m_vm);
                    thrudial.go();
                }    
                    
                if(cmd != Command.CANCELED) {
                    m_vm.playError("bad_cmd");
                }         
            }
        }
        
        clearChannelUUID(m_mailbox.getUser());
        return null;
    }
    
    private String createMsg() {
        String wavPath = null;
        
        if (m_message == null) {
            File wavFile;
            try {
                // Create in the deleted directory, so if somehow things fail, they'll get removed
                // as they age out.
                wavFile = File.createTempFile("temp_recording_", ".wav",
                        new File(m_mailbox.getDeletedDirectory()));
                wavPath = wavFile.getPath();
            } catch (IOException e) {
                throw new RuntimeException("Cannot create temp recording file", e);
            }
            
            m_message = Message.newMessage(m_mailbox, wavFile, m_fses.getDisplayUri(), 
                                           Priority.NORMAL, null);
        } else { 
            wavPath = m_message.getWavPath();
        }
        
        return wavPath;
    }
    
    /**
     * The depositVoicemail dialog 
     * 
     * @return
     */
    public String depositVoicemail(boolean isOnThePhone) {
        try {
        
        User user = m_mailbox.getUser();
        LOG.info("Mailbox "+user+" Deposit Voicemail from " + m_fses.getDisplayUri());              
                
        putChannelUUID(user, m_fses.getVariable("channel-unique-id"));
        
        // by passing null, usingCpUi will return if CPUI is the primary UI
        if(m_vm.usingCpUi(null)) {
            return depositCPUI(isOnThePhone);
        }        
        
        Greeting:
        for(;;) {
            PromptList pl = m_greeting.getPromptList(); ;
        
            // When you are finished, press 1 for more options.
            // To reach the operator, dial 0 at any time.
            
            // Allow caller to barge with 0, *, and any defined Personal Attendant digit
            // Also, allow barge to recording with "#"
        
            m_loc.play(pl, "#0*i"+ m_pa.getValidDigits());
        
            Collect c = new Collect(m_fses, 1, 100, 0, 0);
            c.setTermChars("#");
            c.go();
            String digits = c.getDigits();  
            LOG.info("depositVoicemail Collected digits=" + digits);
            
            if(digits.equals("i")) {
                m_loc.play("please_hold", "");
                m_vm.transfer(m_mailbox.getUser().getUri());
                return null;
            }
                             
            if (digits.equals("*")) {
                clearChannelUUID(m_mailbox.getUser());
                return "retrieve";
            }
            
            // See if the digit they pressed was defined in the Personal Attendant
            String transferUrl = null;
            if (digits.equals("0")) {
                transferUrl = m_vm.getOperator(m_pa);
            } else {
                // See if the Personal Attendant defined that digit to mean anything
                transferUrl = m_pa.getMenuValue(digits) ;
            }
            
            if (transferUrl != null) {
                LOG.info(String.format("Transfer to %s", transferUrl));
                m_vm.transfer(transferUrl);
                return null ;
            }
            
            String wavPath = createMsg();
                        
            boolean recorded = false ;
            boolean playMessage = false;
            for(;;) {
                // Record the message
                if (!recorded) {
                    m_message.setIsToBeStored(true);  // So if they hang up now, we'll save what we got.
                    m_vm.recordMessage(wavPath);
        
                    String digit = m_fses.getDtmfDigit();
                    if (digit != null && digit.equals("0")) {
                        if(m_message.getDuration() > 2) {
                            m_message.storeInInbox(); 
                            m_loc.play("msg_sent", ""); 
                        } else {
                            m_message.setIsToBeStored(false);
                        }
                        
                        m_vm.transfer(m_vm.getOperator(m_pa));
                        return null;
                    }
                    
                    if(digit != null && digit.equals("i")) {
                        m_message.setIsToBeStored(true);
                        m_loc.play("please_hold", "");
                        m_vm.transfer(m_mailbox.getUser().getUri());
                        return null;
                    }
                    
                    LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Deposit Voicemail recorded message");
                    recorded = true ;
                }
                              
                // Confirm caller's intent for this message
        
                Menu menu = new VmMenu(m_vm);
                if (playMessage) {
                    // (pre-menu: message)
                    PromptList messagePl = new PromptList(m_loc);
                    messagePl.addPrompts(wavPath);
                    menu.setPrePromptPl(messagePl);
                    playMessage = false ; // Only play it once
                }
        
                // To play this message, press 1. To send this message, press 2. 
                // To delete this message and try again, press 3. To cancel, press *."
                pl = m_loc.getPromptList("deposit_options");
                IvrChoice choice = menu.collectDigit(pl, "123");
        
                // bad entry, timeout, canceled
                if (!menu.isOkay()) {
                    m_message.setIsToBeStored(false);
                    m_vm.goodbye();
                    return null;
                }
                            
                String digit = choice.getDigits();
                
                // "1" means play the message
                if (digit.equals("1")) {
                    playMessage = true ;
                    continue ;
                }
                
                // "2" means send the message.
                if (digit.equals("2")) {
                    if (m_message.getDuration() > 2) {
                        m_message.storeInInbox(); 
                        break ;
                    }
                    // Message was too short. Don't save the message.
                    m_message.setIsToBeStored(false);
                    
                    // "Sorry, your message was too short and was not delivered."
                    m_loc.play("msg_too_short", "");
                    continue Greeting;
                }
                
                // "3" means "erase" and re-record
                if (digit.equals("3")) {
                    recorded = false ;
                    continue ;
                }
            }
            break;
        }
    
        clearChannelUUID(m_mailbox.getUser());
        
        // "Your message has been recorded."
        m_loc.play("deposit_recorded", "");
    
        // Message sent, now see what else they want to do
        MoreOptions(m_message.getVmMessage());
        
        m_vm.goodbye();
        
        } catch (DisconnectException e) {
        } finally {
            try {
                // Let FS finish any recording's it might be doing
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
            
            clearChannelUUID(m_mailbox.getUser());

            if(m_message != null) {
                // Deliver message that is pending; don't store "click" messages
                if(m_message.getDuration() > 1) {
                    m_message.storeInInbox();
                } else {
                    m_message.deleteTempWav();
                }
            }
        }
        return null;       
    }

    /**
     * See if the caller wants to send this message to other mailboxes
     * @param existingMessage the message they want to send
     */
    private void MoreOptions(VmMessage existingMessage) {
        
        for(;;) {
            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
            PromptList pl = m_loc.getPromptList("deposit_more_options");
            VmMenu menu1 = new VmMenu(m_vm);
            menu1.setSpeakCanceled(false);
            IvrChoice choice = menu1.collectDigit(pl, "1");

            if (!menu1.isOkay()) {
                return;
            }

            String digit = choice.getDigits();
            LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" MoreOptions ("+digit+")");
            
            // Do the copy dialog
            CopyMessage.dialog(existingMessage, m_vm, m_loc);
        }
    }
    
    private Command processCPUI(String digit, PromptList prePl) {       
        Command cmdResult;
        
        CpCmd cmd = new CpCmd(m_vm, "lve_msg_immed", "lve_msg_delay", "lve_msg_help");
        if(prePl != null) {
            cmd.setPrePromptList(prePl);
        }
        
        if(digit == null) {
            cmdResult =  cmd.CollectCmd();
            if(cmdResult == Command.MSGOPTIONS) {
                return cmd.CollectMsgOptionsCmd();
            } else {
                return cmdResult;
            }
        }
 
        if(digit.equals("7")) {                             
            cmdResult = cmd.CollectMsgCmd();
            if(cmdResult == Command.MSGOPTIONS) {
                return cmd.CollectMsgOptionsCmd();
            } else {    
                if(cmdResult == Command.DELETE) {
                    return cmdResult;
                }
                
                if(cmdResult == Command.SEND) {
                    return cmdResult;
                }
                
                if(cmdResult != Command.CANCELED) {
                    m_vm.playError("bad_cmd");
                    
                }
                return Command.CANCELED;
            }
        }
        
        if(digit.equals("2)")) {
            return Command.PLAY;
        }
        
        if(digit.equals("5)")) {
            return Command.RECORD;
        }
            
        if(digit.equals("8")) {
            cmdResult = cmd.CollectMbxCmd();
            if(cmdResult == Command.LOGIN) {
                return Command.LOGIN; 
            } else {
                if(cmdResult != Command.CANCELED) {
                    m_vm.playError("bad_cmd");
                    
                }
                return Command.CANCELED;   
            }    
        }
            
        if(digit.equals("0")) {
            CpThruDial thrudial = new CpThruDial(m_vm);
            thrudial.go();
        }
    
        return Command.NONE;        
    }
}
