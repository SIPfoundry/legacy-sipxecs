/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.rest;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;
import static org.restlet.data.MediaType.TEXT_PLAIN;
import static org.restlet.data.MediaType.TEXT_XML;

public class ActiveGreetingResource extends Resource {
    private CoreContext m_coreContext;
    private String m_userName;
    private MailboxManager m_mboxManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(TEXT_PLAIN));
        m_userName = (String) getRequest().getAttributes().get("user");

    }

    public Representation represent(Variant variant) throws ResourceException {
        User user = m_coreContext.loadUserByUserName(m_userName);
        String activegreeting = user.getSettingValue(MailboxPreferences.ACTIVE_GREETING);
        if (TEXT_PLAIN.equals(variant.getMediaType())) {
            return new StringRepresentation(activegreeting);
        }
        return new Dom4jRepresentation(getDom(activegreeting));

    }

    public void storeRepresentation(Representation entity) throws ResourceException {
        String greeting = (String) getRequest().getAttributes().get("greeting");
        User user = m_coreContext.loadUserByUserName(m_userName);
        // we need the id here. we also have to make sure the id is recognized and not a random
        // string
        user.setSettingValue(MailboxPreferences.ACTIVE_GREETING, ActiveGreeting.fromId(greeting).getId());
        m_coreContext.saveUser(user);
        //write user's mailboxprefs.xml to get consistent data
        m_mboxManager.writePreferencesFile(user);
    }

    private Document getDom(String greeting) {
        DocumentFactory factory = DocumentFactory.getInstance();
        Document document = factory.createDocument();
        document.addElement("activegreeting").addText(greeting);

        return document;
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public MailboxManager getMailboxManager() {
        return m_mboxManager;
    }

    public void setMailboxManager(MailboxManager mboxManager) {
        m_mboxManager = mboxManager;
    }
}
