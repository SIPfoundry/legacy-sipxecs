/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

import java.util.Vector;

import org.sipfoundry.commons.userdb.User;

/**
 * Holds the User selected by the DialByName dialog, and a reason code if none selected
 */
public class DialByNameChoice extends IvrChoice {

    private Vector<User> m_users;
    
    public DialByNameChoice(IvrChoice choice) {
        super(choice.getDigits(), choice.getIvrChoiceReason());
    }
    
    public DialByNameChoice(User user, String digits, IvrChoiceReason ivrReason) {
        super(digits, ivrReason);
        m_users = new Vector<User>();
        m_users.add(user);
    }

    public DialByNameChoice(Vector<User> users, String digits, IvrChoiceReason ivrReason) {
        super(digits, ivrReason);
        m_users = users;
    }
    
    public Vector<User> getUsers() {
        return m_users;
    }
}
