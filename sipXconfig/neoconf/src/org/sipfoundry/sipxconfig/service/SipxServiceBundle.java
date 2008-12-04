/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;


public class SipxServiceBundle {
    private final String m_name;

    public SipxServiceBundle(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    @Override
    public int hashCode() {
        return m_name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return m_name.equals(obj);
    }
}
