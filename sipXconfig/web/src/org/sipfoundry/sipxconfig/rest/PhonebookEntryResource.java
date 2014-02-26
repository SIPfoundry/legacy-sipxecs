/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.restlet.data.MediaType.TEXT_XML;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import com.thoughtworks.xstream.XStream;

import ezvcard.Ezvcard;
import ezvcard.VCard;
import ezvcard.parameter.AddressType;
import ezvcard.parameter.EmailType;
import ezvcard.parameter.ImppType;
import ezvcard.parameter.TelephoneType;
import ezvcard.property.Agent;
import ezvcard.property.Email;
import ezvcard.property.Impp;
import ezvcard.property.Organization;
import ezvcard.property.StructuredName;
import ezvcard.property.Telephone;
import ezvcard.property.TextProperty;

public class PhonebookEntryResource extends Resource {
    private static final Logger LOG = Logger.getLogger(PhonebookEntryResource.class);
    private PhonebookManager m_phonebookManager;
    private String m_internalId;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        m_internalId = (String) getRequest().getAttributes().get("internalId");
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        PhonebookEntry pbe = m_phonebookManager.findPhonebookEntryByInternalId(m_internalId);
        if (pbe == null) {
            Integer uid = Integer.parseInt(m_internalId);
            pbe = m_phonebookManager.getPhonebookEntry(uid);
        }
        if (pbe == null) {
            getResponse().setStatus(Status.CLIENT_ERROR_NOT_FOUND);
            return;
        }
        m_phonebookManager.deletePhonebookEntry(pbe);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        PrivatePhonebookEntry privatePbe = new PhonebookEntryRepresentation(entity).getObject();
        String uid = privatePbe.getUid();
        String internalId = privatePbe.getVcardId();
        String userName = privatePbe.getUserName();
        String vcardFromRequest = privatePbe.getVcard();
        LOG.debug("VCARD from req: " + privatePbe.getVcard());
        VCard vcard = Ezvcard.parse(vcardFromRequest).first();
        LOG.debug("VCARD: begin");
        LOG.debug(vcard.write());
        LOG.debug("VCARD: end");
        PhonebookEntry pbe;
        AddressBookEntry abe;
        if (StringUtils.isBlank(uid)) {
            pbe = m_phonebookManager.findPhonebookEntryByInternalId(internalId);
            if (pbe == null) {
                pbe = new PhonebookEntry();
                abe = new AddressBookEntry();
                pbe.setAddressBookEntry(abe);
                pbe.setInternalId(internalId);
            } else {
                abe = pbe.getAddressBookEntry();
            }
        } else {
            pbe = m_phonebookManager.getPhonebookEntry(Integer.parseInt(uid));
            abe = pbe.getAddressBookEntry();
        }
        pbe.setPhonebook(m_phonebookManager.getPrivatePhonebookCreateIfRequired(userName));
        vcardToPhonebook(pbe, abe, vcard);
        m_phonebookManager.savePhonebookEntry(pbe);
    }


    private static PhonebookEntry vcardToPhonebook(PhonebookEntry pbe, AddressBookEntry abe, VCard vcard) {

        // NAME
        StructuredName n = vcard.getStructuredName();
        pbe.setLastName(n.getFamily());
        pbe.setFirstName(n.getGiven());
        // ORG
        Organization org = vcard.getOrganization();
        if (org != null) {
            List<String> values = org.getValues();
            abe.setCompanyName(emptyIfNull(values, 0));
            abe.setJobDept(emptyIfNull(values, 1));
            abe.setJobTitle(getValueFromIndex(vcard.getTitles(), 0));
        }
        // IMPP
        if (vcard.getImpps() != null) {
            for (Impp impp : vcard.getImpps()) {
                for (ImppType p : impp.getTypes()) {
                    if (p.equals(ImppType.WORK)) {
                        abe.setImId(impp.getHandle());
                    } else if (p.equals(ImppType.PERSONAL)) {
                        abe.setAlternateImId(impp.getHandle());
                    }
                }
            }
        }
        // PHONE
        if (vcard.getTelephoneNumbers() != null) {
            for (Telephone tel : vcard.getTelephoneNumbers()) {
                if (tel.getTypes() != null) {
                    // we need to have the phone number set
                    for (TelephoneType t : tel.getTypes()) {
                        if (t.equals(TelephoneType.CELL)) {
                            abe.setCellPhoneNumber(tel.getText());
                            if (pbe.getNumber() == null) {
                                pbe.setNumber(tel.getText());
                            }
                        } else if (t.equals(TelephoneType.HOME)) {
                            abe.setHomePhoneNumber(tel.getText());
                            if (pbe.getNumber() == null) {
                                pbe.setNumber(tel.getText());
                            }
                        } else if (t.equals(TelephoneType.FAX)) {
                            abe.setFaxNumber(tel.getText());
                        } else {
                            pbe.setNumber(tel.getText());
                        }
                    }
                }
            }
        }
        // EMAIL
        if (vcard.getEmails() != null) {
            for (Email email : vcard.getEmails()) {
                for (EmailType e : email.getTypes()) {
                    if (e.equals(EmailType.WORK)) {
                        abe.setEmailAddress(email.getValue());
                    } else if (e.equals(EmailType.HOME)) {
                        abe.setAlternateEmailAddress(email.getValue());
                    }
                }
            }
        }
        // ADDRESS
        if (vcard.getAddresses() != null) {
            for (ezvcard.property.Address address : vcard.getAddresses()) {
                if (address.getTypes() != null) {
                    for (AddressType a : address.getTypes()) {
                        if (a.equals(AddressType.HOME)) {
                            Address addr = abe.getHomeAddress();
                            if (addr == null) {
                                addr = new Address();
                            }
                            addr.setCity(address.getLocality());
                            addr.setCountry(address.getCountry());
                            addr.setState(address.getRegion());
                            addr.setStreet(address.getStreetAddress());
                            addr.setZip(address.getPostalCode());
                            abe.setHomeAddress(addr);
                        } else if (a.equals(AddressType.WORK)) {
                            Address addr = abe.getOfficeAddress();
                            if (addr == null) {
                                addr = new Address();
                            }
                            addr.setCity(address.getLocality());
                            addr.setCountry(address.getCountry());
                            addr.setState(address.getRegion());
                            addr.setStreet(address.getStreetAddress());
                            addr.setZip(address.getPostalCode());
                            abe.setOfficeAddress(addr);
                        }
                    }
                }
            }
        }
        // ASSISTANT
        Agent asst = vcard.getAgent();
        if (asst != null) {
            VCard asstVcard = asst.getVCard();
            if (asstVcard != null) {
                if (asstVcard.getTelephoneNumbers() != null) {
                    for (Telephone tel : asstVcard.getTelephoneNumbers()) {
                        abe.setAssistantPhoneNumber(tel.getText());
                        break;
                    }
                    if (asstVcard.getFormattedName() != null) {
                        abe.setAssistantName(asstVcard.getFormattedName().getValue());
                    }
                }
            }
        }
        // PHOTO
        // NOT IMPLEMENTED - private contacts use Gravatars, we are unable to manipulate them
        return pbe;

    }

    private static class PrivatePhonebookEntry {
        private String m_internalId;
        private String m_uid;
        private String m_userName;
        private String m_vcard;

        public String getVcardId() {
            return m_internalId;
        }
        public void setVcardId(String vcardId) {
            m_internalId = vcardId;
        }
        public String getUid() {
            return m_uid;
        }
        public void setUid(String uid) {
            m_uid = uid;
        }
        public String getVcard() {
            return m_vcard;
        }
        public void setVcard(String vcard) {
            m_vcard = vcard;
        }
        public void setUserName(String userName) {
            m_userName = userName;
        }
        public String getUserName() {
            return m_userName;
        }
    }

    private static class PhonebookEntryRepresentation extends XStreamRepresentation<PrivatePhonebookEntry> {

        public PhonebookEntryRepresentation(Representation representation) {
            super(representation);
        }
        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("pb", PrivatePhonebookEntry.class);
        }
    }


    private static String emptyIfNull(List<String> values, int i) {
        if (i >= values.size()) {
            return EMPTY;
        }
        return StringUtils.defaultIfEmpty(values.get(i), EMPTY);
    }

    private static String getValueFromIndex(List< ? extends TextProperty> types, int i) {
        if (i >= types.size()) {
            return EMPTY;
        }
        return StringUtils.defaultIfEmpty(types.get(i).getValue(), EMPTY);
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }
}
