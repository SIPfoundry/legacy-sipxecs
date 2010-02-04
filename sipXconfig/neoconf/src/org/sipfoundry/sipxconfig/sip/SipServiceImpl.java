/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sip;

import gov.nist.javax.sip.clientauthutils.UserCredentials;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class SipServiceImpl extends SipStackBean implements SipService {

    static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

    private DomainManager m_domainManager;

    private boolean m_allowForwarding;

    public void sendCheckSync(String addrSpec) {
        AbstractMessage message = new NotifyMessage(this, addrSpec, "check-sync");
        message.createAndSend();
    }

    public void sendNotify(String addrSpec, String eventType, String contentType, byte[] payload) {
        AbstractMessage message = new NotifyMessage(this, addrSpec, eventType, contentType,
                payload);
        message.createAndSend();
    }


    public void sendRefer(User user, String sourceAddrSpec, String displayName, String destinationAddrSpec,
            String referTarget, boolean allowForwarding) {
        UserCredentials credentials = new UserCredentialsImpl(user, m_domainManager.getAuthorizationRealm());
        LOG.debug("sendRefer: source = " + sourceAddrSpec + " dest = " + destinationAddrSpec);
        InviteMessage message = new InviteMessage(this, credentials, displayName, sourceAddrSpec,
                sourceAddrSpec, referTarget, Operator.SEND_3PCC_REFER_CALL_SETUP);
        message.setforwardingAllowed(allowForwarding);
        message.createAndSend();
    }

    public void sendRefer(User user, String sourceAddrSpec, String destinationAddrSpec) {
        sendRefer(user, sourceAddrSpec, null, destinationAddrSpec, destinationAddrSpec, m_allowForwarding);
    }

    public void sendRefer(User user, String sourceAddrSpec, String displayName, String destinationAddrSpec) {
        sendRefer(user, sourceAddrSpec, displayName, destinationAddrSpec, destinationAddrSpec, m_allowForwarding);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setAllowReferForwarding(boolean allowForwarding) {
        m_allowForwarding = allowForwarding;
    }
}
