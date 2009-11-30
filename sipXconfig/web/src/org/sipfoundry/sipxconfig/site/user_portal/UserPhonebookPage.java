/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.lucene.queryParser.ParseException;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.sip.SipService;

public abstract class UserPhonebookPage extends UserBasePage {
    private static final Log LOG = LogFactory.getLog(UserPhonebookPage.class);

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @InjectObject("spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    @InjectObject("spring:sip")
    public abstract SipService getSipService();

    @Asset("/gwt/org.sipfoundry.sipxconfig.userportal.user_phonebook_search/nocache.js")
    public abstract IAsset getUserPhonebookSearchJs();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @Persist
    public abstract void setQuery(String query);

    public abstract String getQuery();

    @Persist
    public abstract Collection<PhonebookEntry> getPhonebookEntries();

    public abstract void setPhonebookEntries(Collection<PhonebookEntry> entries);

    public abstract void setCurrentNumber(String number);

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getPhonebookEntries() == null) {
            initializeEntries();
        }
    }

    private void initializeEntries() {
        User user = getUser();
        String query = getQuery();
        Collection<Phonebook> phonebooks = getPhonebooks();
        Collection<PhonebookEntry> entries = null;
        if (StringUtils.isEmpty(query)) {
            entries = getPhonebookManager().getEntries(phonebooks, user);
        } else {
            entries = getPhonebookManager().search(phonebooks, query, user);
        }
        setPhonebookEntries(entries);
    }

    /**
     * Implements click to call link
     *
     * @param number number to call - refer is sent to current user
     */
    public void call(String number) {
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getUser().getAddrSpec(domain);
        String destAddrSpec = SipUri.fix(number, domain);
        getSipService().sendRefer(getUser(), userAddrSpec, destAddrSpec);
    }

    /**
     * Filters the phonebook entries based on the value of getQuery()
     */
    public void search() throws IOException, ParseException {
        setPhonebookEntries(null);
    }

    private Collection<Phonebook> getPhonebooks() {
        User user = getUser();
        return getPhonebookManager().getPhonebooksByUser(user);
    }

    /**
     * Called whenever new row is about to displayed. Sorts entries into extensions (that look
     * like phone numbers) and sipIds (that look like SIP usernames)
     *
     * @param entry phone book entry
     */
    public void setPhonebookEntry(PhonebookEntry entry) {
        setCurrentNumber(entry.getNumber());
    }

    public void export() {
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);
        try {
            String name = String.format("phonebook_%s.vcf", getUser().getUserName());
            OutputStream stream = TapestryUtils.getResponseOutputStream(getResponse(), name, "text/x-vcard");
            Collection<PhonebookEntry> entries = getPhonebookEntries();
            getPhonebookManager().exportPhonebook(entries, stream);
            stream.close();
        } catch (IOException e) {
            LOG.error("Cannot export phonebook", e);
            Messages messages = getMessages();
            validator.record(messages.format("msg.exportError", e.getLocalizedMessage()),
                    ValidationConstraint.CONSISTENCY);
        }
    }
}
