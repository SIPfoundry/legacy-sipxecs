/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

public class IvrChoice {

    public enum IvrChoiceReason {
        SUCCESS,
        CANCELED,
        TIMEOUT,
        FAILURE
        }
    
    private String m_digits;
    private IvrChoiceReason m_ivrChoiceReason;
    
    public IvrChoice(String digits, IvrChoiceReason reason) {
        m_digits = digits;
        m_ivrChoiceReason = reason;
    }
    
    public String getDigits() {
        return m_digits;
    }
    
    public IvrChoiceReason getIvrChoiceReason() {
        return m_ivrChoiceReason;
    }

}
