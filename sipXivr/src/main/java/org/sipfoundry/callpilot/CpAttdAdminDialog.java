package org.sipfoundry.callpilot;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.RestfulRequest;
import org.sipfoundry.voicemail.VoiceMail;

public class CpAttdAdminDialog {

    private VoiceMail m_vm;
    private String    m_currPswd;
    
    public CpAttdAdminDialog(VoiceMail callPilot, String currPswd) {
        m_vm = callPilot;
        m_currPswd = currPswd;
    }
            
    public void chngAttd() {
        
       CpDialog cpDialog;
       String attdExt;
       PromptList pl;
       
       String transferUrl = m_vm.getMailbox().getPersonalAttendant().getOperator(); 
       if(transferUrl != null) {
           attdExt = ValidUsersXML.getUserPart(transferUrl);
       } else {
           attdExt = "";
       }
       
       for(;;) {
           pl = m_vm.getLoc().getPromptList();
           
           if(attdExt.equals("")) {
               pl.addFragment("is_system_attendant");
           } else {
               pl.addFragment("is_personal_attendant", attdExt);
           }
        
           cpDialog = new CpDialog(m_vm, "to_change_attd");
           cpDialog.setPrePromptList(pl);
           
           String cmd = cpDialog.collectDigit("1*");
           
           if(cmd.equals("*")) {
               break;
           }
           
           cpDialog = new CpDialog(m_vm, "attd_change");          
           attdExt = cpDialog.collectDigits(9);
    
           if(attdExt.length() == 0) {  
               return;
           } 
           
           if(attdExt.equals("0")) {
               attdExt = "";   
           }
    
           // Use sipXconfig's RESTful interface to change the attendant extension
           try {
               RestfulRequest rr = new RestfulRequest(
                   ((IvrConfiguration)m_vm.getLoc().getConfig()).getConfigUrl()+"/sipxconfig/rest/my/voicemail/operator/", 
                   m_vm.getMailbox().getUser().getUserName(), m_currPswd);
               if (!rr.put(attdExt)) {  
                   m_vm.playError("command_failed");
                   break;
               }
               
           } catch (Exception e) {
               m_vm.playError("command_failed");
               break;
           }
           if(attdExt.length() == 0) {
               m_vm.getMailbox().getPersonalAttendant().setOperator(null);
           } else {
               m_vm.getMailbox().getPersonalAttendant().setOperator(m_vm.extensionToUrl(attdExt));
           }
           
       }
    }
}
