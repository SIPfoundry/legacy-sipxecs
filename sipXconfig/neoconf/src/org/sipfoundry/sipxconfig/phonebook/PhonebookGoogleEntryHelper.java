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

import java.lang.reflect.InvocationTargetException;
import java.util.List;

import com.google.gdata.data.Extension;
import com.google.gdata.data.contacts.ContactEntry;
import com.google.gdata.data.contacts.Relation;
import com.google.gdata.data.extensions.Email;
import com.google.gdata.data.extensions.Im;
import com.google.gdata.data.extensions.Name;
import com.google.gdata.data.extensions.Organization;
import com.google.gdata.data.extensions.PhoneNumber;
import com.google.gdata.data.extensions.StructuredPostalAddress;
import com.google.gdata.data.extensions.Where;

import static org.apache.commons.beanutils.BeanUtils.getSimpleProperty;
import static org.apache.commons.beanutils.PropertyUtils.getProperty;
import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.defaultString;
import static org.apache.commons.lang.StringUtils.isEmpty;
import static org.apache.commons.lang.StringUtils.isNotEmpty;
import static org.apache.commons.lang.StringUtils.split;

public class PhonebookGoogleEntryHelper {
    private static final String WORK = "work";
    private static final String GIVEN_NAME = "givenName";
    private static final String FAMILY_NAME = "familyName";
    private static final String FULL_NAME = "fullName";
    private static final String ORG_TITLE = "orgTitle";
    private static final String ORG_NAME = "orgName";
    private static final String ORG_DEPARTMENT = "orgDepartment";
    private static final String MOBILE = "mobile";
    private static final String HOME = "home";
    private static final String FAX = "fax";
    private static final String CITY = "city";
    private static final String COUNTRY = "country";
    private static final String REGION = "region";
    private static final String STREET = "street";
    private static final String POSTCODE = "postcode";
    private static final String VALUE = "value";
    private static final String ASSISTANT = "assistant";

    private final ContactEntry m_contactEntry;
    private final PhonebookEntry m_phonebookEntry;
    private final String m_account;

    PhonebookGoogleEntryHelper(ContactEntry entry, String account) {
        m_contactEntry = entry;
        m_account = account;
        m_phonebookEntry = createPhonebookEntry();
    }

    public PhonebookEntry getPhonebookEntry() {
        return m_phonebookEntry;
    }

    private PhonebookEntry createPhonebookEntry() {
        GooglePhonebookEntry phonebookEntry = new GooglePhonebookEntry();
        phonebookEntry.setInternalId(m_contactEntry.getId());
        phonebookEntry.setGoogleAccount(m_account);
        extractName(phonebookEntry);
        AddressBookEntry abe = new AddressBookEntry();
        extractIMs(abe);
        extractAddress(abe);
        extractPhones(phonebookEntry, abe);
        extractOrgs(abe);
        extractExtensions(abe);
        extractRelations(abe);
        extractEmailAddresses(abe);
        phonebookEntry.setAddressBookEntry(abe);
        return phonebookEntry;
    }

    private void extractName(PhonebookEntry phonebookEntry) {
        Name name = m_contactEntry.getName();
        if (name == null) {
            return;
        }
        String givenName = getGDataValue(name, GIVEN_NAME);
        String familyName = getGDataValue(name, FAMILY_NAME);
        if (isNotEmpty(givenName) || isNotEmpty(familyName)) {
            phonebookEntry.setFirstName(givenName);
            phonebookEntry.setLastName(familyName);
        } else {
            String fullName = getGDataValue(name, FULL_NAME);
            String[] names = split(fullName, " ", 2);
            if (names.length > 0) {
                phonebookEntry.setFirstName(names[0]);
            }
            if (names.length > 1) {
                phonebookEntry.setLastName(names[1]);
            }
        }
    }

    private void extractIMs(AddressBookEntry abe) {
        List<Im> ims = m_contactEntry.getImAddresses();
        for (Im im : ims) {
            if (isEmpty(abe.getImId())) {
                abe.setImId(im.getAddress());
            } else {
                if (isEmpty(abe.getAlternateImId())) {
                    abe.setAlternateImId(im.getAddress());
                }
            }
        }
    }

