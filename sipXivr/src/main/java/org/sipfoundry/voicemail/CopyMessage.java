/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class CopyMessage {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private VoiceMail m_vm;
    private Localization m_loc;

    public CopyMessage(VoiceMail vm, Localization loc) {
        m_vm = vm;
        m_loc = loc ;
    }
    
    /**
     * Copy the message to a selected user's mailbox
     * @param existingMessage the message they want to copy
     */
    public boolean copyDialog(VmMessage existingMessage) {
        EnterExtension ee = new EnterExtension(m_vm, m_loc);
        DialByNameChoice choice = ee.extensionDialog();
        if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
            return false;
        }
        
        // Store the message with each user in the list
        for (User user : choice.getUsers()) {
            Mailbox otherBox = new Mailbox(user, m_loc);
           existingMessage.copy(otherBox);
        }
        // "Your message has been copied."
        m_loc.play("deposit_copied", "");
        return true;
    }
}
