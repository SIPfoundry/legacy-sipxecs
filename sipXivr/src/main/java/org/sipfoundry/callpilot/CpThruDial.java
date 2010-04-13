/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callpilot;

import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.VoiceMail;

public class CpThruDial {

    private VoiceMail m_vm;
    
    public CpThruDial(VoiceMail callPilot) {
        m_vm = callPilot;
    }
        
    public void go() {
       IvrChoice choice; 
        
       // if no input in two seconds then do transfer to operator otherwise
       // check for name dialing prefix ("11") and if entered go into
       // name dialing modes
    
       CpMenu menu = new CpMenu(m_vm);
       menu.setInitialTimeout(2000);
                
       choice = menu.collectDigits(null, 12);
       
       if(choice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
           m_vm.getLoc().play("cmd_canceled", "");
           return;
       }
       
       if(choice.getIvrChoiceReason() == IvrChoiceReason.FAILURE) {
           return;
       }
       
       String digits = choice.getDigits();
       
       if(choice.getIvrChoiceReason() == IvrChoiceReason.TIMEOUT) { 
           m_vm.operator();
           return;
       }
       
       String namePrefix = ((IvrConfiguration)m_vm.getLoc().getConfig()).getCPUINameDialingPrefix();
       
       // either digits entered start with 11 and remaining digits are a name or
       // digits are an extension to transfer to. Must be a SipX user to prevent
       // toll fraud .. ie can't dial 90 to get an operator
       
       if(!digits.startsWith(namePrefix)) {           
           if(m_vm.getValidUsers().getUser(digits) == null) {
               // don't allow transfer to a non-user or external number
               m_vm.playError("invalid_extension");
               return;
           }
           
           String sipUrl = m_vm.extensionToUrl(digits); 
           m_vm.transfer(sipUrl);
       } else {         
           CpDialByName dbn = new CpDialByName(m_vm, m_vm.getConfig(), m_vm.getValidUsers());
           DialByNameChoice dbnChoice = dbn.dialByName();
           if (dbnChoice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
               return;
           }

           Vector<User> u = dbnChoice.getUsers();
           if (u != null) {
               m_vm.transfer(u.firstElement().getUri());
           }
       }
        
       return;
    }
}
