/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import org.apache.log4j.Logger;

public class ApplicationConfiguraton {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    /**
     * Holds the configuration data needed for Applications
     * 
     */

    private int m_initialTimeout; // initial digit timeout (mS)
    private int m_interDigitTimeout; // subsequent digit timeout (mS)
    private int m_extraDigitTimeout; // extra (wait for #) (mS)
    private int m_maximumDigits; // maximum extension length
    private int m_noInputCount; // give up after this many timeouts
    private int m_invalidResponseCount; // give up after this many bad entries
    private boolean m_transferOnFailures; // What to do on failure
    private String m_transferPrompt; // What to say
    private String m_transferUrl; // Where to go


    public ApplicationConfiguraton() {
        // Global defaults if otherwise not specified
        m_initialTimeout = 7000;
        m_interDigitTimeout = 3000;
        m_extraDigitTimeout = 3000;
        m_maximumDigits = 10;
        m_noInputCount = 2;
        m_invalidResponseCount = 2;
    }
    
    public int getInitialTimeout() {
        return m_initialTimeout;
    }
    

    public int getInterDigitTimeout() {
        return m_interDigitTimeout;
    }

    public int getExtraDigitTimeout() {
        return m_extraDigitTimeout;
    }

    public int getMaximumDigits() {
        return m_maximumDigits;
    }

    public int getNoInputCount() {
        return m_noInputCount;
    }

    public int getInvalidResponseCount() {
        return m_invalidResponseCount;
    }

    public boolean isTransferOnFailure() {
        return m_transferOnFailures;
    }

    public String getTransferPrompt() {
        return m_transferPrompt;
    }

    public String getTransferURL() {
        return m_transferUrl;
    }
    
    public boolean isTransferOnFailures() {
        return m_transferOnFailures;
    }

    public void setTransferOnFailures(boolean transferOnFailures) {
        m_transferOnFailures = transferOnFailures;
    }

    public String getTransferUrl() {
        return m_transferUrl;
    }

    public void setTransferUrl(String transferUrl) {
        m_transferUrl = transferUrl;
    }

    public void setInitialTimeout(int initialTimeout) {
        m_initialTimeout = initialTimeout;
    }

    public void setInterDigitTimeout(int interDigitTimeout) {
        m_interDigitTimeout = interDigitTimeout;
    }

    public void setExtraDigitTimeout(int extraDigitTimeout) {
        m_extraDigitTimeout = extraDigitTimeout;
    }

    public void setMaximumDigits(int maximumDigits) {
        m_maximumDigits = maximumDigits;
    }

    public void setNoInputCount(int noInputCount) {
        m_noInputCount = noInputCount;
    }

    public void setInvalidResponseCount(int invalidResponseCount) {
        m_invalidResponseCount = invalidResponseCount;
    }

    public void setTransferPrompt(String transferPrompt) {
        m_transferPrompt = transferPrompt;
    }

}
