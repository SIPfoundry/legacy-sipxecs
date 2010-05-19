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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Represents a regular phonebook entry
 */
public class PhonebookEntry extends BeanWithId {
    private String m_firstName;
    private String m_lastName;
    private String m_number;
    private String m_internalId;
    private AddressBookEntry m_addressBookEntry;
    private Phonebook m_phonebook;

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getNumber() {
        return m_number;
    }

    public void setNumber(String number) {
        m_number = number;
    }

    public AddressBookEntry getAddressBookEntry() {
        return m_addressBookEntry;
    }

    public void setAddressBookEntry(AddressBookEntry addressBookEntry) {
        m_addressBookEntry = addressBookEntry;
    }

    public Phonebook getPhonebook() {
        return m_phonebook;
    }

    public void setPhonebook(Phonebook phonebook) {
        m_phonebook = phonebook;
    }

    public String getInternalId() {
        return m_internalId;
    }

    public void setInternalId(String internalId) {
        m_internalId = internalId;
    }

    /**
     * Utility method for phonebook vcard and csv writers
     *
     * @return an array of fields. Note that order is important
     */
    public String[] getFields() {
        String firstName = StringUtils.defaultString(getFirstName());
        String lastName = StringUtils.defaultString(getLastName());
        String phoneNumber = StringUtils.defaultString(getNumber());

        String cellPhoneNumber = StringUtils.EMPTY;
        String homePhoneNumber = StringUtils.EMPTY;
        String faxNumber = StringUtils.EMPTY;

        String emailAddress = StringUtils.EMPTY;
        String alternateEmailAddress = StringUtils.EMPTY;
        String imId = StringUtils.EMPTY;
        String alternateImId = StringUtils.EMPTY;

        String companyName = StringUtils.EMPTY;
        String jobTitle = StringUtils.EMPTY;
        String jobDept = StringUtils.EMPTY;
        String location = StringUtils.EMPTY;

        String assistantName = StringUtils.EMPTY;
        String assistantPhone = StringUtils.EMPTY;

        String homeAddressStreet = StringUtils.EMPTY;
        String homeAddressZip = StringUtils.EMPTY;
        String homeAddressCountry = StringUtils.EMPTY;
        String homeAddressState = StringUtils.EMPTY;
        String homeAddressCity = StringUtils.EMPTY;

        String officeAddressStreet = StringUtils.EMPTY;
        String officeAddressZip = StringUtils.EMPTY;
        String officeAddressCountry = StringUtils.EMPTY;
        String officeAddressState = StringUtils.EMPTY;
        String officeAddressCity = StringUtils.EMPTY;
        String officeAddressOfficeDesignation = StringUtils.EMPTY;

        AddressBookEntry addressBook = getAddressBookEntry();
        if (addressBook != null) {
            cellPhoneNumber = StringUtils.defaultString(addressBook.getCellPhoneNumber());
            homePhoneNumber = StringUtils.defaultString(addressBook.getHomePhoneNumber());
            faxNumber = StringUtils.defaultString(addressBook.getFaxNumber());

            emailAddress = StringUtils.defaultString(addressBook.getEmailAddress());
            alternateEmailAddress = StringUtils.defaultString(addressBook.getAlternateEmailAddress());
            imId = StringUtils.defaultString(addressBook.getImId());
            alternateImId = StringUtils.defaultString(addressBook.getAlternateImId());

            companyName = StringUtils.defaultString(addressBook.getCompanyName());
            jobTitle = StringUtils.defaultString(addressBook.getJobTitle());
            jobDept = StringUtils.defaultString(addressBook.getJobDept());
            location = StringUtils.defaultString(addressBook.getLocation());

            assistantName = StringUtils.defaultString(addressBook.getAssistantName());
            assistantPhone = StringUtils.defaultString(addressBook.getAssistantPhoneNumber());

            if (addressBook.getHomeAddress() != null) {
                homeAddressStreet = StringUtils.defaultString(addressBook.getHomeAddress().getStreet());
                homeAddressZip = StringUtils.defaultString(addressBook.getHomeAddress().getZip());
                homeAddressCountry = StringUtils.defaultString(addressBook.getHomeAddress().getCountry());
                homeAddressState = StringUtils.defaultString(addressBook.getHomeAddress().getState());
                homeAddressCity = StringUtils.defaultString(addressBook.getHomeAddress().getCity());
            }

            if (addressBook.getOfficeAddress() != null) {
                officeAddressStreet = StringUtils.defaultString(addressBook.getOfficeAddress().getStreet());
                officeAddressZip = StringUtils.defaultString(addressBook.getOfficeAddress().getZip());
                officeAddressCountry = StringUtils.defaultString(addressBook.getOfficeAddress().getCountry());
                officeAddressState = StringUtils.defaultString(addressBook.getOfficeAddress().getState());
                officeAddressCity = StringUtils.defaultString(addressBook.getOfficeAddress().getCity());
                officeAddressOfficeDesignation = StringUtils.defaultString(addressBook.getOfficeAddress()
                        .getOfficeDesignation());
            }
        }
        // CSV format order:
        // First Name,Last Name,Number,Job Title,Job Department,Company Name,Assistant Name,Cell
        // Phone Number,
        // Home Phone Number,Assistant Phone Number,Fax Number,Im Id,Alternate Im Id,Location,
        // Home city,Home country,Home state,Home street,Home Zip/Postal code,
        // Office city,Office country,Office State,Office Street,Office Zip/Postal code,Office
        // designation,Office Mail,Alternate Office Mail stop
        String[] fields = {
            firstName, lastName, phoneNumber, jobTitle, jobDept, companyName, assistantName, cellPhoneNumber,
            homePhoneNumber, assistantPhone, faxNumber, imId, alternateImId, location, homeAddressCity,
            homeAddressCountry, homeAddressState, homeAddressStreet, homeAddressZip, officeAddressCity,
            officeAddressCountry, officeAddressState, officeAddressStreet, officeAddressZip,
            officeAddressOfficeDesignation, emailAddress, alternateEmailAddress
        };
        return fields;
    }

    public boolean isWritable() {
        if (StringUtils.isEmpty(getFirstName()) && StringUtils.isEmpty(getLastName())) {
            return false;
        }
        return true;
    }

    public static String[] labels() {
        return new String[] {
            "First Name", "Last Name", "Number", "Job Title", "Job Department", "Company Name", "Assistant Name",
            "Cell Phone Number", "Home Phone Number", "Assistant Phone Number", "Fax Number", "Im Id",
            "Alternate Im Id", "Location", "Home city", "Home country", "Home state", "Home street",
            "Home Zip/Postal code", "Office city", "Office country", "Office State", "Office Street",
            "Office Zip/Postal code", "Office designation", "Office Mail", "Alternate Office Mail"
        };
    }
}
