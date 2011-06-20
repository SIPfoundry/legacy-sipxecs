/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

import java.util.Map;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;

import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.openfire.client.OpenfireClientException;
import org.sipfoundry.openfire.client.OpenfireXmlRpcPresenceClient;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
import org.sipfoundry.sipcallwatcher.SipResourceState;

public class PhonePresence {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private OpenfireXmlRpcPresenceClient presenceClient;

    public PhonePresence() throws Exception {
        DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir")+"/domain-config");
        this.presenceClient = new OpenfireXmlRpcPresenceClient(IvrConfiguration.get().getOpenfireHost(),
                IvrConfiguration.get().getOpenfireXmlRpcPort(), config.getSharedSecret());
    }

    public Boolean isUserOnThePhone(String userId) throws Exception {
        try {
            Map unifiedPresence = presenceClient.getUnifiedPresenceInfo(userId);

            if (unifiedPresence.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
                throw new OpenfireClientException("Error in processing request for STATUS_CODE");
            }

            if (unifiedPresence.get(XmlRpcPresenceProvider.SIP_PRESENCE).equals(XmlRpcPresenceProvider.ERROR)) {
                throw new OpenfireClientException("Error in processing request for SIP_PRESENCE");
            }

            String sipPresence = (String) unifiedPresence.get(XmlRpcPresenceProvider.SIP_PRESENCE);

            LOG.info("Unified Presence: " + unifiedPresence);

            if (sipPresence.equals(SipResourceState.BUSY.toString())) {
                return true;
            }
        } catch (XmlRpcException e) {
            LOG.error("Error getting presence info for user " + userId, e);
            return false;
        }

        return false;
    }
}
