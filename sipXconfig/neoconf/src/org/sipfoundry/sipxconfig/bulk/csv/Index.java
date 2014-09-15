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
 * "Username", "Pintoken", "Voicemail PIN", "Sip Password", "FirstName", "LastName", "Alias", "UserGroup",
 * "SerialNumber", "Manufacturer", "Model", "Phone Group", "Phone Description"
 */
public enum Index {
    // user fields
    USERNAME("userName", 0), PIN("pin", 1), VOICEMAIL_PIN("voicemailPin", 2),
    SIP_PASSWORD("sipPassword", 3), FIRST_NAME("firstName", 4),
    LAST_NAME("lastName", 5), ALIAS("aliasesString", 6), EMAIL("emailAddress", 7), USER_GROUP("userGroupName", 8),

    // phone fields
    SERIAL_NUMBER("serialNumber", 9), MODEL_ID("modelId", 10), PHONE_GROUP("phoneGroupName", 11), PHONE_DESCRIPTION(
            "description", 12),
    // XMPP
    IM_ID("imId", 13),
    SALUTATION("userProfile.salutation", 14),
    MANAGER("userProfile.manager", 15),
    EMPLOYEE_ID("userProfile.employeeId", 16),
    JOB_TITLE("userProfile.jobTitle", 17),
    JOB_DEPT("userProfile.jobDept", 18),
    COMPANY_NAME("userProfile.companyName", 19),
    ASSISTANT_NAME("userProfile.assistantName", 20),
    CELL_PHONE_NUMBER("userProfile.cellPhoneNumber", 21),
    HOME_PHONE_NUMBER("userProfile.homePhoneNumber", 22),
    ASSISTANT_PHONE_NUMBER("userProfile.assistantPhoneNumber", 23),
    FAX_NUMBER("userProfile.faxNumber", 24),
    DID_NUMBER("userProfile.didNumber", 25),
    ALTERNATE_EMAIL("userProfile.alternateEmailAddress", 26),
    ALTERNATE_IM_ID("userProfile.alternateImId", 27),
    LOCATION("userProfile.location", 28),
    HOME_STREET("userProfile.homeAddress.street", 29),
    HOME_CITY("userProfile.homeAddress.city", 30),
    HOME_STATE("userProfile.homeAddress.state", 31),
    HOME_COUNTRY("userProfile.homeAddress.country", 32),
    HOME_ZIP("userProfile.homeAddress.zip", 33),
    OFFICE_STREET("userProfile.officeAddress.street", 34),
    OFFICE_CITY("userProfile.officeAddress.city", 35),
    OFFICE_STATE("userProfile.officeAddress.state", 36),
    OFFICE_COUNTRY("userProfile.officeAddress.country", 37),
    OFFICE_ZIP("userProfile.officeAddress.zip", 38),
    OFFICE_MAIL_STOP("userProfile.officeAddress.officeDesignation", 39),
    TWITTER_NAME("userProfile.twiterName", 40),
    LINKEDIN_NAME("userProfile.linkedinName", 41),
    FACEBOOK_NAME("userProfile.facebookName", 42),
    XING_NAME("userProfile.xingName", 43),
    // voice mail settings
    ACTIVE_GREETING("activeGreeting", 44),
    PRIMARY_EMAIL_NOTIFICATION("primaryEmailNotification", 45),
    PRIMARY_EMAIL_FORMAT("primaryEmailFormat", 46),
    PRIMARY_EMAIL_ATTACH_AUDIO("primaryEmailAttachAudio", 47),
    ALT_EMAIL_NOTIFICATION("alternateEmailNotification", 48),
    ALT_EMAIL_FORMAT("alternateEmailFormat", 49),
    ALT_EMAIL_ATTACH_AUDIO("alternateEmailAttachAudio", 50),
    VOICEMAIL_SERVER("voicemailServer", 51),
    // user caller alias
    EXTERNAL_NUMBER("externalNumber", 52),
    ANONYMOUS_CALLER_ALIAS("anonymousCallerAlias", 53),
    // additional phone settings
    ADDITIONAL_PHONE_SETTINGS("additionalPhoneSettings", 54),
    // additional line settings
    ADDITIONAL_LINE_SETTINGS("additionalLineSettings", 55),
    // authorization account name - can be used for authentication
    AUTH_ACCOUNT_NAME("userProfile.authAccountName", 56),
    // semi column separated email addresses - can be used for authentication
    EMAIL_ADDRESS_ALIASES("userProfile.emailAddressAliases", 57),
    //generic field 1
    CUSTOM_1("userProfile.custom1", 58),
    //generic field 2
    CUSTOM_2("userProfile.custom2", 59),
    //generic field 3
    CUSTOM_3("userProfile.custom3", 60);

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
            "User name", "PIN", "Voicemail PIN", "SIP password", "First name", "Last name", "User alias",
            "EMail address", "User group", "Phone serial number", "Phone model", "Phone group", "Phone description",
            "Im Id", "Salutation", "Manager", "EmployeeId", "Job Title", "Job department", "Company name",
            "Assistant name", "Cell phone number", "Home phone number", "Assistant phone number", "Fax number",
            "Did number", "Alternate email", "Alternate im", "Location", "Home street", "Home city", "Home state",
            "Home country", "Home zip", "Office street", "Office city", "Office state", "Office country",
            "Office zip", "Office mail stop", "Twitter", "Linkedin", "Facebook", "Xing", "Active greeting",
            "Email voicemail notification", "Email format", "Email attach audio",
            "Alternate email voicemail notification", "Alternate email format", "Alternate email attach audio",
            "Internal Voicemail Server", "Caller ID", "Block Caller ID", "Additional phone settings",
            "Additional line settings", "Auth Account Name", "EMail address aliases", "Custom 1", "Custom 2", "Custom 3"
        };
    }
}
