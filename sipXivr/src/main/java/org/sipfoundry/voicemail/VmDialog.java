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
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.IvrChoice;

public class VmDialog {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    VoiceMail m_vm;
    Localization m_loc;
    PromptList m_prePl;
    PromptList m_pl;
    boolean m_speakCanceled;
    IvrChoice m_choice;
    
    public VmDialog(VoiceMail vm, String plFrag) {
        m_vm = vm ;
        m_loc = vm.getLoc();
        m_speakCanceled = true;
        
        if (plFrag != null) {
            m_pl = m_loc.getPromptList(plFrag);
        }
    }
    
    public void setPromptList(PromptList pl) {
        m_pl = pl;
    }
    
    public void setPrePromptList(PromptList prePl) {
        m_prePl = prePl;
    }
    
    public void setSpeakCanceled(boolean speakCanceled) {
        m_speakCanceled = speakCanceled;
    }
    
    public String collectDigits(int maxDigits) {
        int errorCount = 0;
        for(;;) {
            LOG.info("VmDialog::multidigit");
    
            if (errorCount > m_vm.getConfig().getInvalidResponseCount()) {
                m_vm.failure();
                return null;
            }
    
            // {prompts in m_pl}
            VmMenu menu = new VmMenu(m_vm);
            menu.setSpeakCanceled(m_speakCanceled);
            m_choice = menu.collectDigits(m_pl, maxDigits);
    
            if (!menu.isOkay()) {
                return null;
            }
            
            if (!validate()) {
                errorCount++;
                continue;
            }
            return m_choice.getDigits();
        }
    }
    
    /**
     * Validate multi-digit numbers.
     * @return
     */
    public boolean validate() {
        // Validate m_choice
        return true ;
    }
    
    /**
     * Collect a single digit, in the list of validDigits.
     * 
     * @param validDigits
     * @return digit or null if bad entry, timeout, or canceled
     */
    public String collectDigit(String validDigits) {
        LOG.info("VmDialog::singleDigit");

        VmMenu menu = new VmMenu(m_vm);
        menu.setSpeakCanceled(m_speakCanceled);
        if (m_prePl != null) {
            // {prompts in m_prePl}
            menu.setPrePromptPl(m_prePl);
            m_prePl = null ; // Only play it once
        }

        // {prompts in m_pl}
        m_choice = menu.collectDigit(m_pl, validDigits);

        // bad entry, timeout, canceled
        if (!menu.isOkay()) {
            return null;
        }
            
        return m_choice.getDigits();
    }
    
    public IvrChoice getChoice() {
        return m_choice;
    }
}
