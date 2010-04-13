/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.login;

import org.apache.commons.lang.RandomStringUtils;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;

public class PrivateUserKey extends BeanWithId {
    private String m_key;
    private User m_user;

    public PrivateUserKey(User user) {
        m_user = user;
        m_key = RandomStringUtils.randomAlphanumeric(32);
    }

    public PrivateUserKey() {
        // Hibernate will user this one
    }

    public void setKey(String key) {
        m_key = key;
    }

    public String getKey() {
        return m_key;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public User getUser() {
        return m_user;
    }
}
