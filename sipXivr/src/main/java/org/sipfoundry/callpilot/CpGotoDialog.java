/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callpilot;

import org.sipfoundry.voicemail.Messages;
import org.sipfoundry.voicemail.VmMessage;
import org.sipfoundry.voicemail.VoiceMail;

public class CpGotoDialog {

    private VoiceMail m_vm; 
 
    public CpGotoDialog(VoiceMail callPilot) {
        m_vm = callPilot;
    }
        
    public String gotoMsg() {
   
       if(m_vm.isDeposit()) {
           m_vm.playError("goto_not_allowed");
           return null;
       }    
 
       CpDialog cpDialog = new CpDialog(m_vm, "goto_immed", "goto_delay", "goto_help");
       String msgNo = cpDialog.collectDigits(5); 

       if(msgNo.length() == 0) {
           m_vm.getLoc().play("cmd_canceled", "");
           return null;
       }
       
       return msgNo;
    }
}
