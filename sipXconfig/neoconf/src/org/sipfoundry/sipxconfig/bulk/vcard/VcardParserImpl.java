/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.vcard;

import java.io.IOException;
import java.io.Reader;
import java.util.Iterator;

import net.sf.vcard4j.java.AddressBook;
import net.sf.vcard4j.java.VCard;
import net.sf.vcard4j.java.type.ADR;
import net.sf.vcard4j.java.type.EMAIL;
import net.sf.vcard4j.java.type.N;
import net.sf.vcard4j.java.type.ORG;
import net.sf.vcard4j.java.type.TEL;
import net.sf.vcard4j.java.type.TITLE;
import net.sf.vcard4j.parser.DomParser;
import org.apache.commons.collections.Closure;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.xerces.dom.DocumentImpl;
import org.sipfoundry.sipxconfig.bulk.BulkParser;
import org.w3c.dom.Document;

public class VcardParserImpl implements BulkParser {
    public static final String NAME = "N";
    public static final String ORGANIZATION = "ORG";
    public static final String TITLE_TYPE = "TITLE";

    public void parse(Reader reader, Closure closure) {
        try {
            DomParser parser = new DomParser();
            Document document = new DocumentImpl();
            parser.parse(reader, document);

            AddressBook addressBook = new AddressBook(document);
            for (Iterator vcards = addressBook.getVCards(); vcards.hasNext();) {
                RawPhonebookEntry entry = parseVcard((VCard) vcards.next());
                String[] entryStrings = entry.getEntry();
                if (entryStrings != null) {
                    closure.execute(entryStrings);
                }
            }
        } catch (IOException e) {
            throw new VCardParserException();
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }

    private RawPhonebookEntry parseVcard(VCard vcard) {
        RawPhonebookEntry entry = new RawPhonebookEntry();

        if (hasType(vcard, NAME)) {
            N name = (N) vcard.getTypes(NAME).next();
            entry.m_firstName = name.getGiven();
            entry.m_lastName = name.getFamily();
        }

        for (Iterator tels = vcard.getTypes("TEL"); tels.hasNext();) {
            TelHelper th = new TelHelper(((TEL) tels.next()));

            if (th.isFax()) {
                entry.m_faxNumber = th.get();
            } else if (th.isWork()) {
                entry.m_phoneNumber = th.get();
            } else if (th.isCell()) {
                entry.m_cellPhoneNumber = th.get();
            } else if (th.isHome()) {
                entry.m_homePhoneNumber = th.get();
            }
        }

        if (hasType(vcard, ORGANIZATION)) {
            ORG org = (ORG) vcard.getTypes(ORGANIZATION).next();
            entry.m_companyName = org.getOrgname();
            entry.m_jobDept = org.getOrgunit();
        }

        for (Iterator adrs = vcard.getTypes("ADR"); adrs.hasNext();) {
            ADR adr = (ADR) adrs.next();
            ADR.Parameters params = (ADR.Parameters) adr.getParameters();
            if (params.containsTYPE(ADR.Parameters.TYPE_HOME)) {
                entry.m_homeAddressStreet = adr.getStreet();
                entry.m_homeAddressZip = adr.getPcode();
                entry.m_homeAddressCountry = adr.getCountry();
                entry.m_homeAddressState = adr.getRegion();
                entry.m_homeAddressCity = adr.getLocality();
            } else if (params.containsTYPE(ADR.Parameters.TYPE_WORK)) {
                entry.m_officeAddressStreet = adr.getStreet();
                entry.m_officeAddressZip = adr.getPcode();
                entry.m_officeAddressCountry = adr.getCountry();
                entry.m_officeAddressState = adr.getRegion();
                entry.m_officeAddressCity = adr.getLocality();
                entry.m_officeAddressOfficeDesignation = adr.getPobox();
            }
        }

        if (hasType(vcard, TITLE_TYPE)) {
            TITLE title = (TITLE) vcard.getTypes(TITLE_TYPE).next();
            entry.m_jobTitle = title.get();
        }

        for (Iterator emails = vcard.getTypes("EMAIL"); emails.hasNext();) {
            EMAIL email = (EMAIL) emails.next();
            EMAIL.Parameters params = (EMAIL.Parameters) email.getParameters();
            if (params.containsTYPE(EMAIL.Parameters.TYPE_PREF)) {
                entry.m_emailAddress = email.get();
            } else {
                entry.m_alternateEmailAddress = email.get();
            }
        }
        if (entry.m_emailAddress == null && entry.m_alternateEmailAddress != null) {
            entry.m_emailAddress = new String(entry.m_alternateEmailAddress);
            entry.m_alternateEmailAddress = null;
        }
        return entry;
    }

    private boolean hasType(VCard vcard, String type) {
        return vcard.getTypes(type).hasNext();
    }

    private static class TelHelper {
        private static final int[] ALL_TYPES = {
            TEL.Parameters.TYPE_BBS, TEL.Parameters.TYPE_CAR, TEL.Parameters.TYPE_CELL, TEL.Parameters.TYPE_FAX,
            TEL.Parameters.TYPE_HOME, TEL.Parameters.TYPE_ISDN, TEL.Parameters.TYPE_MODEM, TEL.Parameters.TYPE_MSG,
            TEL.Parameters.TYPE_PAGER, TEL.Parameters.TYPE_PCS, TEL.Parameters.TYPE_PREF, TEL.Parameters.TYPE_VIDEO,
            TEL.Parameters.TYPE_VOICE, TEL.Parameters.TYPE_WORK
        };

        private final TEL m_tel;
        private final TEL.Parameters m_params;

        public TelHelper(TEL tel) {
            m_tel = tel;
            m_params = (TEL.Parameters) m_tel.getParameters();
        }

        public boolean isCell() {
            return m_params.containsTYPE(TEL.Parameters.TYPE_CELL);
        }

        public String get() {
            return m_tel.get();
        }

        public boolean isFax() {
            return containsAll(TEL.Parameters.TYPE_FAX, TEL.Parameters.TYPE_WORK)
                    || containsOnly(TEL.Parameters.TYPE_FAX);
        }

        public boolean isWork() {
            return containsAll(TEL.Parameters.TYPE_WORK, TEL.Parameters.TYPE_VOICE)
                    || containsOnly(TEL.Parameters.TYPE_WORK);
        }

        public boolean isHome() {
            return containsAll(TEL.Parameters.TYPE_HOME, TEL.Parameters.TYPE_VOICE)
                    || containsOnly(TEL.Parameters.TYPE_HOME);
        }

        private boolean containsOnly(int currentParam) {
            if (!m_params.containsTYPE(currentParam)) {
                return false;
            }

            for (int i = 0; i < ALL_TYPES.length; i++) {
                if (ALL_TYPES[i] == currentParam) {
                    continue;
                }
                if (m_params.containsTYPE(ALL_TYPES[i])) {
                    return false;
                }
            }
            return true;
        }

        private boolean containsAll(int... types) {
            for (int type : types) {
                if (!m_params.containsTYPE(type)) {
                    return false;
                }
            }
            return true;
        }

    }

    private static class RawPhonebookEntry {
        private String m_firstName;
        private String m_lastName;

        private String m_phoneNumber;
        private String m_cellPhoneNumber;
        private String m_homePhoneNumber;
        private String m_faxNumber;

        private String m_emailAddress;
        private String m_alternateEmailAddress;

        private String m_companyName;
        private String m_jobTitle;
        private String m_jobDept;

        private String m_homeAddressStreet;
        private String m_homeAddressZip;
        private String m_homeAddressCountry;
        private String m_homeAddressState;
        private String m_homeAddressCity;

        private String m_officeAddressStreet;
        private String m_officeAddressZip;
        private String m_officeAddressCountry;
        private String m_officeAddressState;
        private String m_officeAddressCity;
        private String m_officeAddressOfficeDesignation;

        public String[] getEntry() {
            if (isEmpty()) {
                return null;
            }
            return new String[] {
                m_firstName, m_lastName, m_phoneNumber, m_cellPhoneNumber, m_homePhoneNumber, m_faxNumber,
                m_emailAddress, m_alternateEmailAddress, m_companyName, m_jobTitle, m_jobDept, m_homeAddressStreet,
                m_homeAddressZip, m_homeAddressCountry, m_homeAddressState, m_homeAddressCity,
                m_officeAddressStreet, m_officeAddressZip, m_officeAddressCountry, m_officeAddressState,
                m_officeAddressCity, m_officeAddressOfficeDesignation
            };
        }

        private boolean isEmpty() {
            return StringUtils.isEmpty(m_phoneNumber)
                    || (StringUtils.isEmpty(m_firstName) && StringUtils.isEmpty(m_lastName));
        }
    }
}
