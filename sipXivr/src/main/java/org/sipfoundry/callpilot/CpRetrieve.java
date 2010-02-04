/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.callpilot;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.callpilot.CpCmd.Command;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.Messages;
import org.sipfoundry.voicemail.VmMessage;
import org.sipfoundry.voicemail.VoiceMail;

public class CpRetrieve {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    static final long MSPERDAY = 1000*60*60*24; // number of mS per day
    static final int SKIPDURATION = 6; // in seconds 
    
    VoiceMail m_vm;
    Localization m_loc;
    FreeSwitchEventSocketInterface m_fses;
    Mailbox m_mailbox;
    Messages m_messages;
    String m_userEnteredPin;        // PIN the user entered at login
    
    VmMessage m_vmMessage;
    int       m_msgNum;
    boolean   m_msgPlaying;
    int       m_startPos;
    
    VmMessage m_newVmMessage;       // message created as a result of a compose, reply,
                                    // reply all and forward command
    
    String m_ident;                 // Used to identify a particular call in the logs
    
    public CpRetrieve(VoiceMail vm) {
        m_vm = vm;
        m_loc = vm.getLoc();
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
        m_mailbox = m_vm.getMailbox();
    }
    
    public String retrieveVoiceMail() {

        if (m_mailbox != null) {
            m_ident = "Mailbox "+m_mailbox.getUser().getUserName();
        } else {
            m_ident = "Mailbox (unknown)";
        }
        LOG.info("Retrieve::retrieveVoiceMail "+m_ident+" Retrieve Voicemail from "+ m_fses.getDisplayUri());
        
        for(;;) {
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
                        
                        ArrayList<VmMessage> msgList = new ArrayList<VmMessage>(m_messages.getInbox());                        
                        msgList.addAll(m_messages.getSaved());
                        
                        PromptList statusPl = status();
                        
                        if(m_messages.noMessages()) {
                            m_vmMessage = null;      
                            m_msgNum = 0;
                        } else {
                            m_vmMessage = msgList.get(0); 
                            m_msgNum = Integer.parseInt(m_messages.getMsgNumber(m_vmMessage));
                        }
                        processCmds(statusPl);
                        
                    } else {
                        // Those without (thus are just in the directory), get to record their name
                        LOG.info("Retrieve::retrieveVoiceMail:recordName "+m_ident);
                        CpGrtAdminDialog cpDialog = new CpGrtAdminDialog(m_vm, LOG);
                        cpDialog.rcdSpnName();
                        m_vm.goodbye();
                    }
                } finally {
                    // empty the deleted messages folder
                    m_messages.destroyDeletedMessages();                                  
                    Messages.releaseMessages(m_messages);
                    
                    deleteAllTempFiles();
                }
            } else {
                break;
            }
        }
        return null;
    }
    
    /**
     * Login to the mailbox.
     * 
     */
    private User login() {
        int errorCount = 0;
        User user = null;
        boolean firstTime = true;
        CpDialog dlg;

        if (m_mailbox != null) {
            user = m_mailbox.getUser();
        }
        
        for(;;) {
            if(firstTime) {
                dlg = new CpDialog(m_vm, "login_mailbox", null, "login_delay_help");
            } else {
                dlg = new CpDialog(m_vm, null, null, "login_delay_help");
            }
            
            firstTime = false;          
                        
            String mbxString = dlg.collectDigits(18);
            if(mbxString == null) {
                return null;
            }
            
            if(mbxString.length() == 0) {
                // user entered a single # sign
                mbxString = m_fses.getFromUser();
            }

            // See if the user exists
            user = m_vm.getValidUsers().getUser(mbxString);         

            // "Enter your personal identification number, and then press #.":
            // "To log in as a different user, press #"
            dlg = new CpDialog(m_vm, "password", null, "please_enter_password");
            String password = dlg.collectDigits(18); 
            
            // Only users with voicemail, or are in the directory, are allowed
            // (directory users can record their name, that's all)
            if (user != null && !user.hasVoicemail() && !user.isInDirectory() ) {
                LOG.info("Retrieve::login user "+user.getUserName()+" doesn't have needed permissions.");
                user = null ;
            }
            
            if (user == null || !user.isPinCorrect(password, m_loc.getConfig().getRealm())) {
                // Wrong mbx/password combination
               
                if(++errorCount > 2) {
                    m_loc.play("login_incorrect", "");
                    m_loc.play("contact_admin", "");
                    m_vm.goodbye();
                    return null;
                }
                // "That personal identification number is not valid
                m_loc.play("login_incorrect_again", "");
                continue;
            }
            m_userEnteredPin = password;
        
            return user;
        }
    }
    
    /**
     * Collect the mailbox status prompts
     * 
     *  like "You have 2 new messages, both of them are urgent"
     *
     * @return
     */
    PromptList status() {
        PromptList pl = m_loc.getPromptList();
            
        int unheard = m_messages.getUnheardCount();
        
        // You have X new messages
        if(unheard < 21) {
            pl.addFragment("have_new_messages_" + String.valueOf(unheard));
        } else { 
            pl.addFragment("have_new_messages_lots"); 
        }    
        
        int newUrgent = m_messages.getUrgentCount();
        
        if(newUrgent > 0) {
            if(newUrgent == unheard) {
                if(newUrgent == 1) {
                    pl.addFragment("it_is_urgent"); 
                } else if(newUrgent == 2) {
                    pl.addFragment("both_are_urgent"); 
                } else {
                    pl.addFragment("all_are_urgent"); 
                }             
            } else if(newUrgent < 4) {
                int i = 0;
                VmMessage msg = m_messages.getMsgByNumber(0);
                while(msg != null) {
                    if(msg.isUrgent()) {
                        pl.addFragment("urgent_message", Integer.toString(i));
                    }
                    msg = m_messages.getMsgByNumber(++i);
                }    
                
            } else {
                pl.addFragment("urgent_messages_" + String.valueOf(newUrgent)); 
            }
        }
        
        int unsent = 0;
        if(unsent == 1) {
            pl.addFragment("one_unsent_message");
        } else if(unsent == 2) {
            pl.addFragment("two_unsent_messages");
        } else if(unsent > 0) {
            pl.addFragment("many_messages");
        }
         
        return pl;
    }
        
    private void nextMessage() {
        if(m_messages.noMessages()) {
            // do nothing
            return;
        }
        
        m_msgPlaying = false;
        m_startPos = 0;
        
        if(m_vmMessage == null) {
            // at the start or end of the mailbox
            // try to go to the first msg in the mailbox
            m_msgNum = 1;
        } else { 
            m_msgNum++;  
        }    
            
        m_vmMessage = m_messages.getMsgByNumber(m_msgNum);       
    }
    
    private void previousMessage() {
        if(m_messages.noMessages()) {
            // do nothing
            return;
        }
        
        m_msgPlaying = false;
        m_startPos = 0;
        
        if(m_msgNum == 0) {
            // we were at the start of the mailbox now go to the last message
            m_msgNum = m_messages.getLastMsgNumber();
            m_vmMessage = m_messages.getMsgByNumber(m_msgNum);        
        } else {
            m_msgNum--;
            if(m_msgNum == 0) {
                // was at the very first message
                m_vmMessage = null; 
            } else {
                m_vmMessage = m_messages.getMsgByNumber(m_msgNum);        
            }     
        }
    }
    
    private void gotoMessage(int msgNum) {
        
        VmMessage themsg = m_messages.getMsgByNumber(msgNum);
        
        if(themsg == null) {
            m_vm.playError("invalid_number");
        } else {
            m_msgPlaying = false;
            m_startPos = 0;
            m_msgNum = msgNum;
            m_vmMessage = themsg;
        }      
    }
    
    private CpCmd CreateCmdDialog() {         
        if(m_messages.noMessages()) {
            CpCmd cmd = new CpCmd(m_vm, "mbx_empty_help", "to_compose_msg", "mbx_empty_help"); 
            return cmd;
        }
        
        // perhaps at end of mailbox
        if((m_vmMessage == null) && m_msgNum > 0 ) { 
            CpCmd cmd = new CpCmd(m_vm, "end_of_mailbox_immed", "end_of_mailbox_delay", "end_of_mailbox_help"); 
            return cmd;
        }
        
        // perhaps at start of mailbox
        if((m_vmMessage == null) && m_msgNum == 0 ) {        
            CpCmd cmd = new CpCmd(m_vm, "start_of_mailbox_immed", "start_of_mailbox_delay", "start_of_mailbox_help"); 
            return cmd;
        }
        
        // at start, middle or end of a message
        if(m_vmMessage != null) {
                       
            if(m_msgPlaying) {
                CpCmd cmd = new CpCmd(m_vm, "end_of_msg_immed", "end_of_msg_delay", "end_of_msg_help");
                cmd.midMessagePlayBackPrompts("middle_of_msg_delay", 
                                               "middle_of_msg_help",
                                               m_vmMessage.getDuration()*1000);
                return cmd;
            }
            
            // must be at the start of the message
            CpCmd cmd = new CpCmd(m_vm, null, "start_of_msg_delay", "start_of_msg_help"); 
            return cmd;
        }
        
        return null;
    }
    
    private void processCmds(PromptList statusPl) {
        CpGrtAdminDialog grtDlg;
        CpMsgDialog msgDlg;
        VmMessage currMsg;
        PromptList prePl = null;    
        int duration = 0;
        
        m_startPos = 0;
        m_msgPlaying = false;
    
        for(;;) {                                                   
            if(m_newVmMessage != null) {
                currMsg = m_newVmMessage;
            } else {
                currMsg = m_vmMessage;
            }
            
            if(statusPl != null) {     
                if(currMsg != null) {
                    msgDlg = new CpMsgDialog(m_vm, m_messages, currMsg);
                    
                    prePl = statusPl;
                    prePl.addPrompts(msgDlg.Header());
                }             
                statusPl = null;
            } else {
                m_messages.update();
            }
                                    
            CpCmd cmd = CreateCmdDialog();    
            cmd.setPrePromptList(prePl);            
            prePl = null;
            
            long playStart = System.currentTimeMillis();           
            Command result = cmd.CollectCmd();           
            duration =  (int) ((System.currentTimeMillis() - playStart)/1000);      
            
            switch(result) {          
                case PLAY: 
                case SKIPFORWARD:                        
                case SKIPBACKWARD:      
                    msgDlg = new CpMsgDialog(m_vm, m_messages, currMsg);
                    
                    if (result == Command.PLAY) {
                        m_startPos = 0;                      
                    } else if(result == Command.SKIPFORWARD) {
                        m_startPos = m_startPos + duration + SKIPDURATION;
                    } else {
                        m_startPos = m_startPos + duration - SKIPDURATION;
                    }
                    
                    if(m_vmMessage != null) {                      
                        if(m_startPos >= m_vmMessage.getDuration()) {
                            m_startPos = (int) m_vmMessage.getDuration() - 1;
                            if(m_startPos < 0) {
                                m_startPos = 0;
                            }
                        }
                    }
                    
                    prePl = msgDlg.Play(m_startPos);
                    
                    if(prePl != null) {
                        m_msgPlaying = true;
                    }
                    break;
                                
                case CALLSENDER:
                    msgDlg = new CpMsgDialog(m_vm, m_messages, currMsg);
                    msgDlg.callSender();
                    break;
    
                case DELETE:               
                    msgDlg = new CpMsgDialog(m_vm, m_messages, currMsg);                    
                    prePl = msgDlg.toggleDelete();
                    
                    nextMessage();
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    
                    PromptList hdr = msgDlg.Header();
                    if(hdr != null) {
                        if (prePl != null) {
                            prePl.addPrompts(hdr);
                        } else {
                            prePl = hdr;
                        }                      
                    }                                    
                    break;
                                    
                case REVERT:
                    CpThruDial thrudial = new CpThruDial(m_vm);
                    thrudial.go();
                    break;
                    
                case PREVIOUS:
                    previousMessage();
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    prePl = msgDlg.Header();
                    break;
                    
                case NEXT:
                    nextMessage();
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    prePl = msgDlg.Header();
                    break;
                    
                case RECORD:
                    if(m_vmMessage != null) {
                        m_vm.playError("record_not_allowed_incoming"); 
                    } else {
                        m_vm.playError("record_not_allowed"); 
                    }
                    break;                  
                    
                case MSGOPTIONS:                    
                    if(m_newVmMessage != null) {
                        result = cmd.CollectMsgOptionsCmd();
                        if(result == Command.TOGGLE_URGENCY) {
                            m_newVmMessage.toggleUrgency();
                            if(m_newVmMessage.isUrgent()) {
                                m_loc.play("msg_tagged_urgent", "");
                            } else {
                                m_loc.play("msg_tagged_normal", "");
                            }
                        } 
                    } else { 
                        if(m_vmMessage == null) {
                            m_vm.playError("msg_options_not_allowed");
                        } else {
                            if(m_messages.isDeleted(m_vmMessage)) {
                                m_vm.playError("cannot_tag_deleted");
                            } else { 
                                m_vm.playError("cannot_tag_incoming");
                            }
                        }
                    }
                    break;
                    
                case LOGIN:
                    return;
                    
                case REC_SPKNAME:
                    CpGrtAdminDialog spkNameDlg = new CpGrtAdminDialog(m_vm, LOG);
                    spkNameDlg.rcdSpnName();
                    break;
                    
                case REC_GREETING:
                    grtDlg = new CpGrtAdminDialog(m_vm, LOG);
                    grtDlg.rcdAGreeting();
                    break;
                    
                case CHOOSE_GREETING:
                    grtDlg = new CpGrtAdminDialog(m_vm, LOG);
                    grtDlg.chooseAGreeting();
                    break;                               
                          
                case PWDCHANGE:
                    CpPwdAdminDialog pwdChg = new CpPwdAdminDialog(m_vm, LOG, m_userEnteredPin);
                    String newPin= pwdChg.chngPswd();
                    if(newPin != null) {
                        m_userEnteredPin = newPin;
                    }
                    break;
                    
                case TOOLS:
                    // CpTools cptools = new CpTools(m_vm);
                    // cptools.go();
                    break;
                    
                case GOTO:
                    if(m_messages.noMessages()) {
                        // mailbox must be empty                     
                        m_vm.playError("goto_not_allowed");
                    } else {
                        CpGotoDialog gotoDlg = new CpGotoDialog(m_vm);
                        String msgNum = gotoDlg.gotoMsg();
                        gotoMessage(Integer.parseInt(msgNum));
                        
                        msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                        prePl = msgDlg.Header();
                    }    
                    break;                 
                    
                case COMPOSE:
                    msgDlg = new CpMsgDialog(m_vm, m_messages, null);
                    m_newVmMessage = msgDlg.Compose();                                        
                    break;                   
                    
                case REPLY:     
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    m_newVmMessage = msgDlg.Reply(false);
                    break;
                    
                case REPLYALL:
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    m_newVmMessage = msgDlg.Reply(true);
                    break;
                    
                case FORWARD:     
                    msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                    m_newVmMessage = msgDlg.Compose();
                    break;
                    
                case SEND:
                    if(m_newVmMessage == null) {
                        m_vm.playError("send_not_allowed");      
                    }    
                    break;
                    
                case CANCELED:
                    break;                   
                    
                case ENVELOP:
                    msgDlg = new CpMsgDialog(m_vm, m_messages, currMsg);
                    prePl = msgDlg.PlayEnvelop();
                    break;
                    
                case PERSONAL_ATTENDANT:
                    CpAttdAdminDialog attdAdminDlg = new CpAttdAdminDialog(m_vm, m_userEnteredPin);
                    attdAdminDlg.chngAttd();
                    break;
            }
        }               
    }
    
    private void deleteAllTempFiles() {
        
        File path = new File(m_vm.getMailbox().getDeletedDirectory());
        
        File[] files = path.listFiles();
        for(int i=0; i<files.length; i++) {
            FileUtils.deleteQuietly(files[i]);
        }
    }
}
  
