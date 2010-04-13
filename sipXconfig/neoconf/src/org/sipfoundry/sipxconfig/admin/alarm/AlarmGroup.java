/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.User;

public class AlarmGroup extends BeanWithId implements NamedObject {

    private String m_name;

    private String m_description;

    private boolean m_enabled = true; // default enabled

    private Set<User> m_users = new HashSet<User>();

    private List<String> m_smsAddresses = new ArrayList<String>();

    private List<String> m_emailAddresses = new ArrayList<String>();

    private List<String> m_userEmailAddresses = new ArrayList<String>();

    private List<AlarmTrapReceiver> m_snmpAddresses = new ArrayList<AlarmTrapReceiver>();

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public Set<User> getUsers() {
        return m_users;
    }

    public void setUsers(Set<User> users) {
        m_users = users;
    }

    // SNMP contacts
    public List<AlarmTrapReceiver> getSnmpAddresses() {
        return m_snmpAddresses;
    }

    public void setSnmpAddresses(List<AlarmTrapReceiver> addresses) {
        m_snmpAddresses = addresses;
    }

    // SMS contacts
    public List<String> getSmsAddresses() {
        return m_smsAddresses;
    }

    public void setSmsAddresses(List<String> smsAddresses) {
        m_smsAddresses = smsAddresses;
    }

    public boolean addSmsAddress() {
        return getSmsAddresses().add(StringUtils.EMPTY);
    }

    public String removeSmsAddress(int index) {
        return getSmsAddresses().remove(index);
    }

    // Email contacts
    public List<String> getEmailAddresses() {
        return m_emailAddresses;
    }

    public void setEmailAddresses(List<String> emailAddresses) {
        m_emailAddresses = emailAddresses;
    }

    public boolean addEmailAddress() {
        return getEmailAddresses().add(StringUtils.EMPTY);
    }

    public String removeEmailAddress(int index) {
        return getEmailAddresses().remove(index);
    }

    // Email contacts - Convert user to corresponding email address
    public List<String> getUserEmailAddresses() {
        return m_userEmailAddresses;
    }

    public void setUserEmailAddresses(List<String> userEmailAddresses) {
        m_userEmailAddresses = userEmailAddresses;
    }

    // List of email contacts to export in the XML file
    public List<String> getContactEmailAddresses() {
        List<String> contactEmailAddresses = new ArrayList<String>();
        if (m_enabled) {
            contactEmailAddresses.addAll(m_emailAddresses);
            contactEmailAddresses.addAll(m_userEmailAddresses);
        }
        return contactEmailAddresses;
    }

    // List of SMS contacts to export in the XML file
    public List<String> getContactSmsAddresses() {
        List<String> contactSmsAddresses = new ArrayList<String>();
        if (m_enabled) {
            contactSmsAddresses.addAll(m_smsAddresses);
        }
        return contactSmsAddresses;
    }

    // List of snmp contacts to export in the XML file
    public List<AlarmTrapReceiver> getContactSnmpAddresses() {
        List<AlarmTrapReceiver> contactSnmpAddresses = new ArrayList<AlarmTrapReceiver>();
        if (m_enabled) {
            contactSnmpAddresses.addAll(m_snmpAddresses);
        }
        return contactSnmpAddresses;
    }
}
