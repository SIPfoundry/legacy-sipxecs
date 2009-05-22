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
import org.restlet.data.ChallengeResponse;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.permission.PermissionName.TUI_CHANGE_PIN;

public class VoicemailPinResource extends Resource {
    private DomainManager m_domainManager;
    private CoreContext m_coreContext;
    private User m_user;
    private String m_newPin;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);

        ChallengeResponse challengeResponse = request.getChallengeResponse();
        String username = challengeResponse.getIdentifier();
        m_user = m_coreContext.loadUserByUserName(username);

        setModifiable(m_user.hasPermission(TUI_CHANGE_PIN));
        setAvailable(m_user.hasPermission(TUI_CHANGE_PIN));
        setReadable(false);

        m_newPin = (String) getRequest().getAttributes().get("pin");
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        String realm = m_domainManager.getDomain().getSipRealm();
        m_user.setPin(m_newPin, realm);
        m_coreContext.saveUser(m_user);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
