/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.PresenceEventDispatcher;
import org.jivesoftware.openfire.user.PresenceEventListener;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.openfire.client.OpenfireXmlRpcUnifiedPresenceNotificationClient;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;

/**
 * Singleton object that is meant to collect all the presence-related information related the
 * openfire users on the system. More specifically, by virtue of being a PresenceEventListener, it
 * will receive notifications whenever openfire users change their XMPP presence or status message
 * via their XMPP clients.
 *
 * This object also collects the SIP state information related to the SIP user linked to openfire
 * users. Any sub-component of this plug-in that wants to change the SIP state of a user must
 * inform this object via its sipStateChanged() method.
 *
 * Finally, this object aggregates the XMPP presence and SIP state into a 'unified presence' which
 * can be queried via the getUnifiedPresence() method.
 */
public class PresenceUnifier implements PresenceEventListener {
    private static PresenceUnifier instance = null;
    private static SipXOpenfirePlugin sipXopenfirePlugin = null;
    private static Logger log = Logger.getLogger(PresenceUnifier.class);
    private final Map<String, UnifiedPresence> unifiedPresenceMap = new ConcurrentHashMap<String, UnifiedPresence>();
    private final Map<String, UnifiedPresenceChangeListener> changeListenerMap = new HashMap<String, UnifiedPresenceChangeListener>();

    public static void setPlugin(SipXOpenfirePlugin plugin) {
        sipXopenfirePlugin = plugin;
    }

    public static PresenceUnifier getInstance() {
        if (instance == null) {
            instance = new PresenceUnifier();
        }
        return instance;
    }

    public UnifiedPresence getUnifiedPresence(String xmppUsername) throws UserNotFoundException {
        UnifiedPresence unifiedPresence = unifiedPresenceMap.get(xmppUsername);
        if (unifiedPresence == null) {
            unifiedPresence = new UnifiedPresence(xmppUsername);
            unifiedPresenceMap.put(xmppUsername, unifiedPresence);
            User ofUser = XMPPServer.getUserManager().getUser(xmppUsername);
            Presence presence = XMPPServer.getInstance().getPresenceManager().getPresence(ofUser);
            if (presence != null) {
                unifiedPresence.setXmppPresence(presence);
            }
        }
        return unifiedPresence;
    }

    public void setUnifiedPresence(String xmppUsername, UnifiedPresence.XmppPresence presence) {
        UnifiedPresence unifiedPresence;
        unifiedPresence = unifiedPresenceMap.get(xmppUsername);
        if (unifiedPresence == null) {
            unifiedPresence = new UnifiedPresence(xmppUsername);
            unifiedPresenceMap.put(xmppUsername, unifiedPresence);
        }
        unifiedPresence.setXmppPresence(presence);
    }

    // Notification that XMPP presence changed
    @Override
    public void presenceChanged(ClientSession session, Presence presence) {
        try {
            presenceChanged(session.getUsername(), presence);
        } catch (UserNotFoundException ex) {
            log.debug("presenceChanged caught " + ex.getMessage());
        }
    }

    // Notification that XMPP presence changed
    public void presenceChanged(String xmppUsername, Presence presence) {
        UnifiedPresence unifiedPresence = handleXmppPresenceChange(xmppUsername, presence);

        notifyListenersOfChange(unifiedPresence);
        log.debug("presenceChanged[2] for " + xmppUsername + " new presence show: " + presence.getShow()
                + " new presence type: " + presence.getType() + " new presence status: " + presence.getStatus()
                + " new unified presence: " + unifiedPresence.getUnifiedPresence());
    }

    @Override
    public void availableSession(ClientSession session, Presence presence) {
        try {
            UnifiedPresence unifiedPresence = handleXmppPresenceChange(session.getUsername(), presence);

            notifyListenersOfChange(unifiedPresence);
            log.debug("availableSession for " + session.getUsername() + " new presence show: " + presence.getShow()
                    + " new presence type: " + presence.getType() + " new presence status: " + presence.getStatus()
                    + " new unified presence: " + unifiedPresence.getUnifiedPresence());
        } catch (UserNotFoundException ex) {
            log.debug("unavailableSession caught " + ex.getMessage());
        }
    }