    private void extractPhones(PhonebookEntry phonebookEntry, AddressBookEntry abe) {
        if (!m_contactEntry.hasPhoneNumbers()) {
            return;
        }

        String mobileNumber = null;
        String homeNumber = null;
        for (PhoneNumber number : m_contactEntry.getPhoneNumbers()) {
            String rel = defaultString(number.getRel());
            String phoneNumber = number.getPhoneNumber();
            if (rel.endsWith(WORK)) {
                phonebookEntry.setNumber(phoneNumber);
            } else if (rel.endsWith(MOBILE)) {
                abe.setCellPhoneNumber(phoneNumber);
                mobileNumber = phoneNumber;
            } else if (rel.endsWith(HOME)) {
                abe.setHomePhoneNumber(phoneNumber);
                homeNumber = phoneNumber;
            } else if (rel.endsWith(FAX)) {
                abe.setFaxNumber(phoneNumber);
            }
        }
        //Set the available phone number if any
        if (phonebookEntry.getNumber() == null) {
            if (mobileNumber != null) {
                phonebookEntry.setNumber(mobileNumber);
            } else if (homeNumber != null) {
                phonebookEntry.setNumber(homeNumber);
            }
        }
    }

    private void extractAddress(AddressBookEntry abe) {
        if (!m_contactEntry.hasStructuredPostalAddresses()) {
            return;
        }
        for (StructuredPostalAddress address : m_contactEntry.getStructuredPostalAddresses()) {
            if (address == null) {
                continue;
            }
            Address phonebookAddress = new Address();
            extractPostalAddress(phonebookAddress, address);
            if (address.getRel().contains(WORK)) {
                abe.setOfficeAddress(phonebookAddress);
            } else {
                abe.setHomeAddress(phonebookAddress);
            }
        }
    }

    private void extractOrgs(AddressBookEntry abe) {
        List<Organization> orgs = m_contactEntry.getOrganizations();
        if (orgs.isEmpty()) {
            return;
        }
        Organization org = orgs.get(0);
        if (org == null) {
            return;
        }
        abe.setJobTitle(getGDataValue(org, ORG_TITLE));
        abe.setCompanyName(getGDataValue(org, ORG_NAME));
        abe.setJobDept(getGDataValue(org, ORG_DEPARTMENT));
    }

    private void extractRelations(AddressBookEntry abe) {
        for (Relation relation : m_contactEntry.getRelations()) {
            if (relation.hasRel() && relation.getRel().toValue().contains(ASSISTANT)) {
                abe.setAssistantName(relation.getValue());
            }
        }
    }

    private void extractExtensions(AddressBookEntry abe) {
        for (Extension extension : m_contactEntry.getExtensions()) {
            if (extension instanceof Where) {
                abe.setLocation(((Where) extension).getValueString());
            }
        }
    }

    private void extractPostalAddress(Address phonebookAddress, StructuredPostalAddress addressGmail) {
        phonebookAddress.setCity(getGDataValue(addressGmail, CITY));
        phonebookAddress.setCountry(getGDataValue(addressGmail, COUNTRY));
        phonebookAddress.setState(getGDataValue(addressGmail, REGION));
        phonebookAddress.setStreet(getGDataValue(addressGmail, STREET));
        phonebookAddress.setZip(getGDataValue(addressGmail, POSTCODE));
    }

    private void extractEmailAddresses(AddressBookEntry abe) {
        List<Email> emails = m_contactEntry.getEmailAddresses();
        for (Email email : emails) {
            if (email.getPrimary()) {
                abe.setEmailAddress(email.getAddress());
            } else {
                abe.setAlternateEmailAddress(email.getAddress());
            }
        }
    }

    /**
     * GData has its own system of "extensible" beans. This is a brute force way of extracting
     * values from them.
     */
    private static String getGDataValue(Object bean, String name) {
        try {
            String value = null;
            Object beanProp = getProperty(bean, name);
            if (beanProp != null) {
                value = getSimpleProperty(beanProp, VALUE);
            }
            return defaultString(value);
        } catch (IllegalAccessException e) {
            return EMPTY;
        } catch (InvocationTargetException e) {
            return EMPTY;
        } catch (NoSuchMethodException e) {
            return EMPTY;
        }
    }
}
