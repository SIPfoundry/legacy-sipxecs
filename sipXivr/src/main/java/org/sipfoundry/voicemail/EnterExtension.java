/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class EnterExtension {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private VoiceMail m_vm;
    private Localization m_loc;

    public EnterExtension(VoiceMail vm, Localization loc) {
        m_vm = vm;
        m_loc = loc ;
    }
    
    public DialByNameChoice extensionDialog() {
        Vector<User> userList = new Vector<User>() ;
        
        for(;;) {
            // "Please dial an extension."
            // "Press 8 to use a distribution list,"
            // "Or press 9 for the dial by name directory."
            PromptList pl = m_loc.getPromptList("dial_extension");
            VmMenu menu = new VmMenu(m_loc, m_vm);
            IvrChoice choice = menu.collectDigits(pl, 10);
            
            if (!menu.isOkay()) {
                return new DialByNameChoice(choice);
            }
            
            String digits = choice.getDigits();
            LOG.info("EnterExtension::extensionDialog collected ("+digits+")");
            
            if (digits.equals("8")) {
                Vector<User> users = m_vm.selectDistributionList();
                if (users == null) {
                    continue;
                }
                userList.addAll(users);
                break ;
                
            } else if (digits.equals("9")) {
                // Do the DialByName dialog
                DialByName dbn = new DialByName(m_loc, m_vm.getConfig(), m_vm.getValidUsers());
                DialByNameChoice dbnChoice = dbn.dialByName();
                
                // If they canceled DialByName, backup
                if (dbnChoice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                    continue ;
                }
                
                // If an error occurred, failure
                if (dbnChoice.getUsers() == null) {
                    m_vm.failure();
                    return new DialByNameChoice(dbnChoice);
                }
                
                // Add the selected user to the list
                userList.addAll(dbnChoice.getUsers());
                break ;
            } else {
                User user = m_vm.getValidUsers().isValidUser(digits);
                if (user == null) {
                    // "that extension is not valid"
                    m_loc.play("invalid_extension", "");
                    continue ;
                }
                userList.add(user) ;
                break ;
            }
        }
        return new DialByNameChoice(userList, "", IvrChoiceReason.SUCCESS);
    }
}
