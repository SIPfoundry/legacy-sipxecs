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
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.voicemail.mailbox.GreetingType;

/**
 * The greeting for this mailbox
 */
public class Greeting {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private User m_user;
    private File m_name;

    public Greeting(User user, File recordedName) {
        m_user = user;
        m_name = recordedName;
    }

    /**
     * Generate a greeting from info we have, in place of a user recorded one.
     * 
     * @param pl PromptList which gets the prompts added to it.
     */
    private void generateGenericGreeting(PromptList pl) {
        if (m_name.exists()) {
            // Has a recorded name.

            // {name} ...
            pl.addFragment("greeting_hasname", m_name.getPath());
        } else {
            String mailboxName = m_user.getUserName();
            if (mailboxName.length() <= 8) {
                // 8 or less characters, spell it out

                // The owner of extension {letters} ...
                pl.addFragment("greeting_spellout", mailboxName);
            } else {
                // the owner of this extension ...
                pl.addPrompts("owner.wav");
            }
        }
    }

    /**
     * Return the list of prompts that make up the greeting type
     * 
     * @param type
     * @return
     */
    public PromptList getPromptList(PromptList pl, GreetingType type, String greetingFile, boolean playDefaultVmOption) {
        if (greetingFile != null) {
            // A user recorded version exists, use it.
            pl.addPrompts(greetingFile);
        } else {
            // Generate a greeting from info we have
            generateGenericGreeting(pl);
            switch (type) {
            case NONE:
                // User never specified what to do
                LOG.info("Mailbox " + m_user.getUserName() + " Default Greeting");
                // ... is not available.
                pl.addFragment("greeting_type_NONE");
                break;
            case STANDARD:
                LOG.info("Mailbox " + m_user.getUserName() + " Standard Greeting");
                // ... is not available.
                pl.addFragment("greeting_type_STANDARD");
                break;
            case OUT_OF_OFFICE:
                LOG.info("Mailbox " + m_user.getUserName() + " Out Of Office Greeting");
                // ... is out of the office.
                pl.addFragment("greeting_type_OUT_OF_OFFICE");
                break;
            case EXTENDED_ABSENCE:
                LOG.info("Mailbox " + m_user.getUserName() + " Extended absence Greeting");
                // ... is on extended leave
                pl.addFragment("greeting_type_EXTENDED_ABSENCE");
                break;
            }
            // Please leave a message.
            pl.addFragment("greeting_please_leave_message");
        }

        if (playDefaultVmOption) {
            pl.addFragment("VoiceMail_options");
        }

        return pl;
    }
}
