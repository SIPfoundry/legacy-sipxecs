/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.userdb.profile;

public class Address {

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
