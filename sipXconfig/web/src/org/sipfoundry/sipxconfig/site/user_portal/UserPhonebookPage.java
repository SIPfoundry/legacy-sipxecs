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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.tapestry.annotations.InjectObject;
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

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    public String getWidgetSrc() {
        return getRequestCycle().getAbsoluteURL('/' + UserPhonebookWidgetPage.PAGE + ".html");
    }

    protected Collection<PhonebookEntry> getPhonebookEntries() {
        User user = getUser();
        Collection<Phonebook> phonebooks = getPhonebookManager().getAllPhonebooksByUser(user);
        return getPhonebookManager().getEntries(phonebooks, user);
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
        String displayName = "ClickToCall";
        getSipService().sendRefer(getUser(), userAddrSpec, displayName, destAddrSpec);
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
