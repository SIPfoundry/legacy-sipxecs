/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.callpilot;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.VoiceMail;

public class CpDialog {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.callpilot");
    VoiceMail m_vm;
    Localization m_loc;
    PromptList m_prePl;
    PromptList m_plImmed;
    PromptList m_plDelay;
    PromptList m_plHelp;
    PromptList m_plDelayNonEmpty;
    IvrChoice m_choice;
    
    public CpDialog(VoiceMail vm, String immed, String delay, String help) {
        m_vm = vm ;
        m_loc = vm.getLoc();
        
        if (immed != null) {
            m_plImmed = m_loc.getPromptList(immed);
        }
        
        if (delay != null) {
            m_plDelay = m_loc.getPromptList(delay);
        }
        
        if (help != null) {
            m_plHelp = m_loc.getPromptList(help);
        }
        
        setNonEmptyDelayPrompt("if_finished");
    } 
    
    public CpDialog(VoiceMail vm, String prompt) {
        m_vm = vm ;
        m_loc = vm.getLoc(); 
        
        m_plImmed = m_loc.getPromptList(prompt);
        m_plDelay = m_plImmed;
        m_plHelp =  m_plImmed;
        
        setNonEmptyDelayPrompt("if_finished");
    }     

    public void setNonEmptyDelayPrompt(String delay) {
        if(delay != null) {
            m_plDelayNonEmpty = m_loc.getPromptList(delay);            
        }
    }

    public void setHelpPromptList(PromptList helpPl) {
        m_plHelp = helpPl;
    }
    
    public void setPrePromptList(PromptList prePl) {
        m_prePl = prePl;
    }
    
    public void setPrePromptList(String prePrompt) {     
        m_prePl = m_loc.getPromptList(prePrompt);
    }

    public String collectDigits(int maxDigits, String earlierDigits) {

        String digit;
        String digitsSoFar = earlierDigits;
        LOG.info("CpDialog::multidigit");
        
        while(digitsSoFar.length() < maxDigits) {
            
            if(digitsSoFar.length() == 0) {
                digit = collectDigit("0123456789*#");
            } else {
                digit = collectDigit("0123456789*#", null, m_plDelayNonEmpty, m_plHelp);
            }
                  
            if(digit == null) {
                return null;
            }
            
            if(digit.equals("*")) {
                digitsSoFar = "";
                digit = collectDigit("0123456789*#", m_plHelp, m_plHelp, null);
                
                if(digit == null) {
                    return null;
                }
            }      
 
            if(digit.equals("#")) {
                break;
            }
            
            digitsSoFar += digit;    
        }
            
        return digitsSoFar;
    }
    
    
    public String collectDigits(int maxDigits) {
        return collectDigits(maxDigits, "");
    }
    
    /**
     * Collect a single digit, in the list of validDigits.
     * 
     * @param validDigits
     * @return digit or null if bad entry, timeout, or canceled
     */
    public String collectDigit(String validDigits) {
        return(collectDigit(validDigits, m_plImmed, m_plDelay, m_plHelp));
    }
    
    private String collectDigit(String validDigits, PromptList immed,
                                PromptList delay, PromptList help) {

        boolean warningGiven = false;
        CpMenu menu = new CpMenu(m_vm);
        
        menu.setPrePromptPl(m_prePl);

        for(;;) {
            m_choice = menu.collectDigit(immed, validDigits);
        
            m_prePl = null;
            menu.setPrePromptPl(null);

            if(m_choice.getIvrChoiceReason() == IvrChoiceReason.TIMEOUT) {         
                m_choice = menu.collectDigit(delay, validDigits);
            }
            
            if(m_choice.getIvrChoiceReason() == IvrChoiceReason.TIMEOUT) {
                menu.setInitialTimeout(10000);
                m_choice = menu.collectDigit(help, validDigits);
            }        
            
            if(m_choice.getIvrChoiceReason() == IvrChoiceReason.TIMEOUT) {
                m_choice = menu.collectDigit(m_loc.getPromptList("disc_warn"), validDigits);
                warningGiven = true;
            }
             
            if(m_choice.getIvrChoiceReason() == IvrChoiceReason.TIMEOUT) {
                m_vm.goodbye();
                return null;        
            }
            
            // bad entry, timeout, canceled
            if (!menu.isOkay()) {
                return null;
            }
            
            if(warningGiven) {
                if (m_choice.getDigits().equals("#")) {
                    // through away # and try again
                    warningGiven = false;
                    continue;
                }
            }
                
            return m_choice.getDigits();
        }
    }
    
    public IvrChoice getChoice() {
        return m_choice;
    }
}
