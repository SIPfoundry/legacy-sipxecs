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
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.springframework.beans.factory.annotation.Required;

public class CallResource extends UserResource {
    private static final String VALID_PHONE_OR_SIP_URI = "([-_.!~*'\\(\\)&=+$,;?/a-zA-Z0-9]|"
            + "(%[0-9a-fA-F]{2}))+|([-_.!~*'\\(\\)&=+$,;?/a-zA-Z0-9]|"
            + "(%[0-9a-fA-F]{2}))+@\\w[-._\\w]*\\w\\.\\w{2,6}";

    private SipService m_sipService;
    private DomainManager m_domainManager;
    private String m_to;
    private String m_from;
    private Status m_errorStatus;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        String domain = m_domainManager.getDomain().getName();
        String to = (String) getRequest().getAttributes().get("to");
        to = to.replaceAll("[\\[\\]\\(\\)\\.\\{\\}\\-]", "");
        if (!to.isEmpty() && to.matches(VALID_PHONE_OR_SIP_URI)) {
            m_from = getUser().getAddrSpec(domain);
            m_to = SipUri.fix(to, domain);
        } else {
            m_errorStatus = Status.CLIENT_ERROR_BAD_REQUEST;
        }

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
        if (m_errorStatus == null) {
            m_sipService.sendRefer(getUser(), m_from, m_to);
        } else {
            getResponse().setStatus(m_errorStatus);
        }
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
