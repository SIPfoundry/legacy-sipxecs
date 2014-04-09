/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.branch;


import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.phonebook.Address;


public class Branch extends BeanWithId implements NamedObject {
    private String m_name;
    private String m_description;
    private Address m_address = new Address();
    private String m_phoneNumber;
    private String m_faxNumber;
    private String m_timeZone;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public Address getAddress() {
        return m_address;
    }

    /**
     * (hibernate injects null value here when all homeAddress fields are empty) see:
     * http://opensource.atlassian.com/projects/hibernate/browse/HB-31
     */
    public void setAddress(Address address) {
        m_address = address == null ? new Address() : address;
    }

    public String getPhoneNumber() {
        return m_phoneNumber;
    }

    public void setPhoneNumber(String phoneNumber) {
        m_phoneNumber = phoneNumber;
    }

    public String getFaxNumber() {
        return m_faxNumber;
    }

    public void setFaxNumber(String faxNumber) {
        m_faxNumber = faxNumber;
    }

    public String getTimeZone() {
        return m_timeZone;
    }

    public void setTimeZone(String timeZone) {
        m_timeZone = timeZone;
    }
}
