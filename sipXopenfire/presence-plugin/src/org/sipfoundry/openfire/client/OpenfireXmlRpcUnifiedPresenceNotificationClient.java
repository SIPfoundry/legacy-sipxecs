/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.client;

import java.net.URL;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.PresenceUnifier;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresence;
import org.sipfoundry.openfire.plugin.presence.UnifiedPresenceChangeListener;

public class OpenfireXmlRpcUnifiedPresenceNotificationClient extends OpenfireXmlRpcClient implements
        UnifiedPresenceChangeListener {

    private final XmlRpcClient client;
    private final String serverUrl;
    private final SipXOpenfirePlugin plugin;
    private static Logger logger = Logger.getLogger(OpenfireXmlRpcUnifiedPresenceNotificationClient.class);

    public OpenfireXmlRpcUnifiedPresenceNotificationClient(String serverUrl, SipXOpenfirePlugin plugin)
            throws Exception {
        this.plugin = plugin;
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setEnabledForExceptions(true);
        config.setEnabledForExtensions(true);
        config.setConnectionTimeout(120000);
        config.setReplyTimeout(120000);
        this.serverUrl = serverUrl;
        config.setServerURL(new URL(serverUrl));
        this.client = new XmlRpcClient();
        this.client.setConfig(config);
    }

    @Override
    public void unifiedPresenceChanged(String xmppUsername, UnifiedPresence newUnifiedPresence) {
        String retval = null;
        try {
            String[] args = new String[6];
            String xmppId = newUnifiedPresence.getXmppUsername() + "@" + plugin.getXmppDomain();
            args[0] = newUnifiedPresence.getXmppPresence().toString();
            args[1] = xmppId;
            args[2] = plugin.getSipId(xmppId);
            args[3] = "";
            args[4] = newUnifiedPresence.getUnifiedPresence();
            args[5] = newUnifiedPresence.getXmppStatusMessage();
            retval = (String) client.execute("UnifiedPresenceChangeListener.unifiedPresenceChanged", args);
        } catch (XmlRpcException e) {
            logger.info("unifiedPresenceChanged caught XmlRpcException for server " + serverUrl + " : ", e);
            logger.info("removing change listener as a result");
            PresenceUnifier.getInstance().removeUnifiedPresenceChangeListener("xmlrpc", serverUrl);
        } catch (Exception e) {
            logger.info("unifiedPresenceChanged caught exception " + e.getMessage() + "for server " + serverUrl);
        }
        if (retval != null && !retval.equals("ok")) {
            logger.info("unifiedPresenceChanged returned an error while processing request " + retval);
        }
    }

    @Override
    public boolean equals(Object obj) {
        return (obj instanceof Integer)
                && (serverUrl.equals(((OpenfireXmlRpcUnifiedPresenceNotificationClient) obj).serverUrl));
    }

    @Override
    public int hashCode() {
        return serverUrl.hashCode();
    }

}
