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
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class EnterExtension {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    /**
     * The Enter Extension dialog
     * Lets the user enter an extension which is validated against the ValidUsers list,
     * and also must have voicemail permissions.
     * Or they can enter a 9 and use dial by name spelling to find the extension.
     * Or they can enter an 8 and select a distribution list to select several extensions.
     * 
     * @return The user's choice
     */
    public static DialByNameChoice dialog(VoiceMail vm, Localization loc) {
        Vector<User> userList = new Vector<User>() ;
        
        for(;;) {
            // "Please dial an extension."
            // "Press 8 to use a distribution list,"
            // "Or press 9 for the dial by name directory."
            PromptList pl = loc.getPromptList("dial_extension");
            VmMenu menu = new VmMenu(vm);
            IvrChoice choice = menu.collectDigits(pl, 10);
            
            if (!menu.isOkay()) {
                return new DialByNameChoice(choice);
            }
            
            String digits = choice.getDigits();
            LOG.info("EnterExtension::extensionDialog collected ("+digits+")");
            
            if (digits.equals("8")) {
                Vector<User> users = vm.selectDistributionList();
                if (users == null) {
                    continue;
                }
                userList.addAll(users);
                break ;
                
            } else if (digits.equals("9")) {
                // Do the DialByName dialog
                DialByName dbn = new DialByName(loc, vm.getConfig(), vm.getValidUsers());
                dbn.setOnlyVoicemailUsers(true);
                DialByNameChoice dbnChoice = dbn.dialByName();
                
                // If they canceled DialByName, backup
                if (dbnChoice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                    continue ;
                }
                
                // If an error occurred, failure
                if (dbnChoice.getUsers() == null) {
                    vm.failure();
                    return new DialByNameChoice(dbnChoice);
                }
                
                // Add the selected user to the list
                userList.addAll(dbnChoice.getUsers());
                break ;
            } else {
                User user = vm.getValidUsers().getUser(digits);
                if (user == null || !user.hasVoicemail()) {
                    // "that extension is not valid"
                    loc.play("invalid_extension", "");
                    continue ;
                }
                userList.add(user) ;
                break ;
            }
        }
        return new DialByNameChoice(userList, "", IvrChoiceReason.SUCCESS);
    }
}
