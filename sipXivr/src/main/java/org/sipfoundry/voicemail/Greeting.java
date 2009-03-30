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
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.MailboxPreferences;
import org.sipfoundry.sipxivr.PromptList;


/**
 * The greeting for this mailbox
 */
public class Greeting {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private MailboxPreferences m_MailboxPreferences;
    private String m_userDirectory;
    private Mailbox m_mailbox;

    public Greeting(Mailbox mailbox) {
        m_mailbox = mailbox;
        m_MailboxPreferences = mailbox.getMailboxPreferences();
        m_userDirectory = mailbox.getUserDirectory();
    }
    
    private String getActiveGreetingFile() {
        String fileName = null;
        
        switch(m_MailboxPreferences.getActiveGreeting()) {
        case STANDARD:
            fileName = "standard.wav";
            break;
        case OUT_OF_OFFICE:
            fileName = "outofoffice.wav";
            break;
        case EXTENDED_ABSENCE:
            fileName = "extendedabs.wav";
            break;
        default:
            return null;
        }
        File greetingFile = new File(m_userDirectory + fileName);
        if (greetingFile.exists())
            return m_userDirectory + fileName;

        return null;
    }

    /**
     * Generate a greeting from info we have, in place of a user recorded one.
     * 
     * @param pl
     */
    private void generateGenericGreeting(PromptList pl) {
        String nameFileName = m_mailbox.getRecordedName();
        if (nameFileName != null) {
            // Has a recorded name.
            
            // {name} ...
            pl.addFragment("greeting_hasname", nameFileName);
        } else {
            String mailboxName = m_mailbox.getUser().getUserName();
            if (mailboxName.length() <= 6) {
                // 6 or less characters, spell it out
                
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
     * Return the list of prompts that make up this greeting
     * @return
     */
    public PromptList getPromptList() {
        
        PromptList pl = new PromptList(m_mailbox.getLocalization());
        String activeGreetingFile = getActiveGreetingFile();
        if (activeGreetingFile != null) {
            // A user recorded version exists, use it.
            pl.addPrompts(activeGreetingFile);
        } else {
            // Generate a greeting from info we have
            generateGenericGreeting(pl) ;
            switch(m_MailboxPreferences.getActiveGreeting()) {
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
