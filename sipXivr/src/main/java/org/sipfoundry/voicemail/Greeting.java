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

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.GreetingType;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.MailboxPreferences;


/**
 * The greeting for this mailbox
 */
public class Greeting {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private VoiceMail m_vm;
    private MailboxPreferences m_MailboxPreferences;
    private String m_userDirectory;
    private Mailbox m_mailbox;

    public Greeting(VoiceMail vm) {
        m_vm = vm;
        m_mailbox = vm.getMailbox();
        m_MailboxPreferences = m_mailbox.getMailboxPreferences();
        m_userDirectory = m_mailbox.getUserDirectory();
    }
    
    public Greeting(Mailbox mailbox) {
        m_vm = null;
        m_mailbox = mailbox;
        m_MailboxPreferences = m_mailbox.getMailboxPreferences();
        m_userDirectory = m_mailbox.getUserDirectory();
    }
    
    /**
     * Get the name of the file for the particular greeting type.
     * 
     * @param type
     * @return
     */
    private String getGreetingTypeName(GreetingType type) {
        switch(type) {
        case STANDARD:
            return "standard.wav";
        case OUT_OF_OFFICE:
            return "outofoffice.wav";
        case EXTENDED_ABSENCE:
            return "extendedabs.wav";
        default:
            return null;
        }        
    }
    
    
    /**
     * Get the path to the particular greeting type.  Null if there is no such greeting.
     * @return
     */
    public String getGreetingPath(GreetingType type) {
        String filePath = getGreetingTypeName(type);

        if (filePath == null) {
            return null;
        }
    
        File greetingFile = new File(m_userDirectory, filePath);
        if (greetingFile.exists())
            return greetingFile.getPath();

        return null;
    }

    /**
     * Moves the recording to the appropriate place/name for the GreetingType.
     * 
     * @param type
     * @param recording
     */
    public void saveGreetingFile(GreetingType type, File recording) {
        String filePath = getGreetingTypeName(type);
        File greetingFile = new File (m_userDirectory, filePath);
        if (greetingFile.exists()) {
            greetingFile.delete();
        }
        recording.renameTo(greetingFile);
        ExtMailStore.SaveGreetingInFolder(m_mailbox, type, greetingFile);
    }
    
    /**
     * Generate a greeting from info we have, in place of a user recorded one.
     * 
     * @param pl  PromptList which gets the prompts added to it.
     */
    private void generateGenericGreeting(PromptList pl) {
        File nameFile = m_mailbox.getRecordedNameFile();
        if (nameFile.exists()) {
            // Has a recorded name.
            
            // {name} ...
            pl.addFragment("greeting_hasname", nameFile.getPath());
        } else {
            String mailboxName = m_mailbox.getUser().getUserName();
            if (mailboxName.length() <= 8) {
                // 8 or less characters, spell it out
                
                // The owner of extension {letters} ...
                pl.addFragment("greeting_spellout", mailboxName);
            }
            else {
                // the owner of this extension ...
                pl.addPrompts("owner.wav");
            }
        }
    }
    
    /**
     * Return the list of prompts that make up the current active greeting
     * @return
     */
    public PromptList getPromptList() {
        return getPromptList(m_MailboxPreferences.getActiveGreeting().getGreetingType());
    }
    
    /**
     * Return the list of prompts that make up the greeting type
     * @param type
     * @return
     */
    public PromptList getPromptList(GreetingType type) {
        PromptList pl = new PromptList(m_vm.getLoc());
        String greetingFile = getGreetingPath(type);
        if (greetingFile != null) {
            // A user recorded version exists, use it.
            pl.addPrompts(greetingFile);
        } else {
            // Generate a greeting from info we have
            generateGenericGreeting(pl) ;
            switch(type) {
            case NONE:
                // User never specified what to do
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Default Greeting");
                // ... is not available.  
                pl.addFragment("greeting_type_NONE");
                break;
            case STANDARD:
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Standard Greeting");
                // ... is not available.
                pl.addFragment("greeting_type_STANDARD");
                break;
            case OUT_OF_OFFICE:
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Out Of Office Greeting");
                // ... is out of the office.
                pl.addFragment("greeting_type_OUT_OF_OFFICE");
                break;
            case EXTENDED_ABSENCE:
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" Extended absence Greeting");
                // ... is on extended leave
                pl.addFragment("greeting_type_EXTENDED_ABSENCE");
                break;
            }
            // Please leave a message.
            pl.addFragment("greeting_please_leave_message");

        }
        return pl ;
    }
}
