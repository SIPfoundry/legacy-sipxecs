/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.Play;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class Menu {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    protected Localization m_loc;
    protected FreeSwitchEventSocketInterface m_fses;
    
    int m_maxDigits=1;
    int m_invalidMax=3;
    int m_timeoutMax=3;
    int m_initialTimeout=3000;
    int m_interDigitTimeout=3000;
    int m_extraDigitTimeout=1000;
    
    PromptList m_prePromptPl;
    PromptList m_timeoutPl;
    PromptList m_errorPl;
    
    IvrChoice m_choice;
    
    boolean m_isStarCancel=false;
    String m_termChars = "";
    
    public Menu(Localization loc) {
        m_loc = loc;
        m_fses = loc.getFreeSwitchEventSocketInterface();
    }

    /**
     * Collect several DTMF digits (0-9) from the user.
     * "*" will cancel.
     * "#" is a terminating character
     * @param menuPl
     * @param maxDigits
     * @return
     */
    public IvrChoice collectDigits(PromptList menuPl, int maxDigits) {
        m_maxDigits = maxDigits;
        m_isStarCancel = true;
        m_termChars = "#*";
        return collect(menuPl, "0123456789");
    }

    /**
     * Collect a single DTMF digit (0-9) from the user.
     * "*" will cancel
     * @param menuPl
     * @param validDigits
     * @return
     */
    public IvrChoice collectDigit(PromptList menuPl, String validDigits) {
        m_maxDigits = 1 ;
        m_isStarCancel = true ;
        m_termChars = "#*";
        return collect(menuPl, validDigits);
    }

    /**
     * Collect several DTMF keypresses (0-9,#,*) from the user.
     * @param menuPl
     * @param maxDigits
     * @return
     */
    public IvrChoice collectDtmf(PromptList menuPl, int maxDigits) {
        m_maxDigits = maxDigits ;
        m_isStarCancel = false;
        m_termChars="#";
        return collect(menuPl, "0123456789#*");
    }

    protected IvrChoice collect(PromptList menuPl, String validDigits) {
        String digits;
        int invalidCount = 0;
        int timeoutCount = 0;
        boolean playPrePrompt = true ;
        for (;;) {
            // Check for a failure condition
            if (invalidCount > m_invalidMax) {
                m_choice = new IvrChoice(null, IvrChoiceReason.FAILURE);
                return m_choice;
            }
            if (timeoutCount > m_timeoutMax) {
                m_choice = new IvrChoice(null, IvrChoiceReason.TIMEOUT);
                return m_choice;
            }
    
            PromptList pl = m_loc.getPromptList();
            // Play any pre-menu prompts
            if (m_prePromptPl != null && playPrePrompt) {
                pl.addPrompts(m_prePromptPl);
                playPrePrompt = false ; // Only play it once
            }
            
            // Play the menu prompts
            if(menuPl != null) {
                pl.addPrompts(menuPl);           
            }
            
            Play p = new Play(m_fses, pl);
            
            if (m_isStarCancel) {
                validDigits += "*" ; // Add "*" to valid digits so * can barge
            }
            p.setDigitMask(validDigits);
            p.go();
    
            // Wait for the caller to enter a selection.
            Collect c ;
            if (m_maxDigits == 1) {
                c = new Collect(m_fses, 1, m_initialTimeout, 0, 0);
            } else {
                c = new Collect(m_fses, m_maxDigits, m_initialTimeout, m_interDigitTimeout, m_extraDigitTimeout);
            }
            
            c.setTermChars(m_termChars);
            c.go();
            digits = c.getDigits();
            LOG.info("Menu::menu Collected digits=("+m_fses.redact(digits)+")");
    
            // See if it timed out (no digits)
            if (digits.equals("")) {
                LOG.info("Menu::menu timeout");
                if (m_timeoutPl != null) {
                    m_loc.play(m_timeoutPl, "");
                }
                timeoutCount++;
                continue;
            }
    
            // See if they canceled with "*"
            if (m_isStarCancel && digits.contains("*")) {
                LOG.info("Menu::menu canceled");
                return m_choice = new IvrChoice(null, IvrChoiceReason.CANCELED);
            }
            
            if (m_maxDigits == 1) {
                // Check if entered digits match any actions
                if (validDigits.contains(digits)) {
                    return m_choice = new IvrChoice(digits, IvrChoiceReason.SUCCESS);
                }
        
                // What they entered has no corresponding action.
                LOG.info("Menu::menu Invalid entry");
                if (m_errorPl != null) {
                    m_loc.play("invalid_try_again", "");
                }
                invalidCount++;
            } else {
                return m_choice = new IvrChoice(digits, IvrChoiceReason.SUCCESS);
            }
        }
        
    }


    public int getInitialTimeout() {
        return m_initialTimeout;
    }


    /**
     * Set the time to wait for the first digit (in mS)
     * @param initialTimeout_mS
     */
    public void setInitialTimeout(int initialTimeout_mS) {
        m_initialTimeout = initialTimeout_mS;
    }


    public int getInterDigitTimeout() {
        return m_interDigitTimeout;
    }


    /**
     * Set the time to wait for the second and subsequent digits (in mS)
     * @param interDigitTimeout_mS
     */
    public void setInterDigitTimeout(int interDigitTimeout_mS) {
        m_interDigitTimeout = interDigitTimeout_mS;
    }


    public int getExtraDigitTimeout() {
        return m_extraDigitTimeout;
    }


    /**
     * Set the time to wait for the return key (in mS)
     * @param extraDigitTimeout_mS
     */
    public void setExtraDigitTimeout(int extraDigitTimeout_mS) {
        m_extraDigitTimeout = extraDigitTimeout_mS;
    }


    public int getInvalidMax() {
        return m_invalidMax;
    }


    public void setInvalidMax(int invalidMax) {
        m_invalidMax = invalidMax;
    }


    public int getTimeoutMax() {
        return m_timeoutMax;
    }


    public void setTimeoutMax(int timeoutMax) {
        m_timeoutMax = timeoutMax;
    }

    public PromptList getPrePromptPl() {
        return m_prePromptPl;
    }

    public void setPrePromptPl(PromptList prePromptPl) {
        m_prePromptPl = prePromptPl;
    }

    public PromptList getTimeoutPl() {
        return m_timeoutPl;
    }

    public void setTimeoutPl(PromptList timeoutPl) {
        m_timeoutPl = timeoutPl;
    }

    public PromptList getErrorPl() {
        return m_errorPl;
    }

    public void setErrorPl(PromptList errorPl) {
        m_errorPl = errorPl;
    }

    public IvrChoice getChoice() {
        return m_choice;
    }
    
    /**
     * true if TIMEOUT
     * @return
     */
    public boolean isTimeout() {
        return m_choice.getIvrChoiceReason().equals(IvrChoiceReason.TIMEOUT);
    }

    /**
     * true if CANCELED
     * @return
     */
    public boolean isCanceled() {
        return m_choice.getIvrChoiceReason().equals(IvrChoiceReason.CANCELED);
    }

    /**
     * true if SUCCESS (not TIMEOUT, not CANCELED)
     * @return
     */
    public boolean isOkay() {
        return m_choice.getIvrChoiceReason().equals(IvrChoiceReason.SUCCESS);
    }
}
