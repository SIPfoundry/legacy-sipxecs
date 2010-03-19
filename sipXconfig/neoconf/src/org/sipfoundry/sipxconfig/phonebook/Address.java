/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class Address extends BeanWithId {

    private String m_street;
    private String m_city;
    private String m_country;
    private String m_state;
    private String m_zip;
    private String m_officeDesignation;

    public String getStreet() {
        return m_street;
    }

    public void setStreet(String street) {
        m_street = street;
    }

    public String getCity() {
        return m_city;
    }

    public void setCity(String city) {
        m_city = city;
    }

    public String getCountry() {
        return m_country;
    }

    public void setCountry(String country) {
        m_country = country;
    }

    public String getState() {
        return m_state;
    }

    public void setState(String state) {
        m_state = state;
    }

    public String getZip() {
        return m_zip;
    }

    public void setZip(String zip) {
        m_zip = zip;
    }

    public String getOfficeDesignation() {
        return m_officeDesignation;
    }

    public void setOfficeDesignation(String officeDesignation) {
        m_officeDesignation = officeDesignation;
    }
}
