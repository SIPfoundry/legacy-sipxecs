/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

public class AlarmServerContacts {
    private List<String> m_addresses = new ArrayList<String>();

    public void setAddresses(List<String> addresses) {
        m_addresses = addresses;
    }

    public List<String> getAddresses() {
        return m_addresses;
    }

    public boolean addAddress() {
        return getAddresses().add(StringUtils.EMPTY);
    }

    public String removeAddress(int index) {
        return getAddresses().remove(index);
    }

    public boolean isEmpty() {
        return getAddresses().isEmpty();
    }

    public int size() {
        return getAddresses().size();
    }
}
