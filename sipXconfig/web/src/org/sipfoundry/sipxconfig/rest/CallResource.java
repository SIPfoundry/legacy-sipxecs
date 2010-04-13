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

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Method;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.springframework.beans.factory.annotation.Required;

public class CallResource extends UserResource {
    private SipService m_sipService;
    private DomainManager m_domainManager;
    private String m_to;
    private String m_from;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        String domain = m_domainManager.getDomain().getName();
        String to = (String) getRequest().getAttributes().get("to");
        m_from = getUser().getAddrSpec(domain);
        m_to = SipUri.fix(to, domain);

        // NOTE: Due to the bug in Restlet, it requires PUT and POST request must have
        // entity. The following hack is to workaround the bug.
        if (request.getMethod().equals(Method.PUT) && !request.isEntityAvailable()) {
            request.setEntity(" ", MediaType.APPLICATION_ATOM_XML);
        }
    }

    @Override
    public boolean allowGet() {
        return false;
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public void put(Representation entity) {
        m_sipService.sendRefer(getUser(), m_from, m_to);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setSipService(SipService sipService) {
        m_sipService = sipService;
    }
}
