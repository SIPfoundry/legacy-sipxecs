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
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.Menu;
import org.sipfoundry.sipxivr.PersonalAttendant;
import org.sipfoundry.sipxivr.RemoteRequest;
import org.sipfoundry.sipxivr.RestfulRequest;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class Deposit {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private VoiceMail m_vm;
    private Localization m_loc;
    private FreeSwitchEventSocketInterface m_fses;
    private Mailbox m_mailbox;
    
    // maps username to freeswitch channel UUID
    private static Map<String, String> m_depositMap = Collections.synchronizedMap(new HashMap<String, String>());

    public static String getChannelUUID(User user) {
        return m_depositMap.get(user.getUserName());
    }
    
    private void sendIM(User user, String instantMsg) {
        URL sendIMUrl;
        try {
            sendIMUrl = new URL(IvrConfiguration.get().getSendIMUrl() + "/" +
                                    user.getUserName() + "/SendIM");
            
            RemoteRequest rr = new RemoteRequest(sendIMUrl, "text/plain", instantMsg);
            if (!rr.http()) {
                LOG.error("Deposit::sendIM Trouble with RemoteRequest "+ rr.getResponse());
            }
            
        } catch (MalformedURLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } 
          
    }
    
    
    private void putChannelUUID(User user, String uuid) {
        m_depositMap.put(user.getUserName(), uuid);     
        
        sendIM(m_mailbox.getUser(), m_fses.getVariable("channel-caller-id-name") +
        " is leaving a message.");
    }
    
    private void clearChannelUUID(User user) {
        if(m_depositMap.remove(user.getUserName()) != null) {
            sendIM(m_mailbox.getUser(), m_fses.getVariable("channel-caller-id-name") +
            " is no longer leaving a message.");
        }
    }
    
    public Deposit(VoiceMail vm) {
        m_vm = vm;
        m_loc = vm.getLoc();
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
        m_mailbox = vm.getMailbox();
    }
    
    /**
     * The depositVoicemail dialog 
     * 
     * @return
     */
    public String depositVoicemail() {
        try {
        
        String displayUri = m_fses.getDisplayUri();
        LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Deposit Voicemail from "+displayUri);
        
        PersonalAttendant pa = m_mailbox.getPersonalAttendant();
        String localeString = pa.getLanguage();
        if (localeString != null) {
            LOG.debug("Changing locale for this call to "+localeString);
            m_loc.changeLocale(localeString);
        }
        
        Message message = null;
        String wavPath = null;
        File wavFile = null;
        
        putChannelUUID(m_mailbox.getUser(), 
                       m_fses.getVariable("channel-unique-id"));
        
        Greeting:
        for(;;) {
            // {user's greeting}
            Greeting greeting = new Greeting(m_vm) ;
            PromptList pl = greeting.getPromptList(); ;
        
            // When you are finished, press 1 for more options.
            // To reach the operator, dial 0 at any time.
            pl.addFragment("VoiceMail_options");
            
            // Allow caller to barge with 0, *, and any defined Personal Attendant digit
            // Also, allow barge to recording with "#"
        
            m_loc.play(pl, "#0*i"+pa.getValidDigits());
        
            Collect c = new Collect(m_fses, 1, 100, 0, 0);
            c.setTermChars("#");
            c.go();
            String digits = c.getDigits();  
            LOG.info("depositVoicemail Collected digits=" + digits);
        
            if (digits.equals("*")) {
                return "retrieve";
            }
            
            if(digits.equals("i")) {
                m_loc.play("please_hold", "");
                String uri = m_mailbox.getUser().getUri();
                m_vm.transfer(uri);
                return null;
            }
            
            // See if the digit they pressed was defined in the Personal Attendant
            String transferUrl = null;
            if (digits.equals("0")) {
                transferUrl = m_vm.getOperator(pa);
            } else {
                // See if the Personal Attendant defined that digit to mean anything
                transferUrl = pa.getMenuValue(digits) ;
            }
            
            if (transferUrl != null) {
                LOG.info(String.format("Transfer to %s", transferUrl));
                m_vm.transfer(transferUrl);
                return null ;
            }
            
            if (message == null) {
                try {
                    // Create in the deleted directory, so if somehow things fail, they'll get removed
                    // as they age out.
                    wavFile = File.createTempFile("temp_recording_", ".wav",
                            new File(m_mailbox.getDeletedDirectory()));
                    wavPath = wavFile.getPath();
                } catch (IOException e) {
                    throw new RuntimeException("Cannot create temp recording file", e);
                }
                
                message = Message.newMessage(m_mailbox, wavFile, displayUri, Priority.NORMAL);
                m_vm.getMessages().add(message) ;
            }
            
            boolean recorded = false ;
            boolean playMessage = false;
            for(;;) {
                // Record the message
                if (!recorded) {
                    message.setIsToBeStored(true);  // So if they hang up now, we'll save what we got.
                    m_vm.recordMessage(wavPath);
        
                    String digit = m_fses.getDtmfDigit();
                    if (digit != null && digit == "0") {
                        // Don't save the message.
                        message.setIsToBeStored(false);
                        m_vm.transfer(m_vm.getOperator(pa));
                        return null;
                    }
                    
                    if(digit != null && digit.equals("i")) {
                        message.setIsToBeStored(true);
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
        
                // To play this message, press 1.  
                // To send this message, press 2. 
                // To delete this message and try again, press 3.  
                // To cancel, press *."
                pl = m_loc.getPromptList("deposit_options");
                IvrChoice choice = menu.collectDigit(pl, "123");
        
                // bad entry, timeout, canceled
                if (!menu.isOkay()) {
                    message.setIsToBeStored(false);
                    return null;
                }
                            
                String digit = choice.getDigits();
        
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Deposit Voicemail options ("+digit+")");
                
                // "1" means play the message
                if (digit.equals("1")) {
                    LOG.info(String.format("Playing back message (%s)", wavPath));
                    playMessage = true ;
                    continue ;
                }
                
                // "2" means send the message.
                if (digit.equals("2")) {
                    if (message.getDuration() > 2) {
                        message.storeInInbox(); 
                        break ;
                    }
                    // Message was too short (why do we care?)
                    // Don't save the message.
                    message.setIsToBeStored(false);
                    
                    // "Sorry, your message was too short and was not delivered."
                    // "Please record again."
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
        MoreOptions(message.getVmMessage());
        
        } catch (DisconnectException e) {
            clearChannelUUID(m_mailbox.getUser());
            throw e;
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
}
