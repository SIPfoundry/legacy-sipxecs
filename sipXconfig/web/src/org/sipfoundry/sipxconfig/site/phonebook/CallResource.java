/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import org.restlet.Context;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.springframework.beans.factory.annotation.Required;

public class CallResource extends Resource {
    private SipService m_sipService;
    private DomainManager m_domainManager;
    private CoreContext m_coreContext;
    private String m_to;
    private String m_from;
    private User m_user;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        ChallengeResponse challengeResponse = request.getChallengeResponse();
        String username = challengeResponse.getIdentifier();
        m_user = m_coreContext.loadUserByUserName(username);

        String domain = m_domainManager.getDomain().getName();
        String to = (String) getRequest().getAttributes().get("to");
        m_from = m_user.getAddrSpec(domain);
        m_to = SipUri.fix(to, domain);
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
        m_sipService.sendRefer(m_user, m_from, m_to);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setSipService(SipService sipService) {
        m_sipService = sipService;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
