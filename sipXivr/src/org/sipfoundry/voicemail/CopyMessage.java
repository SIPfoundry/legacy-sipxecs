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
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class CopyMessage {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    /**
     * Copy the message to a selected user's mailbox
     * @param existingMessage the message they want to copy
     */
    public static boolean dialog(VmMessage existingMessage, VoiceMail vm, Localization loc) {
        DialByNameChoice choice = EnterExtension.dialog(vm, loc);
        if (choice.getIvrChoiceReason() != IvrChoiceReason.SUCCESS) {
            return false;
        }
        
        // Store the message with each user in the list that has a mailbox
        for (User user : choice.getUsers()) {
            if (user.hasVoicemail()) {
               Mailbox otherBox = new Mailbox(user);
               existingMessage.copy(otherBox);
            }
        }
        // "Your message has been copied."
        loc.play("deposit_copied", "");
        return true;
    }
}
