/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callpilot;

import java.io.File;
import java.util.Vector;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.ApplicationConfiguraton;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.VoiceMail;

public class CpDialByName extends DialByName {

    private VoiceMail m_vm;
    
    CpDialByName(VoiceMail vm, ApplicationConfiguraton config, ValidUsersXML validUsers) {
        super(vm.getLoc(), config, validUsers);
        m_vm = vm;
        m_loc = vm.getLoc();
    }

    protected DialByNameChoice selectChoice(String digits) {
        Vector<User> matches = m_validUsers.lookupDTMF(digits, true);
        CpDialog dlg;

        if (matches.size() == 0) {
            // Indicate no match: "No name in the directory matches the name you entered."
            m_loc.play("no_matches_found", "");
            return new DialByNameChoice(new IvrChoice(digits, IvrChoiceReason.SUCCESS));
        }
        
        int choices = matches.size();
        int i = 0;
        
        for (;;) {
            dlg = new CpDialog(m_vm, "accept_selection");

            User u = matches.get(i);
            
            // Try to speak the user's recorded name
            File nameFile = new Mailbox(u).getRecordedNameFile();
            String namePrompts;
            if (nameFile.exists()) {
                namePrompts = nameFile.getPath();
            } else {
                PromptList ext = new PromptList(m_loc);
                // "Extension {extension}"
                ext.addFragment("extension", u.getUserName());
                namePrompts = ext.toString();
            }
            
            PromptList prePl = new PromptList(m_loc);            
            prePl.addFragment("a_prompt", namePrompts);
            
            dlg.setPrePromptList(prePl);
            
            String digit = dlg.collectDigit("*#1");
            
            if(digit.equals("#")) {
                return new DialByNameChoice(u, digit, IvrChoiceReason.SUCCESS);
            }
            
            if(digit.equals("1")) {
                i = (i + 1) % choices;
                continue;
            }
            
            if(digit.equals("*")) {
                m_loc.play("canceled", "");
                return new DialByNameChoice(new IvrChoice("*", IvrChoiceReason.CANCELED));
            }      
        }
    }    
}
