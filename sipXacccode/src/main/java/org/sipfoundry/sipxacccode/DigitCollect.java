/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxacccode;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.Play;
import org.sipfoundry.commons.freeswitch.PromptList;

public class DigitCollect {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxacccode");
    protected Localization m_loc;
    protected FreeSwitchEventSocketInterface m_fses;
    
    int m_maxDigits=1;
    int m_invalidMax=2;
    int m_timeoutMax=2;
    int m_initialTimeout=5000;
    int m_interDigitTimeout=3000;
    int m_extraDigitTimeout=1000;
    
    PromptList m_prePromptPl;
    PromptList m_timeoutPl;
    PromptList m_errorPl;
    
    boolean m_isStarCancel=false;
    String m_termChars = "";
    String m_digits = "";
    
    public DigitCollect(Localization loc) {
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
    public String collectDigits(PromptList menuPl, int maxDigits) {
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
    public String collectDigit(PromptList menuPl, String validDigits) {
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
    public String collectDtmf(PromptList menuPl, int maxDigits) {
        m_maxDigits = maxDigits ;
        m_isStarCancel = false;
        m_termChars="#";
        return collect(menuPl, "0123456789#*");
    }

    protected String collect(PromptList menuPl, String validDigits) {
        String digits;
        int invalidCount = 0;
        int timeoutCount = 0;
        boolean playPrePrompt = true ;
        for (;;) {
            // Check for a failure condition
            if (invalidCount > m_invalidMax) {
                m_digits = "";
                return m_digits;
            }
            if (timeoutCount > m_timeoutMax) {
                m_digits = "" ;
                return m_digits;
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
            LOG.info("DigitCollect::collect Collected digits=("+m_fses.redact(digits)+")");
    
            // See if it timed out (no digits)
            if (digits.equals("")) {
                LOG.info("DigitCollect::collect timeout");
                if (m_timeoutPl != null) {
                    m_loc.play(m_timeoutPl, "");
                }
                timeoutCount++;
                continue;
            }
    
            // See if they canceled with "*"
            if (m_isStarCancel && digits.contains("*")) {
                LOG.info("DigitCollect::collect canceled");
                return m_digits = "";
            }
            
            if (m_maxDigits == 1) {
                // Check if entered digits match any actions
                if (validDigits.contains(digits)) {
                    return m_digits = digits;
                }
        
                // What they entered has no corresponding action.
                LOG.info("DigitCollect::collect Invalid entry");
                if (m_errorPl != null) {
                    m_loc.play("invalid_try_again", "");
                }
                invalidCount++;
            } else {
                return m_digits = digits;
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

    /**
     * true if TIMEOUT
     * @return
     */
    public boolean isTimeout() {
        boolean timeout = false;
        if (m_digits == "") {
           timeout = true;
        }
        return timeout;
    }

    /**
     * true if CANCELED
     * @return
     */
    public boolean isCanceled() {
        boolean cancelled = false;
        if (m_digits == "") {
           cancelled = true;
        }
        return cancelled;
    }

    /**
     * true if SUCCESS (not TIMEOUT, not CANCELED)
     * @return
     */
    public boolean isOkay() {
        boolean okay = false;
        if (m_digits != "") {
           okay = true;
        }
        return okay;
    }
}
