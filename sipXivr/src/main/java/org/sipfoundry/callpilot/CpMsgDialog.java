package org.sipfoundry.callpilot;

import java.io.File;
import java.io.IOException;
import java.text.DateFormat;
import java.util.Calendar;
import java.util.Vector;
import org.sipfoundry.callpilot.CpCmd.Command;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.voicemail.Message;
import org.sipfoundry.voicemail.MessageDescriptor;
import org.sipfoundry.voicemail.MessageDescriptorReader;
import org.sipfoundry.voicemail.Messages;
import org.sipfoundry.voicemail.VmMessage;
import org.sipfoundry.voicemail.VoiceMail;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class CpMsgDialog{
    
    private enum NewMsgDialog {       
        COMPOSE,
        FORWARD,      
        REPLY,
        REPLYALL
    }
    
    static final int SKIPDURATION = 6; // in seconds 

    private VoiceMail m_vm;
    private VmMessage m_vmMessage;
    private Messages  m_messages;
    private MessageDescriptor m_md; 
    private Vector<User> m_recipientList; 
    private NewMsgDialog m_newMsgDlg;
    private int          m_startPos;
 
    public CpMsgDialog(VoiceMail callPilot, Messages msgs, VmMessage vmMsg) {
        m_vm = callPilot;
        m_vmMessage = vmMsg;
        m_messages = msgs;
        m_recipientList = null;
        if(vmMsg != null) {
            m_md = new MessageDescriptorReader().readObject(m_vmMessage.getDescriptorFile());
        }    
    }

    public PromptList Play(int offset) {         
        if(m_vmMessage == null) {
           m_vm.playError("nothing_to_play");
           return null;
        }

        m_messages.LoadWaveFile(m_vmMessage);                
        m_messages.markMessageHeard(m_vmMessage, true);
        
        PromptList pl = m_vm.getLoc().getPromptList();
        pl.addPrompts(m_vmMessage.getAudioFile().getPath());
                
        pl.setOffset(offset);
                
        return pl;       
    }
    
    public PromptList Header() {
        if(m_vmMessage == null) {
            return null;
        }
        
        PromptList pl = m_vm.getLoc().getPromptList();
                
        pl.addFragment("message_number", m_messages.getMsgNumber(m_vmMessage));
        
        if(isMsgDeleted()) {
            pl.addFragment("deleted");
        } else if(m_vmMessage.isUnHeard()) {
            pl.addFragment("new");
        }       
 
        if(m_md.getPriority() == Priority.URGENT) {
            pl.addFragment("this_msg_urgent");
        }
      
        return pl;       
    }
    
    private boolean isMsgDeleted() {
        return m_messages.isDeleted(m_vmMessage);
    }

    public PromptList Skip(int offsetInMs) {
        return null;
    }
 
    public PromptList PlayEnvelop() {
        if(m_vmMessage == null) {
            m_vm.playError("envelop_not_allowed");
            return null;
        }
        
        PromptList pl = m_vm.getLoc().getPromptList();
        
        if(isMsgDeleted()) {
            // play "deleted message"
            pl.addFragment("deleted_msg");
        }
                
        pl.addPrompts(datePl());
                
        pl.addPrompts(fromPl());
        
        Vector<String> otherRecipients = m_vmMessage.getMessageDescriptor().getOtherRecipients();
        if(otherRecipients != null) {
           pl.addFragment("also_to");
           for(String recipient: otherRecipients) {
               playUser(pl, recipient);
           }           
        }
        
        return pl;
    }

    public PromptList toggleDelete() {
       if(m_vmMessage == null) {
           m_vm.playError("delete_not_allowed");
           return null;
       }
        
       PromptList pl = m_vm.getLoc().getPromptList();
       
       if(!isMsgDeleted()) {           
           m_messages.deleteMessage(m_vmMessage);
           pl.addFragment("msg_num_deleted", m_messages.getMsgNumber(m_vmMessage));
       } else {
           // play msg <n> restored
           m_messages.saveMessage(m_vmMessage);
           pl.addFragment("msg_num_restored", m_messages.getMsgNumber(m_vmMessage));
       }
       return pl;
    }

    public void callSender() {
        
       if((m_vmMessage != null)) {
           MessageDescriptor md = m_vmMessage.getMessageDescriptor();
           String from = md.getFromUri();
           if(from != null && from.length() > 0) {
                // try to transfer
                m_vm.transfer(from);
                return;
            }
        }     
        m_vm.playError("call_sender_not_allowed");        
    }

    public PromptList Help() { 
        return null;
    }
    
    private User getFrom() {
        String from = ValidUsersXML.getUserPart(m_vmMessage.getMessageDescriptor().getFromUri());                
        User user = m_vm.getValidUsers().getUser(from);
        if (user != null) {
            // If user doesn't have voicemail, don't allow reply or reply all
            if (!user.hasVoicemail()) {
                user = null;
            }
        }
        return user;
    }
    
    private File recordMessageBody(File oldFile) {        
        File wavFile = null;
        
        if(oldFile != null) {
            wavFile =  oldFile;
        } else {
            try {
                // Create in the deleted directory, so if somehow things fail, they'll get removed
                // as they age out.
                wavFile = File.createTempFile("temp_recording_", ".wav", 
                        new File(m_vm.getMailbox().getDeletedDirectory()));
            } catch (IOException e) {
                throw new RuntimeException("Cannot create temp recording file", e);
            }
        }
       
        String wavPath = wavFile.getPath();
    
        m_vm.recordMessage(wavPath);
        String digit = m_vm.getLoc().getFreeSwitchEventSocketInterface().getDtmfDigit();
        if (digit == null) {
            digit = "";
        }
                 
        return wavFile;
    }     
    
    private void sendTheMessage(File msgBody, boolean isUrgent) {      
        Message message = null; 
        
        if(m_newMsgDlg == NewMsgDialog.FORWARD) {
            message = Message.newMessage(null, msgBody, 
                              m_vm.getMailbox().getUser().getUri(), 
                              isUrgent ? Priority.URGENT : Priority.NORMAL, m_recipientList);
        }
                
        for (User destUser : m_recipientList) {
            if (destUser.hasVoicemail()) {
                Mailbox destMailbox = new Mailbox(destUser);
                
                if(m_newMsgDlg != NewMsgDialog.FORWARD) {
                    if(message == null) {
                        message = Message.newMessage(destMailbox, msgBody, 
                                m_vm.getMailbox().getUser().getUri(), 
                                isUrgent ? Priority.URGENT : Priority.NORMAL, m_recipientList);
        
                        message.storeInInbox(); 
                    } else {
                        message.getVmMessage().copy(destMailbox);
                    }
                } else {
                    m_vmMessage.forward(destMailbox, message);
                }
            }
        }    
        m_vm.getLoc().play("msg_sent", "");
    }
    
    private void recordTheMessage() {
        
        PromptList prePl = null;
        File msgBody = null;
        CpCmd cmd;
        boolean isUrgent = false;
        int duration;
    
        for(;;) {        
            if(msgBody == null) {
                cmd = new CpCmd(m_vm, "compose_immed", "compose_immed", "compose_help"); 
            } else {
                cmd = new CpCmd(m_vm, null, "compose_rcd_stopped_delay", 
                                      "compose_rcd_stopped_help"); 
            }
           
            cmd.setPrePromptList(prePl);
            prePl = null;
            
            long playStart = System.currentTimeMillis();           
            Command cmdResult = cmd.CollectCmd();       
            duration =  (int) ((System.currentTimeMillis() - playStart)/1000);        
            
            switch(cmdResult) {            
            case ENVELOP:
                CpMsgDialog msgDlg = new CpMsgDialog(m_vm, m_messages, m_vmMessage);
                prePl = msgDlg.PlayEnvelop();    
                break;
                
            case CANCELED:                    
                break;
                
            case MSGOPTIONS:
                cmdResult = cmd.CollectMsgOptionsCmd();
                if(cmdResult == Command.TOGGLE_URGENCY) {
                    isUrgent = !isUrgent;
                                   
                    if(isUrgent) {
                        m_vm.getLoc().play("msg_tagged_urgent", "");
                    } else {
                        m_vm.getLoc().play("msg_tagged_normal", "");
                    } 
                }
                break;
                
            case SEND:
                // only time one can send without recording is when forwarding
                if((msgBody == null) &&  m_newMsgDlg != NewMsgDialog.FORWARD) {
                    m_vm.playError("cannot_send_emtpy_msg");  
                } else {
                    sendTheMessage(msgBody, isUrgent);
                    return;
                }
                break;                 
                
            case PLAY:
            case SKIPFORWARD:
            case SKIPBACKWARD:
                if(msgBody == null) {
                    m_vm.playError("nothing_to_play");
                } else { 
                    if (cmdResult == Command.PLAY) {
                        m_startPos = 0;                      
                    } else if(cmdResult == Command.SKIPFORWARD) {
                        m_startPos = m_startPos + duration + SKIPDURATION;
                    } else {
                        m_startPos = m_startPos + duration - SKIPDURATION;
                    }
                    
                    int durationInSec = (int) (msgBody.length()/16000);
                    
                    if(m_startPos > durationInSec) {
                        m_startPos = durationInSec;
                        if(m_startPos < 0) {
                            m_startPos = 0;
                        }
                    }
                                  
                    prePl = new PromptList(m_vm.getLoc());
                    prePl.addPrompts(msgBody.getPath());
                    prePl.setOffset(m_startPos);
                }    
                break;
                
            case RECORD:
                msgBody = recordMessageBody(msgBody);
                prePl = new PromptList(m_vm.getLoc());
                prePl.addFragment("compose_rcd_stopped_immed");                            
                break;
                
            case DELETE:
                m_vm.getLoc().play("msg_deleted", "");
                return;
                
            default:
                m_vm.playError("bad_cmd"); 
                break;
            }        
        }        
    }

    public VmMessage Compose() {
                
        if(m_vmMessage == null) {
            m_vm.getLoc().play("compose", "");
            m_newMsgDlg = NewMsgDialog.COMPOSE;
        } else {
            m_vm.getLoc().play("forward", "");
            m_newMsgDlg = NewMsgDialog.FORWARD;
        }
                
        CpAddrListDialog addrListDlg = new CpAddrListDialog(m_vm);
        m_recipientList = addrListDlg.getAddressList();
        
        if(m_recipientList.size() == 0) {
            m_vm.playError("empty_addr_list");       
            return null;
        }
       
        recordTheMessage();     
        return null;
    }

    public VmMessage Reply(boolean replyToAll) {
        
       User fromUser = getFrom();
       if(fromUser == null) {
           m_vm.playError("sender_not_a_mailbox");    
           return null;
       }

       m_recipientList = new Vector<User>();
       m_recipientList.add(fromUser);
       
       if(replyToAll) {
           m_vm.getLoc().play("replyall", "");
           m_newMsgDlg = NewMsgDialog.REPLYALL;
           
           Vector<String> otherRecipients = m_vmMessage.getMessageDescriptor().getOtherRecipients();
           if(otherRecipients != null) {
               User recipUser;
               for(String recipient : otherRecipients) {
                   // don't add otherselves to the recipients list 
                   if(recipient.equals(m_vm.getMailbox().getUser().getUserName())) {
                       continue;
                   }
                   
                   recipUser = m_vm.getValidUsers().getUser(recipient);
                   if(recipUser != null && recipUser.hasVoicemail()) {
                       m_recipientList.add(recipUser);
                   }
               }
               
           }
       } else { 
           m_vm.getLoc().play("reply", "");  
           m_newMsgDlg = NewMsgDialog.REPLY;
       }
       
       recordTheMessage();
       return null;
    }
    
    private PromptList datePl() {        
        Localization loc = m_vm.getLoc();
        DateFormat ttsDateFormat = TextToPrompts.ttsDateFormat();

        Calendar msgCal = Calendar.getInstance();
        msgCal.setTime(m_md.getTimeStampDate());
        Calendar nowCal =  Calendar.getInstance();

        long daysInMsgYear = msgCal.getMaximum(Calendar.DAY_OF_YEAR);
        long diffDays;
        long diffYears = nowCal.get(Calendar.YEAR) - msgCal.get(Calendar.YEAR);

        if (diffYears == 0) {
            // In the same year, so calculate
            // the difference in days between now and when the message was recorded
            diffDays = nowCal.get(Calendar.DAY_OF_YEAR) - msgCal.get(Calendar.DAY_OF_YEAR);
        } else {
            if (diffYears == 1) {
                // In different years, so offset by # days in the year
                diffDays = nowCal.get(Calendar.DAY_OF_YEAR) + daysInMsgYear -
                    msgCal.get(Calendar.DAY_OF_YEAR);
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
        String rxCalString = ttsDateFormat.format(msgCal.getTime());
        String fragment = "msg_info_old";

        if (diffDays < 7) {
            // within a week
            fragment = "msg_info_thisweek";
        } else if (diffYears == 0) {
            // In the same year 
            fragment = "msg_info_thisyear";
        } 

        PromptList datePl = loc.getPromptList(fragment, rxCalString);
        
        // "Message Received {date} at {time}"
        return loc.getPromptList("msg_received", datePl.toString(), rxCalString);
    }
    
    private PromptList fromPl() {
        Localization loc = m_vm.getLoc();
        
        PromptList pl = loc.getPromptList("msg_received_from");
        
        // Build the appropriate "from" based on what we know about the caller who left this message
        String from = ValidUsersXML.getUserPart(m_md.getFromUri());
        
        playUser(pl, from);
        return pl;
    }
            
    private void playUser(PromptList pl, String addr) {
        
        User user = m_vm.getValidUsers().getUser(addr);
        
        if (user != null) {
            Mailbox userMbox = new Mailbox(user);
            File nameFile = userMbox.getRecordedNameFile();
            if (nameFile.exists()) {                
                pl.addPrompts(nameFile.getPath(), user.getUserName());
            } else {
                // "at extension {extension}";
                pl.addFragment("extension", user.getUserName());
            }
        } else {
            // Call from outside.         
            if (addr != null && addr.matches("[0-9]{1,16}")) {
                // Up to 16 digits (no non-digits)
                pl.addFragment("phone_number", addr);
            } else {
                pl.addFragment("an_external_caller");
            }
        }
    }
}
