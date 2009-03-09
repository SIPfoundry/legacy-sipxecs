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

import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.Play;
import org.sipfoundry.sipxivr.PromptList;


/**
 * 
 */
public class Name {
    private String m_userDirectory;

    public Name(Mailbox mailbox) {
        super();
        m_userDirectory = mailbox.getUserDirectory();
    }
    
    /*
     * 
     */
    public String getNameFile() {
        String fileName = m_userDirectory+"name.wav";

        File nameFile = new File(m_userDirectory+"name.wav") ;
        if (nameFile.exists())
            return fileName ;
  
        return null;
    }
    
    Play getPlayer(FreeSwitchEventSocketInterface m_fses) {
        String nameFile = getNameFile();
        if (nameFile != null) {
            Play p = new Play(m_fses);
            PromptList pl = new PromptList();
            pl.addPrompts(nameFile);
            p.setPromptList(pl);
            return p ;
        } 
        return null;
    }
}
