/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class AddressBookEntry extends BeanWithId {

    private String m_jobTitle;
    private String m_jobDept;
    private String m_companyName;
    private String m_assistantName;
    private String m_location;

    private Address m_homeAddress;
    private Address m_officeAddress;
    private String m_cellPhoneNumber;
    private String m_assistantPhoneNumber;
    private String m_faxNumber;
    private String m_imId;
    private String m_alternateImId;

    public String getJobTitle() {
        return m_jobTitle;
    }

    public void setJobTitle(String jobTitle) {
        m_jobTitle = jobTitle;
    }

    public String getJobDept() {
        return m_jobDept;
    }

    public void setJobDept(String jobDept) {
        m_jobDept = jobDept;
    }

    public String getCompanyName() {
        return m_companyName;
    }

    public void setCompanyName(String companyName) {
        m_companyName = companyName;
    }

    public String getAssistantName() {
        return m_assistantName;
    }

    public void setAssistantName(String assistantName) {
        m_assistantName = assistantName;
    }

    public Address getHomeAddress() {
        return m_homeAddress;
    }

    public void setHomeAddress(Address homeAddress) {
        m_homeAddress = homeAddress;
    }

    public Address getOfficeAddress() {
        return m_officeAddress;
    }

    public void setOfficeAddress(Address officeAddress) {
        m_officeAddress = officeAddress;
    }

    public String getCellPhoneNumber() {
        return m_cellPhoneNumber;
    }

    public void setCellPhoneNumber(String cellPhoneNumber) {
        m_cellPhoneNumber = cellPhoneNumber;
    }

    public String getAssistantPhoneNumber() {
        return m_assistantPhoneNumber;
    }

    public void setAssistantPhoneNumber(String assistantPhoneNumber) {
        m_assistantPhoneNumber = assistantPhoneNumber;
    }

    public String getFaxNumber() {
        return m_faxNumber;
    }

    public void setFaxNumber(String faxNumber) {
        m_faxNumber = faxNumber;
    }

    public String getImId() {
        return m_imId;
    }

    public void setImId(String imId) {
        m_imId = imId;
    }

    public String getAlternateImId() {
        return m_alternateImId;
    }

    public void setAlternateImId(String alternateImId) {
        m_alternateImId = alternateImId;
    }

    public String getLocation() {
        return m_location;
    }

    public void setLocation(String location) {
        m_location = location;
    }
}
