/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

import java.util.HashMap;
import java.util.Map;

import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.sipfoundry.sipcallwatcher.CallWatcher;
import org.sipfoundry.sipcallwatcher.SipResourceState;
import org.sipfoundry.sipcallwatcher.ResourceStateChangeListener;
import org.sipfoundry.sipcallwatcher.ResourceStateEvent;
import org.xmpp.packet.JID;

public class ResourceStateChangeListenerImpl implements ResourceStateChangeListener {

    private static Logger logger = Logger.getLogger(ResourceStateChangeListenerImpl.class);
    private SipXOpenfirePlugin plugin;

    public ResourceStateChangeListenerImpl(SipXOpenfirePlugin plugin) {
        this.plugin = plugin;
        logger.addAppender(CallWatcher.getSipStackBean().getStackAppender());
    }

    public void handleResourceStateChange(ResourceStateEvent resourceStateEvent) {
        try {
            String resource = resourceStateEvent.getUser();
            SipURI sipUri = (SipURI) CallWatcher.getSipStackBean().getAddressFactory().createURI(resource);
            String sipUserId = sipUri.getUser();
            SipResourceState resourceState = resourceStateEvent.getState();
            logger.info("handleResourceStateChange " + sipUserId + " resourceState = "
                    + resourceState + "; remote endpoint:" + resourceStateEvent.getRemoteEndpoint());
            String xmppId = plugin.getXmppId(sipUserId);
            JID jid = new JID(xmppId);
            PresenceUnifier.getInstance().sipStateChanged( jid.getNode(), resourceState, resourceStateEvent.getRemoteEndpoint() );
        } catch (Exception ex) {
            logger.error("Exception processing resource change event", ex);
        }

    }

}
