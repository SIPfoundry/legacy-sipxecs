package org.sipfoundry.callpilot;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Vector;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.userdb.DistributionList;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.Distributions;
import org.sipfoundry.voicemail.DistributionsReader;
import org.sipfoundry.voicemail.VoiceMail;

public class CpAddrListDialog {

    private VoiceMail m_vm;
    private Vector<User> m_userList;
    private int m_numAddressesLastAdded; 
    
    public CpAddrListDialog(VoiceMail callPilot) {
        m_vm = callPilot;
    }
    
    private void cancelLastAddress() {
        if(m_numAddressesLastAdded > 0) {
            if(m_numAddressesLastAdded == 1) {
                User deletedUser =  m_userList.remove(m_userList.size()-1);
                m_vm.getLoc().play("address_canceled", "", deletedUser.getUserName());
            } else {
                for(int i=0; i<m_numAddressesLastAdded; i++) {
                    m_userList.remove(m_userList.size()-1);
                }
                m_vm.getLoc().play("address_canceled", "", "");
            }
            
            m_numAddressesLastAdded = 0;
        } else {
            m_vm.playError("cannot_cancel_address");                
        }
    }
    
    private boolean AddDistributionList(String index) {
        
        Mailbox mbx = m_vm.getMailbox();
        Distributions d = null;
        
        // See if the new way to get distribution lists is being used.
        HashMap<String, DistributionList> dlists = mbx.getUser().getDistributionLists();
        if (dlists == null) {
            // Use the old way (distributionListsFile in the user's mailbox directory)
            DistributionsReader dr = new DistributionsReader();
            d = dr.readObject(mbx.getDistributionListsFile()) ;
        } else {
            // Delete the old distributions.xml file so it isn't accidentally used later
            mbx.getDistributionListsFile().delete();
        }
                   
        Collection<String> userNames = null;
        if (dlists != null) {
            DistributionList list = dlists.get(index);
            if (list != null) {
                // TODO: system distribution list parm set to null for now
                userNames = list.getList(null); 
            }
        } else if (d != null) {
            userNames = d.getList(index);
        } else { 
            m_vm.playError("invalid_number");
            return false;
        }
            
        m_numAddressesLastAdded = 0;
        if (userNames != null) {
            for (String userName : userNames) {
                User u = m_vm.getValidUsers().getUser(userName);
                if (u != null && u.hasVoicemail()) {
                    m_userList.add(u);
                    m_numAddressesLastAdded++;
                }
            }             
        } else { 
            m_vm.playError("invalid_number");
            return false;
        }
        
        return true;
    }
    
    public Vector<User> getAddressList() {
        m_userList = new Vector<User>(); 
        CpDialog cpDialog;
        PromptList prePl = null;
        PromptList helpPl;
        User user;
        
        String namePrefix = ((IvrConfiguration)m_vm.getLoc().getConfig()).getCPUINameDialingPrefix();
        
        for(;;) {
            helpPl = m_vm.getLoc().getPromptList();
            
            if(m_userList.size() == 0) {
                cpDialog = new CpDialog(m_vm, "first_addr_immed", "first_addr_delay", null);                   
                helpPl.addFragment("first_addr_help", namePrefix);
            } else {
                cpDialog = new CpDialog(m_vm, "second_addr_immed", "second_addr_delay", null);
                helpPl.addFragment("second_addr_help", namePrefix);
            }  
            
            cpDialog.setHelpPromptList(helpPl);
            
            if(prePl != null) {
                cpDialog.setPrePromptList(prePl);
                prePl = null;
            }
            
            String addrStr = cpDialog.collectDigits(namePrefix.length()); 

            if(addrStr.length() == 0) {
                return m_userList;
            }
            
            if(addrStr.equals("0")) {
                cancelLastAddress();
                continue;
            }
            
            if(addrStr.length() == 1) {
                // looks like user entered a PDL number               
                if(AddDistributionList(addrStr)) {
                    prePl = m_vm.getLoc().getPromptList();
                    prePl.addFragment("group_list", addrStr);
                }
                continue;
            }
            
            if(addrStr.equals(namePrefix)) { 
                user = getAddressByName();
                if(user == null) {
                    continue;
                } else {
                    addrStr = user.getUserName();
                }
            } else {
                // get the rest of the non-name address
                addrStr = cpDialog.collectDigits(15, addrStr);
            }                      
            
            // check again!
            if(addrStr.length() == 0) {
                return m_userList;
            }
            
            if(addrStr.equals("0")) {
                cancelLastAddress();
                continue;
            }
         
            user = m_vm.getValidUsers().getUser(addrStr);
            if(user == null) {                            
                m_vm.playError("no_mailbox_at", addrStr);
            } else if(!user.hasVoicemail()) {
                m_vm.playError("no_mailbox_at", addrStr);
            } else {
                m_userList.add(user);
                m_numAddressesLastAdded = 1;
                prePl = m_vm.getLoc().getPromptList();
                
                File nameFile = new Mailbox(user).getRecordedNameFile();              
                
                if (nameFile.exists()) {           
                    prePl.addFragment("a_prompt", nameFile.getPath());
                } else {   
                    prePl.addFragment("extension", user.getUserName());
                }
            }             
        }   
    }
    
    private User getAddressByName() {
        
        CpDialByName dbn = new CpDialByName(m_vm, m_vm.getConfig(), m_vm.getValidUsers());
        DialByNameChoice dbnChoice = dbn.dialByName();
        
        if (dbnChoice.getIvrChoiceReason() == IvrChoiceReason.SUCCESS) {
            Vector<User> userList = dbnChoice.getUsers();
            if(userList != null) {
                return userList.firstElement();
            }
                
            return null;
        } 
        return null;
    }   
}