    @Override
    public void unavailableSession(ClientSession session, Presence presence) {
        try {
            UnifiedPresence unifiedPresence = handleXmppPresenceChange(session.getUsername(), presence);

            notifyListenersOfChange(unifiedPresence);
            log.debug("unavailableSession for " + session.getUsername() + " new presence show: "
                    + presence.getShow() + " new presence type: " + presence.getType() + " new presence status: "
                    + presence.getStatus() + " new unified presence: " + unifiedPresence.getUnifiedPresence());
        } catch (UserNotFoundException ex) {
            log.debug("unavailableSession caught " + ex.getMessage());
        }
    }

    @Override
    public void subscribedToPresence(JID subscriberJID, JID authorizerJID) {
        // nothing to do
    }

    @Override
    public void unsubscribedToPresence(JID subscriberJID, JID authorizerJID) {
        // nothing to do
    }

    private UnifiedPresence handleXmppPresenceChange(String xmppUsername, Presence presence) {
        UnifiedPresence unifiedPresence;
        unifiedPresence = unifiedPresenceMap.get(xmppUsername);
        if (unifiedPresence == null) {
            unifiedPresence = new UnifiedPresence(xmppUsername);
            unifiedPresenceMap.put(xmppUsername, unifiedPresence);
        }

        String newXmppStatusMessage = presence.getStatus();
        // add the SIP State information if we are on a call.
        unifiedPresence.setXmppPresence(presence);
        unifiedPresence.setXmppStatusMessage(newXmppStatusMessage);
        try {
            sipXopenfirePlugin.setPresenceStatus(unifiedPresence.getJidAsString(),
                    unifiedPresence.getXmppStatusMessageWithSipState());
        } catch (Exception e) {
            log.error(
                    "PresenceUnifier::xmppStatusMessageChanged() - caught exception for user "
                            + unifiedPresence.getJidAsString() + ":", e);
        }
        notifyListenersOfChange(unifiedPresence);
        return unifiedPresence;
    }

    private PresenceUnifier() {
        PresenceEventDispatcher.addListener(this);
    }

    public synchronized void addUnifiedPresenceChangeListener(String protocol, String serverUrl) {
        if (protocol.equals("xmlrpc")) {
            UnifiedPresenceChangeListener changeListener = null;
            try {
                if ((changeListener = changeListenerMap.get(serverUrl)) == null) {
                    changeListener = new OpenfireXmlRpcUnifiedPresenceNotificationClient(serverUrl,
                            sipXopenfirePlugin);
                    pushAllToListener(changeListener);
                    changeListenerMap.put(serverUrl, changeListener);
                    log.debug("changeListenerSet adding server for URL" + serverUrl);
                } else {
                    pushAllToListener(changeListener);
                    log.debug("changeListenerSet already contains server for URL" + serverUrl);
                }
            } catch (Exception ex) {
                log.error("addUnifiedPresenceChangeListener caught exception: ", ex);
            }
        }
    }

    public synchronized void removeUnifiedPresenceChangeListener(String protocol, String serverUrl) {
        if (protocol.equals("xmlrpc")) {
            changeListenerMap.remove(serverUrl);
        }
    }

    private synchronized void notifyListenersOfChange(UnifiedPresence changedPresence) {
        Collection<UnifiedPresenceChangeListener> listeners = changeListenerMap.values();
        for (UnifiedPresenceChangeListener listener : listeners) {
            listener.unifiedPresenceChanged(changedPresence.getXmppUsername(), changedPresence);
        }
    }

    private synchronized void pushAllToListener(UnifiedPresenceChangeListener listenerToRefresh) {
        Collection<UnifiedPresence> unifiedPresences = unifiedPresenceMap.values();
        for (UnifiedPresence up : unifiedPresences) {
            listenerToRefresh.unifiedPresenceChanged(up.getXmppUsername(), up);
        }
    }
}
