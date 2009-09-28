/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Map;

import org.apache.commons.lang.StringUtils;

public class OutlookPhonebookCsvHeader extends PhonebookCsvHeader {
    private static final String FIRST_NAME = "First Name";
    private static final String LAST_NAME = "Last Name";
    private static final String PHONE = "Primary Phone";
    private static final String JOB_TITLE = "Job Title";
    private static final String JOB_DEPT = "Department";
    private static final String COMPANY_NAME = "Company";
    private static final String LOCATION = "Location";
    private static final String ASSISTANT_NAME = "Assistant's Name";
    private static final String PHONE_MOBILE = "Mobile Phone";
    private static final String PHONE_HOME = "Home Phone";
    private static final String PHONE_ASSISTANT = "Assistant's Phone";
    private static final String FAX_HOME = "Home Fax";
    private static final String ADDRESS_CITY_HOME = "Home City";
    private static final String ADDRESS_CITY_BUSINESS = "Business City";
    private static final String ADDRESS_COUNTRY_HOME = "Home Country/Region";
    private static final String ADDRESS_COUNTRY_BUSINESS = "Business Country/Region";
    private static final String ADDRESS_STATE_HOME = "Home State";
    private static final String ADDRESS_STATE_BUSINESS = "Business State";
    private static final String ADDRESS_STREET_HOME = "Home Street";
    private static final String ADDRESS_STREET_BUSINESS = "Business Street";
    private static final String ADDRESS_ZIP_HOME = "Home Postal Code";
    private static final String ADDRESS_ZIP_BUSINESS = "Business Postal Code";

    public OutlookPhonebookCsvHeader(Map<String, Integer> header) {
        super(header);
    }

    public String getFirstName(String[] row) {
        return getValueForSymbol(row, FIRST_NAME);
    }

    public String getLastName(String[] row) {
        return getValueForSymbol(row, LAST_NAME);
    }

    public String getNumber(String[] row) {
        return StringUtils.defaultString(getValueForSymbol(row, PHONE));
    }

    public AddressBookEntry getAddressBookEntry(String[] row) {
        if (row.length < 25) {
            return null;
        }

        AddressBookEntry abe = new AddressBookEntry();
        abe.setJobTitle(getValueForSymbol(row, JOB_TITLE));
        abe.setJobDept(getValueForSymbol(row, JOB_DEPT));
        abe.setCompanyName(getValueForSymbol(row, COMPANY_NAME));
        abe.setAssistantName(getValueForSymbol(row, ASSISTANT_NAME));
        abe.setCellPhoneNumber(getValueForSymbol(row, PHONE_MOBILE));
        abe.setHomePhoneNumber(getValueForSymbol(row, PHONE_HOME));
        abe.setAssistantPhoneNumber(getValueForSymbol(row, PHONE_ASSISTANT));
        abe.setFaxNumber(getValueForSymbol(row, FAX_HOME));
        abe.setLocation(getValueForSymbol(row, LOCATION));

        Address homeAddress = new Address();
        homeAddress.setCity(getValueForSymbol(row, ADDRESS_CITY_HOME));
        homeAddress.setCountry(getValueForSymbol(row, ADDRESS_COUNTRY_HOME));
        homeAddress.setState(getValueForSymbol(row, ADDRESS_STATE_HOME));
        homeAddress.setStreet(getValueForSymbol(row, ADDRESS_STREET_HOME));
        homeAddress.setZip(getValueForSymbol(row, ADDRESS_ZIP_HOME));
        abe.setHomeAddress(homeAddress);

        Address officeAddress = new Address();
        officeAddress.setCity(getValueForSymbol(row, ADDRESS_CITY_BUSINESS));
        officeAddress.setCountry(getValueForSymbol(row, ADDRESS_COUNTRY_BUSINESS));
        officeAddress.setState(getValueForSymbol(row, ADDRESS_STATE_BUSINESS));
        officeAddress.setStreet(getValueForSymbol(row, ADDRESS_STREET_BUSINESS));
        officeAddress.setZip(getValueForSymbol(row, ADDRESS_ZIP_BUSINESS));
        abe.setOfficeAddress(officeAddress);

        return abe;
    }
}
