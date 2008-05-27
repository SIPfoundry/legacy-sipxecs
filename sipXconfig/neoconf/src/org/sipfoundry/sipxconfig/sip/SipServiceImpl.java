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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.annotation.Required;

/**
 * Implementation note: This is very, very basic sip implementation. Going forward, either use
 * sipXtack java binding or interact w/sipXpublisher thru some RPC to have it deliver sip messages
 * on behalf or sipXconfig
 */
public class SipServiceImpl implements SipService {

    static final Log LOG = LogFactory.getLog(SipServiceImpl.class);

    private String m_serverName;

    private String m_proxyHost;

    private int m_proxyPort = SipUri.DEFAULT_SIP_PORT;

    public void setProxyHost(String proxy) {
        m_proxyHost = proxy;
    }

    @Required
    public void setProxyPort(int port) {
        m_proxyPort = port;
    }

    @Required
    public void setFromServerName(String serverName) {
        m_serverName = serverName;
    }

    public void sendCheckSync(String addrSpec) {
        AbstractMessage message = new NotifyMessage(m_serverName, addrSpec, "check-sync");
        message.createAndSend(m_proxyHost, m_proxyPort);
    }

    public void sendNotify(String addrSpec, String eventType, String contentType, byte[] payload) {
        AbstractMessage message = new NotifyMessage(m_serverName, addrSpec, eventType, contentType, payload);
        message.createAndSend(m_proxyHost, m_proxyPort);
    }

    public void sendRefer(String sourceAddrSpec, String destinationAddSpec) {
        AbstractMessage message = new ReferMessage(m_serverName, sourceAddrSpec, destinationAddSpec);
        message.createAndSend(m_proxyHost, m_proxyPort);
    }
}
