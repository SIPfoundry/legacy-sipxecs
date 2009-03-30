/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

/**
 * Holds the User selected by the DialByName dialog, and a reason code if none selected
 */
public class DialByNameChoice extends IvrChoice {

    private User m_user;
    
    DialByNameChoice(User user, String digits, IvrChoiceReason ivrReason) {
        super(digits, ivrReason);
        m_user = user;
    }
    
    public User getUser() {
        return m_user;
    }
}
