package org.sipfoundry.openfire.plugin.presence;

import java.util.HashMap;
import java.util.Map;

import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.sipcallwatcher.CallWatcher;
import org.sipfoundry.sipcallwatcher.ProtocolObjects;
import org.sipfoundry.sipcallwatcher.ResourceState;
import org.sipfoundry.sipcallwatcher.ResourceStateChangeListener;
import org.sipfoundry.sipcallwatcher.ResourceStateEvent;

public class ResourceStateChangeListenerImpl implements ResourceStateChangeListener {

    private static Logger logger = Logger.getLogger(ResourceStateChangeListenerImpl.class);
    private SipXOpenfirePlugin plugin;
    private Map<String, String> savedMessages = new HashMap<String, String>();

    public ResourceStateChangeListenerImpl(SipXOpenfirePlugin plugin) {
        this.plugin = plugin;
        logger.addAppender(CallWatcher.getLogAppender());
    }

    public void handleResourceStateChange(ResourceStateEvent resourceStateEvent) {
        try {
            String resource = resourceStateEvent.getUser();
            SipURI sipUri = (SipURI) ProtocolObjects.addressFactory.createURI(resource);
            String user = sipUri.getUser() + "@" + sipUri.getHost();
            ResourceState resourceState = resourceStateEvent.getState();
            String xmppId = plugin.getXmppId(user);
            logger
                    .info("handleResourceStateChange " + user + " resourceState = "
                            + resourceState);
            plugin.setSipPresence(user, resourceState.toString());

            if (resourceState.equals(ResourceState.BUSY)) {
                if (xmppId != null) {
                    savedMessages.put( xmppId, plugin.getPresenceStatus(xmppId) );
                    String statusMessage = plugin.getOnThePhoneMessage(user);
                    if ( statusMessage != null ) {
                        plugin.setPresenceStatus(xmppId, statusMessage);
                    }
                }
            } else if (resourceState.equals(ResourceState.IDLE)) {
                if (xmppId != null) {
                    String savedMessage = savedMessages.get( xmppId );
                    if (savedMessage != null) {
                        plugin.setPresenceStatus(xmppId, savedMessage);
                    } else {
                        plugin.setPresenceStatus(xmppId, "");
                    }
                }
            }
        } catch (Exception ex) {
            logger.error("Exception processing resource change event", ex);
        }

    }

}
