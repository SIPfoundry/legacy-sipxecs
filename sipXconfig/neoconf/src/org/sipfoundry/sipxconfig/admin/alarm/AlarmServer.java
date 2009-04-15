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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class AlarmServer extends BeanWithId {
    private boolean m_emailNotificationEnabled = true; // default enabled

    private AlarmServerContacts m_contacts = new AlarmServerContacts();

    private String m_fromEmailAddress;

    public boolean isEmailNotificationEnabled() {
        return m_emailNotificationEnabled;
    }

    public void setEmailNotificationEnabled(boolean emailNotificationEnabled) {
        m_emailNotificationEnabled = emailNotificationEnabled;
    }

    public AlarmServerContacts getContacts() {
        return m_contacts;
    }

    public void setContacts(AlarmServerContacts contacts) {
        m_contacts = contacts;
    }

    public void setFromEmailAddress(String fromEmailAddress) {
        m_fromEmailAddress = fromEmailAddress;
    }

    public String getFromEmailAddress() {
        return m_fromEmailAddress;
    }

}
