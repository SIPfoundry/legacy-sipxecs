/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callpilot;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.RestfulRequest;
import org.sipfoundry.voicemail.VoiceMail;

public class CpPwdAdminDialog {

    private VoiceMail m_vm;
    private String m_currPswd;
    private Logger m_log;
    
    public CpPwdAdminDialog(VoiceMail callPilot, Logger log, String currPswd) {
        m_vm = callPilot;
        m_currPswd = currPswd;
        m_log = log;
    }
        
    public String chngPswd() {
    
       CpDialog cpDialog = new CpDialog(m_vm, "new_password");
       String newPswd = cpDialog.collectDigits(9);

       if(newPswd.length() == 0) {  
           return null;
       } 

       // TODO validated length and play "password_invalid, how short can it be?????"

       cpDialog = new CpDialog(m_vm, "new_password_again");
       String newPswdAgain = cpDialog.collectDigits(9);
        
       if(newPswdAgain.equals(newPswd)) {
           // Use sipXconfig's RESTful interface to change the PIN
           try {
               RestfulRequest rr = new RestfulRequest(
                   ((IvrConfiguration)m_vm.getLoc().getConfig()).getConfigUrl()+"/sipxconfig/rest/my/voicemail/pin/", 
                   m_vm.getMailbox().getUser().getUserName(), m_currPswd);
               if (rr.put(newPswd)) {
                   m_log.info("Retrieve::voicemailOptions:changePin "+
                           m_vm.getMailbox().getUser().getUserName()
                             + " Pin changed.");
        
                   m_vm.getLoc().play("password_changed", "");
                   return newPswd;
               }
               
               m_log.error("Retrieve::voicemailOptions new pin trouble "+rr.getResponse());
           } catch (Exception e) {
               m_log.error("Retrieve::voicemailOptions new pin trouble", e);
           }

       } else {
           m_vm.playError("passwords_dont_match");
       }
       return null;
    }
}
