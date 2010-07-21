/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Values of the enums below determine the exact format of CSV file
 *
 * "Username", "Pintoken", "Sip Password", "FirstName", "LastName", "Alias", "UserGroup",
 * "SerialNumber", "Manufacturer", "Model", "Phone Group", "Phone Description"
 */
public enum Index {
    // user fields
    USERNAME("userName", 0), PIN("pin", 1), SIP_PASSWORD("sipPassword", 2), FIRST_NAME(
            "firstName", 3), LAST_NAME("lastName", 4), ALIAS("aliasesString", 5), EMAIL(
            "emailAddress", 6), USER_GROUP("userGroupName", 7),

    // phone fields
    SERIAL_NUMBER("serialNumber", 8), MODEL_ID("modelId", 9), PHONE_GROUP("phoneGroupName", 10), PHONE_DESCRIPTION(
            "description", 11),
    // XMPP
    IM_ID("imId", 12),
    JOB_TITLE("createdAddressBookEntry.jobTitle", 13),
    JOB_DEPT("createdAddressBookEntry.jobDept", 14),
    COMPANY_NAME("createdAddressBookEntry.companyName", 15),
    ASSISTANT_NAME("createdAddressBookEntry.assistantName", 16),
    CELL_PHONE_NUMBER("createdAddressBookEntry.cellPhoneNumber", 17),
    HOME_PHONE_NUMBER("createdAddressBookEntry.homePhoneNumber", 18),
    ASSISTANT_PHONE_NUMBER("createdAddressBookEntry.assistantPhoneNumber", 19),
    FAX_NUMBER("createdAddressBookEntry.faxNumber", 20),
    ALTERNATE_EMAIL("createdAddressBookEntry.alternateEmailAddress", 21),
    ALTERNATE_IM_ID("createdAddressBookEntry.alternateImId", 22),
    LOCATION("createdAddressBookEntry.location", 23),
    HOME_STREET("createdAddressBookEntry.homeAddress.street", 24),
    HOME_CITY("createdAddressBookEntry.homeAddress.city", 25),
    HOME_STATE("createdAddressBookEntry.homeAddress.state", 26),
    HOME_COUNTRY("createdAddressBookEntry.homeAddress.country", 27),
    HOME_ZIP("createdAddressBookEntry.homeAddress.zip", 28),
    OFFICE_STREET("createdAddressBookEntry.officeAddress.street", 29),
    OFFICE_CITY("createdAddressBookEntry.officeAddress.city", 30),
    OFFICE_STATE("createdAddressBookEntry.officeAddress.state", 31),
    OFFICE_COUNTRY("createdAddressBookEntry.officeAddress.country", 32),
    OFFICE_ZIP("createdAddressBookEntry.officeAddress.zip", 33);

    private final String m_name;
    private final int m_value;

    Index(String name, int value) {
        m_name = name;
        m_value = value;
    }

    public String getName() {
        return m_name;
    }

    public int getValue() {
        return m_value;
    }

    public String get(String[] row) {
        return (m_value < row.length ? row[m_value] : StringUtils.EMPTY);
    }

    public void set(String[] row, String value) {
        row[m_value] = value;
    }

    public void setProperty(Object bean, String[] row) {
        String value = get(row);
        if (value.length() == 0) {
            return;
        }
        try {
            BeanUtils.setProperty(bean, m_name, value);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e.getCause());
        }
    }

    public String getProperty(Object bean) {
        try {
            return BeanUtils.getProperty(bean, m_name);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e.getCause());
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    public static String[] getAllNames() {
        Index[] values = values();
        String[] names = new String[values.length];
        for (int i = 0; i < names.length; i++) {
            names[i] = values[i].getName();
        }
        return names;
    }

    public static String[] newRow() {
        String[] row = new String[values().length];
        Arrays.fill(row, StringUtils.EMPTY);
        return row;
    }

    public static String[] labels() {
        return new String[] {
            "User name", "Voice-mail PIN", "SIP password", "First name", "Last name",
            "User alias", "EMail address", "User group", "Phone serial number", "Phone model",
            "Phone group", "Phone description", "Im Id", "Job Title", "Job department",
            "Company name", "Assistant name", "Cell phone number", "Home phone number",
            "Assistant phone number", "Fax number", "Alternate email", "Alternate im",
            "Location", "Home street", "Home city", "Home state", "Home country", "Home zip",
            "Office street", "Office city", "Office state", "Office country", "Office zip"
        };
    }
}
